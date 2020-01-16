#include "ofxNvFlex.h"
#include <stdlib.h>
#define DEBUG_MODE false

//error
namespace m {
	void ErrorCallback(NvFlexErrorSeverity, const char* msg, const char* file, int line)
	{
		printf("Flex: %s - %s:%d\n", msg, file, line);
		 
	}
}

void ofx_nvflex::init_flex()
{

	int g_device = -1;
	g_device = NvFlexDeviceGetSuggestedOrdinal();
	bool success = NvFlexDeviceCreateCudaContext(g_device);

	if (!success)
	{
		printf("Error creating CUDA context.\n");
	}

	NvFlexInitDesc desc;
	desc.deviceIndex = g_device;
	desc.enableExtensions = true;
	desc.renderDevice = 0;
	desc.renderContext = 0;
	desc.computeType = eNvFlexCUDA;

	library = NvFlexInit(NV_FLEX_VERSION, m::ErrorCallback, &desc);

	cursor = 0; 

	buffers = new SimBuffers(library);
	buffers->MapBuffers();
	buffers->InitBuffers();

	NvFlexSetSolverDescDefaults(&g_solverDesc);

	maxParticles = 50000;

	g_maxDiffuseParticles = 0;
	g_maxNeighborsPerParticle = 196;
	g_maxContactsPerParticle = 8;

	//g_solverDesc.featureMode = eNvFlexFeatureModeSimpleFluids ; // eNvFlexFeatureModeDefault;
	g_solverDesc.featureMode = eNvFlexFeatureModeSimpleSolids; // eNvFlexFeatureModeDefault;
//	g_solverDesc.featureMode = eNvFlexFeatureModeDefault;

	g_solverDesc.maxParticles = maxParticles;
	g_solverDesc.maxDiffuseParticles = 0;
	g_solverDesc.maxNeighborsPerParticle = g_maxNeighborsPerParticle;
	g_solverDesc.maxContactsPerParticle = g_maxContactsPerParticle;

 

	solver = NvFlexCreateSolver(library, &g_solverDesc);
	
	 
}

Mesh* ImportMeshFromObj(const char* path)
{
	ifstream file(path);

	if (!file)
		return NULL;

	Mesh* m = new Mesh();

	vector<Point3> positions;
	vector<Vector3> normals;
	vector<Vector2> texcoords;
	vector<Vector3> colors;
	vector<uint32_t>& indices = m->m_indices;

	//typedef unordered_map<VertexKey, uint32_t, MemoryHash<VertexKey> > VertexMap;
	typedef map<VertexKey, uint32_t> VertexMap;
	VertexMap vertexLookup;

	// some scratch memory
	const uint32_t kMaxLineLength = 1024;
	char buffer[kMaxLineLength];

	//double startTime = GetSeconds();

	while (file)
	{
		file >> buffer;

		if (strcmp(buffer, "vn") == 0)
		{
			// normals
			float x, y, z;
			file >> x >> y >> z;

			normals.push_back(Vector3(x, y, z));
		}
		else if (strcmp(buffer, "vt") == 0)
		{
			// texture coords
			float u, v;
			file >> u >> v;

			texcoords.push_back(Vector2(u, v));
		}
		else if (buffer[0] == 'v')
		{
			// positions
			float x, y, z;
			file >> x >> y >> z;

			positions.push_back(Point3(x, y, z));
		}
		else if (buffer[0] == 's' || buffer[0] == 'g' || buffer[0] == 'o')
		{
			// ignore smoothing groups, groups and objects
			char linebuf[256];
			file.getline(linebuf, 256);
		}
		else if (strcmp(buffer, "mtllib") == 0)
		{
			// ignored
			std::string MaterialFile;
			file >> MaterialFile;
		}
		else if (strcmp(buffer, "usemtl") == 0)
		{
			// read Material name
			std::string materialName;
			file >> materialName;
		}
		else if (buffer[0] == 'f')
		{
			// faces
			uint32_t faceIndices[4];
			uint32_t faceIndexCount = 0;

			for (int i = 0; i < 4; ++i)
			{
				VertexKey key;

				file >> key.v;

				if (!file.eof())
				{
					// failed to read another index continue on
					if (file.fail())
					{
						file.clear();
						break;
					}

					if (file.peek() == '/')
					{
						file.ignore();

						if (file.peek() != '/')
						{
							file >> key.vt;
						}

						if (file.peek() == '/')
						{
							file.ignore();
							file >> key.vn;
						}
					}

					// find / add vertex, index
					VertexMap::iterator iter = vertexLookup.find(key);

					if (iter != vertexLookup.end())
					{
						faceIndices[faceIndexCount++] = iter->second;
					}
					else
					{
						// add vertex
						uint32_t newIndex = uint32_t(m->m_positions.size());
						faceIndices[faceIndexCount++] = newIndex;

						vertexLookup.insert(make_pair(key, newIndex));

						// push back vertex data
						assert(key.v > 0);

						m->m_positions.push_back(positions[key.v - 1]);

						// obj format doesn't support mesh colours so add default value
						m->m_colours.push_back(Colour(1.0f, 1.0f, 1.0f));

						// normal [optional]
						if (key.vn)
						{
							m->m_normals.push_back(normals[key.vn - 1]);
						}

						// texcoord [optional]
						if (key.vt)
						{
							m->m_texcoords[0].push_back(texcoords[key.vt - 1]);
						}
					}
				}
			}

			if (faceIndexCount == 3)
			{
				// a triangle
				indices.insert(indices.end(), faceIndices, faceIndices + 3);
			}
			else if (faceIndexCount == 4)
			{
				// a quad, triangulate clockwise
				indices.insert(indices.end(), faceIndices, faceIndices + 3);

				indices.push_back(faceIndices[2]);
				indices.push_back(faceIndices[3]);
				indices.push_back(faceIndices[0]);
			}
			else
			{
				cout << "Face with more than 4 vertices are not supported" << endl;
			}

		}
		else if (buffer[0] == '#')
		{
			// comment
			char linebuf[256];
			file.getline(linebuf, 256);
		}
	}

	// calculate normals if none specified in file
	m->m_normals.resize(m->m_positions.size());

	const uint32_t numFaces = uint32_t(indices.size()) / 3;
	for (uint32_t i = 0; i < numFaces; ++i)
	{
		uint32_t a = indices[i * 3 + 0];
		uint32_t b = indices[i * 3 + 1];
		uint32_t c = indices[i * 3 + 2];

		Point3& v0 = m->m_positions[a];
		Point3& v1 = m->m_positions[b];
		Point3& v2 = m->m_positions[c];

		Vector3 n = SafeNormalize(Cross(v1 - v0, v2 - v0), Vector3(0.0f, 1.0f, 0.0f));

		m->m_normals[a] += n;
		m->m_normals[b] += n;
		m->m_normals[c] += n;
	}

	for (uint32_t i = 0; i < m->m_normals.size(); ++i)
	{
		m->m_normals[i] = SafeNormalize(m->m_normals[i], Vector3(0.0f, 1.0f, 0.0f));
	}

	//cout << "Imported mesh " << path << " in " << (GetSeconds()-startTime)*1000.f << "ms" << endl;

	return m;
}

Mesh* mImportMeshFromObj(const char* path)
{
	ifstream file(path);

	if (!file)
	{
		cout << path << endl;
		return NULL;
	}

	Mesh* m = new Mesh();

	vector<Point3>  positions;
	vector<Vector3> normals;
	vector<Vector2> texcoords;
	vector<Vector3> colors;
	vector<uint32_t>  edge;
	vector<uint32_t>& indices = m->m_indices;

 	typedef map<VertexKey, uint32_t> VertexMap;
	VertexMap vertexLookup;

 	const uint32_t kMaxLineLength = 1024;
	char buffer[kMaxLineLength];

	while (file)
	{
		file >> buffer;

		if (strcmp(buffer, "vn") == 0)
		{
			// normals
			float x, y, z;
			file >> x >> y >> z;

			normals.push_back(Vector3(x, y, z));
		}
		else if (strcmp(buffer, "vt") == 0)
		{
			// texture coords
			float u, v;
			file >> u >> v;

			texcoords.push_back(Vector2(u, v));
		}
		else if (buffer[0] == 'v')
		{
			// positions
			float x, y, z;
			file >> x >> y >> z;

			positions.push_back(Point3(x, y, z));
		}
		else if (buffer[0] == 's' || buffer[0] == 'g' || buffer[0] == 'o')
		{
			// ignore smoothing groups, groups and objects
			char linebuf[256];
			file.getline(linebuf, 256);
		}
		else if (strcmp(buffer, "mtllib") == 0)
		{
			// ignored
			std::string MaterialFile;
			file >> MaterialFile;
		}
		else if (strcmp(buffer, "usemtl") == 0)
		{
			// read Material name
			std::string materialName;
			file >> materialName;
		} 
		else if (buffer[0] == 'f')
		{
			// faces
			uint32_t faceIndices[4];
			uint32_t faceIndexCount = 0;

			for (int i = 0; i < 4; ++i)
			{
				VertexKey key;

				file >> key.v;

				if (!file.eof())
				{
					// failed to read another index continue on
					if (file.fail())
					{
						file.clear();
						break;
					}

					if (file.peek() == '/')
					{
						file.ignore();

						if (file.peek() != '/')
						{
							file >> key.vt;
						}

						if (file.peek() == '/')
						{
							file.ignore();
							file >> key.vn;
						}
					}

					// find / add vertex, index
					VertexMap::iterator iter = vertexLookup.find(key);

					if (iter != vertexLookup.end())
					{
						faceIndices[faceIndexCount++] = iter->second;
					}
					else
					{
						// add vertex
						uint32_t newIndex = uint32_t(m->m_positions.size());
						faceIndices[faceIndexCount++] = newIndex;

						vertexLookup.insert(make_pair(key, newIndex));

						// push back vertex data
						assert(key.v > 0);

						m->m_positions.push_back(positions[key.v - 1]);

						// obj format doesn't support mesh colours so add default value
						m->m_colours.push_back(Colour(1.0f, 1.0f, 1.0f));

						// normal [optional]
						if (key.vn)
						{
							m->m_normals.push_back(normals[key.vn - 1]);
						}

						// texcoord [optional]
						if (key.vt)
						{
							m->m_texcoords[0].push_back(texcoords[key.vt - 1]);
						}
					}
				}
			}
			if (faceIndexCount == 3)
			{
				// a triangle
				indices.insert(indices.end(), faceIndices, faceIndices + 3);
				edge.push_back(faceIndices[0]);
				edge.push_back(faceIndices[1]);
				edge.push_back(faceIndices[1]);
				edge.push_back(faceIndices[2]);
				edge.push_back(faceIndices[2]);
				edge.push_back(faceIndices[0]);
			}
			else if (faceIndexCount == 4)
			{
				// a triangle
				indices.insert(indices.end(), faceIndices, faceIndices + 3);
				edge.push_back(faceIndices[0]);
				edge.push_back(faceIndices[1]);
				edge.push_back(faceIndices[1]);
				edge.push_back(faceIndices[2]);
				edge.push_back(faceIndices[2]);
				edge.push_back(faceIndices[3]);
				edge.push_back(faceIndices[3]);
				edge.push_back(faceIndices[0]);
			}
		}
		else if (buffer[0] == 'l')
		{
			// faces
			uint32_t faceIndices[4];
			uint32_t faceIndexCount = 0;


			for (int i = 0; i < 4; ++i)
			{
				VertexKey key;

				file >> key.v;

				if (!file.eof())
				{
					// failed to read another index continue on
					if (file.fail())
					{
						file.clear();
						break;
					}

					if (file.peek() == '/')
					{
						file.ignore();

						if (file.peek() != '/')
						{
							file >> key.vt;
						}

						if (file.peek() == '/')
						{
							file.ignore();
							file >> key.vn;
						}
					}

					// find / add vertex, index
					VertexMap::iterator iter = vertexLookup.find(key);

					if (iter != vertexLookup.end())
					{
						faceIndices[faceIndexCount++] = iter->second;
					}
					else
					{
						// add vertex
						uint32_t newIndex = uint32_t(m->m_positions.size());
						faceIndices[faceIndexCount++] = newIndex;

						vertexLookup.insert(make_pair(key, newIndex));

						// push back vertex data
						assert(key.v > 0);

						m->m_positions.push_back(positions[key.v - 1]);

						// obj format doesn't support mesh colours so add default value
						m->m_colours.push_back(Colour(1.0f, 1.0f, 1.0f));

						// normal [optional]
						if (key.vn)
						{
							m->m_normals.push_back(normals[key.vn - 1]);
						}

						// texcoord [optional]
						if (key.vt)
						{
							m->m_texcoords[0].push_back(texcoords[key.vt - 1]);
						}
					}
				}
			}
			if (faceIndexCount == 2)
			{
				// a triangle
				indices.insert(indices.end(), faceIndices, faceIndices +3);
				edge.push_back(faceIndices[1]);
				edge.push_back(faceIndices[0]);
				edge.push_back(faceIndices[0]);
				edge.push_back(faceIndices[1]);
			} 
			else
			{
				cout << "Face with more than 4 vertices are not supported" << endl;
			}
	 
		}
		else if (buffer[0] == '#')
		{
			// comment
			char linebuf[256];
			file.getline(linebuf, 256);
		}
	}
 
	cout << "Imported mesh : " << path << endl;
	cout << "num points : " << m->m_positions.size() << endl;

	m->m_indices = edge;
	cout << "edges nums " << edge.size()<<endl;
	cout << "edges nums " << edge[0] << " "<< edge[1] <<" "<<edge[2] << endl;
	return m;

}

inline int GridIndex(int x, int y, int dx) { return y * dx + x; }


void  ofx_nvflex::CreateSpring(int i, int j, float stiffness, float give )
{
	buffers->springIndices.push_back(i);
	buffers->springIndices.push_back(j);
	buffers->springLengths.push_back((1.0f + give)*Length(Vec3(buffers->positions[i]) - Vec3(buffers->positions[j])));
	buffers->springStiffness.push_back(stiffness);
}

void ofx_nvflex::CreateSpringGrid(Vec3 lower, int dx, int dy, int dz, float radius, int phase, float stretchStiffness, float bendStiffness, float shearStiffness, Vec3 velocity, float invMass)
{
	int baseIndex = int(buffers->positions.size());

	for (int z = 0; z < dz; ++z)
	{
		for (int y = 0; y < dy; ++y)
		{
			for (int x = 0; x < dx; ++x)
			{
				Vec3 position = lower + radius * Vec3(float(x), float(z), float(y));

				buffers->positions.push_back(Vec4(position.x, position.z, position.y, invMass));
				buffers->restPositions.push_back(Vec4(position.x, position.y, position.z, invMass));
				buffers->activeIndices.push_back(x+y*dx+z*(dx*dy) );
				buffers->velocities.push_back(velocity);
				buffers->phases.push_back(phase);

				if (x > 0 && y > 0)
				{
					buffers->triangles.push_back(baseIndex + GridIndex(x - 1, y - 1, dx));
					buffers->triangles.push_back(baseIndex + GridIndex(x, y - 1, dx));
					buffers->triangles.push_back(baseIndex + GridIndex(x, y, dx));

					buffers->triangles.push_back(baseIndex + GridIndex(x - 1, y - 1, dx));
					buffers->triangles.push_back(baseIndex + GridIndex(x, y, dx));
					buffers->triangles.push_back(baseIndex + GridIndex(x - 1, y, dx));

					buffers->triangleNormals.push_back(Vec3(0.0f, 1.0f, 0.0f));
					buffers->triangleNormals.push_back(Vec3(0.0f, 1.0f, 0.0f));
				}
			}
		}
	}

	// horizontal
	for (int y = 0; y < dy; ++y)
	{
		for (int x = 0; x < dx; ++x)
		{
			int index0 = y * dx + x;

			if (x > 0)
			{
				int index1 = y * dx + x - 1;
				CreateSpring(baseIndex + index0, baseIndex + index1, stretchStiffness);
			}

			if (x > 1)
			{
				int index2 = y * dx + x - 2;
				CreateSpring(baseIndex + index0, baseIndex + index2, bendStiffness);
			}

			if (y > 0 && x < dx - 1)
			{
				int indexDiag = (y - 1)*dx + x + 1;
				CreateSpring(baseIndex + index0, baseIndex + indexDiag, shearStiffness);
			}

			if (y > 0 && x > 0)
			{
				int indexDiag = (y - 1)*dx + x - 1;
				CreateSpring(baseIndex + index0, baseIndex + indexDiag, shearStiffness);
			}
		}
	}

	// vertical
	for (int x = 0; x < dx; ++x)
	{
		for (int y = 0; y < dy; ++y)
		{
			int index0 = y * dx + x;

			if (y > 0)
			{
				int index1 = (y - 1)*dx + x;
				CreateSpring(baseIndex + index0, baseIndex + index1, stretchStiffness);
			}

			if (y > 1)
			{
				int index2 = (y - 2)*dx + x;
				CreateSpring(baseIndex + index0, baseIndex + index2, bendStiffness);
			}
		}
	}
}

void ofx_nvflex::CreateSpringBrush(Vec3 lower, int dx, int dy, int dz, float radius, int phase, float stretchStiffness, float bendStiffness, float shearStiffness, Vec3 velocity, float invMass)
{

	// create one line 
	int num_points = 35;
	int num_l = 10;
	srand(time(NULL) );

	for (int k = 0; k < num_l; k++)
	{
		for (int j = 0; j < num_l; j++)
		{
			int baseIndex = int(buffers->positions.size());

			for (int i = 0; i < num_points; i++)
			{

				int n = i + j * num_points + k * num_points *num_l; 
				float mix = ((rand() +n) % 100)*0.01;
				float mixb = ((rand() + n+1000) % 100)*0.01;
				Vec3 position = Vec3(float(j*0.2f)+mix*0.2f, float(i + 1)*0.3f, float(k*0.2f)+mixb*0.2f);

				buffers->positions.push_back(Vec4(position.x, position.y, position.z, invMass));
				buffers->restPositions.push_back(Vec4(position.x, position.y, position.z, invMass));
				buffers->activeIndices.push_back(n);
				buffers->velocities.push_back(velocity);
				buffers->phases.push_back(phase);
				cursor++;
				if (i > 0 && i < num_points - 1)
				{
					buffers->triangles.push_back(baseIndex + i - 1);
					buffers->triangles.push_back(baseIndex + i);
					buffers->triangles.push_back(baseIndex + i + 1);
					buffers->triangleNormals.push_back(Vec3(0.0f, 0.0f, 1.0f));
				}

				if (i > 0)
				{
					CreateSpring(baseIndex + i - 1, baseIndex + i, stretchStiffness);
				}

				if (i > 1)
				{
					CreateSpring(baseIndex + i - 2, baseIndex + i , stretchStiffness);
				}
				if (i > 2 )
				{
					CreateSpring(baseIndex + i -3, baseIndex + i , stretchStiffness);
				}
				if (i > 4)
				{
					CreateSpring(baseIndex + i - 5, baseIndex + i, stretchStiffness);
				}
				if (i > 9)
				{
					CreateSpring(baseIndex + i - 10, baseIndex + i, stretchStiffness);
				}
			}
		}
	}

}


void ofx_nvflex::create_softbody(Instance instance, int group)
{
	CreateSpringBrush(Vec3(0.0f,1.0f,0.0f), 70, 70, 1, 1.6f, NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter), 1.0f, 1.0f, 1.0f, Vec3(0.0f, 0.0f, 0.0f), 1.0f);
}

void ofx_nvflex::create_softbody_old(Instance instance, int group)
{
	 
	buffers->springIndices.map();
	buffers->springLengths.map();
	buffers->springStiffness.map();
	
	Mesh* mesh = ImportMeshFromObj(GetFilePathByPlatform(instance.mFile).c_str());
	
	// print mesh data
	cout << " create cloth from Mesh ? " << endl;
	cout << "num faces  " << mesh->GetNumFaces() << endl;
	cout << "num vertices  " << mesh->GetNumVertices() << endl;
	cout << "num indices  " << mesh->m_indices.size() << endl;
	cout << "num pos  " << mesh->m_positions.size() << endl;
	cout << "old importer mode " << endl; 
	NvFlexTriangleMeshId triM = NvFlexCreateTriangleMesh(library);
	//triM->mesh = 
	
	NvFlexExtAsset* asset = NvFlexExtCreateClothFromMesh(
		(float*)&mesh->m_positions[0],
		mesh->m_positions.size(),
		(int*)&mesh->m_indices[0],
		mesh->GetNumFaces(),
		1.0f,
		1.0f,
		0.5f,
		0.0f,
		0.0f);

	cout << "asset done " << endl;
	cout << "asset valid "<< asset->numParticles << endl;
	cout << "asset valid " << asset->numSprings << endl;

	cout << "mesh valid " << mesh->m_positions.size() << endl;
	cout << "mesh valid " << mesh->m_indices.size() << endl;

	// add particle data to solver
	/*
	for (int i = 0; i < mesh->m_positions.size(); ++i)
	{
		buffers->positions.push_back(mesh->m_positions[i]);
		buffers->restPositions.push_back(mesh->m_positions[i]);
		buffers->velocities.push_back(0.0f);

		const int phase = NvFlexMakePhase(group, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter);
		buffers->phases.push_back(phase);
		buffers->activeIndices.push_back(cursor);
		buffers->ids.push_back(cursor);
		buffers->cols.push_back(Vec3(1.0f, 0.0f, 0.0f));// Vec3(mesh->m_colours[i]));
		cursor++;
	}
	std::cout << endl << "set buffers particles  " << mesh->m_positions.size() << endl;
	// buffers->activeIndices.push_back(0);

	// add link data to the solver 
	for (int i = 0; i < mesh->m_indices.size(); i += 2)
	{
		int num = mesh->m_indices[i + 0];
		int numb = mesh->m_indices[i + 1];

		buffers->springIndices.push_back(num);
		buffers->springIndices.push_back(numb);
		//buffers->activeIndices.push_back(cursor);
		//cursor++;
		ofVec3f pos(buffers->positions[num].x, buffers->positions[num].y, buffers->positions[num].z);
		ofVec3f posb(buffers->positions[numb].x, buffers->positions[numb].y, buffers->positions[numb].z);
		float len = (pos - posb).length();
		buffers->springLengths.push_back(len+0.01f); //len+0.05f 
		buffers->springStiffness.push_back(1.0f);

	}*/
	
	for (int i = 0; i < asset->numParticles; ++i)
	{
		//Vec4 pos(asset->particles[i * 4], asset->particles[i * 4 + 1], asset->particles[i * 4 + 2], 1.0f);
		buffers->positions.push_back(mesh->m_positions[i]);
		buffers->restPositions.push_back(mesh->m_positions[i]);
		buffers->velocities.push_back(0.0f) ;
		
		const int phase = NvFlexMakePhase(group, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter);
		buffers->phases.push_back(phase);
		buffers->activeIndices.push_back(cursor);
		buffers->ids.push_back(cursor);
		buffers->cols.push_back(Vec3(1.0f, 0.0f, 0.0f));		// Vec3(mesh->m_colours[i]));
		cursor++;
	}

	std::cout << endl << "set buffers particles  " << mesh->m_positions.size() << endl;

	// add link data to the solver 
	for (int i = 0; i <asset->numSprings; i ++)
	{
		buffers->springIndices.push_back(asset->springIndices[i*2]);
		buffers->springIndices.push_back(asset->springIndices[i*2+1]);
		buffers->springLengths.push_back(asset->springRestLengths[i]); 
		buffers->springStiffness.push_back(asset->springCoefficients[i]);

	}
	
	std::cout << endl << "set spring buffers" << buffers->springIndices.size()*0.5 << " " << buffers->springIndices.size() << endl;
	std::cout << endl << "set spring buffers" << buffers->springStiffness.size() << endl;
	std::cout << endl << "set spring buffers" << buffers->springLengths.size() << endl;

	/*
	for (int i = 0; i < buffers->springIndices.size(); i+=2)
	{
		std::cout<< buffers->springIndices[i] <<"  " <<buffers->springIndices[i+0] << endl;
	}*/

	std::cout << "pos size"<< buffers->positions.size() << endl;
	std::cout << endl << "cloth created from obj"<<endl;
}

 

void  ofx_nvflex::emit_particles(float x, float y, float dirx, float diry, float odx, float ody, float rate, Vec3 c) {

	float invMass = 1 / 1.0 ;
 
	if (cursor >= maxParticles - 1000)
	{
		// shift
		int num = buffers->positions.size();
		for (int i = 5000; i < num ; ++i)
		{
			if (i < num - 1000)
			{
				buffers->positions[i] = buffers->positions[i + 1000];
				buffers->velocities[i] = buffers->velocities[i + 1000];
				buffers->phases[i] = buffers->phases[i + 1000];
				buffers->ids[i] = buffers->ids[i+1000] ;
				buffers->cols[i] = buffers->cols[i+1000];
			}	 
		}

		buffers->positions.resize(num - 1000);
		buffers->velocities.resize(num - 1000);
		buffers->phases.resize(num - 1000);
		buffers->activeIndices.resize(num - 1000);
		buffers->ids.resize(num - 1000);
		buffers->cols.resize(num - 1000);
		cursor -= 1000;
	} 
	
	int phase =  NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid);
	int nump = 10 * rate; 

	for (int i = 0; i < nump; i++)
	{
		srand(time(NULL) + i);
		float mix = ((rand() + 2000) % 100)*0.01;

		Vec3 position = Vec3(float(x + 0.05*(rand() % 100 - 50)), float(0.5f), float(y + 0.05*((rand() + 10) % 100 - 50))) - ((Vec3(dirx, 0, diry))*mix * 1); // +RandomUnitVector() * 4;
		Vec3 velocity = Vec3((dirx*(0.6) + odx * (0.4)) * 5 + 0.002*((rand() + 100) % 100 - 50), 0, (diry*(0.6) + ody * (0.4)) * 5 + 0.002*((rand() + 200) % 100 - 50))*(((rand() + 4000) % 100)*0.01); // +RandomUnitVector() * 4;;

		if (cursor < maxParticles - 1) {
			buffers->positions.push_back(Vec4(position.x, position.y, position.z, invMass));
			buffers->ids.push_back( (rand() + 2000) % 1000);
			buffers->cols.push_back(c);
			buffers->velocities.push_back(velocity);
			buffers->phases.push_back(phase);
			buffers->activeIndices.push_back(cursor);
			cursor++;		 
		}
	 
	}
};



void ofx_nvflex::update(int s )
{
	if (DEBUG_MODE)
	{
		cout << "update  " << endl;
	}

	/*
	if (s)
	{
		// setup spring buffers 
		if (buffers->springIndices.size())
		{
			indicesBuffer = NvFlexAllocBuffer(library, buffers->springIndices.size(), sizeof(int), eNvFlexBufferHost);
			lengthsBuffer = NvFlexAllocBuffer(library, buffers->springLengths.size(), sizeof(float), eNvFlexBufferHost);
			coefficientsBuffer = NvFlexAllocBuffer(library, buffers->springStiffness.size(), sizeof(float), eNvFlexBufferHost);

			int *springIndices = new int[buffers->springIndices.size()];
			float *springLengths = new float[buffers->springLengths.size()];
			float *springCoefficients = new float[buffers->springStiffness.size()];
 
			for (int i = 0; i < buffers->springIndices.size(); i++)
			{
				springIndices[i] = buffers->springIndices[i];
			}
			for (int i = 0; i < buffers->springLengths.size(); i++)
			{
				springLengths[i] = buffers->springLengths[i];
				springCoefficients[i] = buffers->springStiffness[i];
			} 

			memcpy(NvFlexMap(indicesBuffer, eNvFlexMapWait), springIndices, buffers->springIndices.size());
			memcpy(NvFlexMap(lengthsBuffer, eNvFlexMapWait), springLengths, buffers->springLengths.size());
			memcpy(NvFlexMap(coefficientsBuffer, eNvFlexMapWait), springCoefficients, buffers->springStiffness.size());

		}
	}*/

	
	buffers->UnmapBuffers();

	if (DEBUG_MODE)
	{
		cout << "unmap " << endl;
	}

	NvFlexCopyDesc copyDesc;
	copyDesc.dstOffset = 0;
	copyDesc.srcOffset = 0;
 
	if (buffers->activeIndices.size()) {

		copyDesc.elementCount =  buffers->activeIndices.size();
		NvFlexSetActive( solver,  buffers->activeIndices.buffer, &copyDesc);
		NvFlexSetActiveCount( solver,  buffers->activeIndices.size());
	}
	if (DEBUG_MODE)
	{
		cout << "set indices " << endl;
	}

	if (buffers->positions.size()) {

		copyDesc.elementCount = buffers->positions.size();
		NvFlexSetParticles(solver, buffers->positions.buffer, &copyDesc);
		NvFlexSetVelocities(solver, buffers->velocities.buffer, &copyDesc);
		NvFlexSetPhases(solver, buffers->phases.buffer, &copyDesc);
	}
	if (DEBUG_MODE)
	{
		cout << "set pos " << endl;
	}
	
	if (s)
	{
		if (buffers->springIndices.size())
		{
			if (buffers->springIndices.size())
			{
				assert((buffers->springIndices.size() & 1) == 0);
				assert((buffers->springIndices.size() / 2) == g_buffers->springLengths.size());

				NvFlexSetSprings(solver, buffers->springIndices.buffer, buffers->springLengths.buffer, buffers->springStiffness.buffer, buffers->springLengths.size());
			}
			/*
			if (DEBUG_MODE)
			{
				cout << "unmap sping buffers try" << endl;
			}

			NvFlexUnmap(indicesBuffer)  ;
			NvFlexUnmap(lengthsBuffer)  ;
			NvFlexUnmap(coefficientsBuffer) ;

			if (DEBUG_MODE)
			{
				cout << "set spring solver" << endl;
				cout << "solver to set " << buffers->springIndices.size() << endl;
				cout << "solver to set " << buffers->springStiffness.size() << endl;
			}
			
			NvFlexSetSprings(solver, indicesBuffer, lengthsBuffer, coefficientsBuffer, buffers->springStiffness.size());
			if (DEBUG_MODE)
			{
				cout << "solver set " << buffers->springStiffness.size() << endl;
			}*/
		}
	}

    /*
	if (buffers->shapeFlags.size()) {
		NvFlexSetShapes(
			solver,
			buffers->shapeGeometry.buffer,
			buffers->shapePositions.buffer,
			buffers->shapeRotations.buffer,
			buffers->shapePrevPositions.buffer,
			buffers->shapePrevRotations.buffer,
			buffers->shapeFlags.buffer,
			buffers->shapeFlags.size());
	}
	*/

	if (DEBUG_MODE)
	{
		cout << "set shapes " << endl;
	}

	 NvFlexSetParams(solver, &g_params);

	 if (DEBUG_MODE)
	 {
		 cout << "set params " << endl;
	 }
	 NvFlexUpdateSolver(solver, 0.04, numSubsteps, profile);

	 if (DEBUG_MODE)
	 {
		 cout << "update solver  " << endl;
	 }

	if (buffers->positions.size()) {
		copyDesc.elementCount = buffers->positions.size();
		NvFlexGetParticles(solver, buffers->positions.buffer, &copyDesc);
		NvFlexGetVelocities(solver, buffers->velocities.buffer, &copyDesc);
	}
	if (DEBUG_MODE)
	{
		cout << "NvFlexGetParticles " << endl;
	}
	 
	
	if (0)
	{
		if (buffers->springIndices.size())
		{
			NvFlexGetSprings(solver, indicesBuffer, lengthsBuffer, coefficientsBuffer, buffers->springStiffness.size());

			/*buffers->springIndices.map();
			buffers->springLengths.map();
			buffers->springStiffness.map();*/

			if (DEBUG_MODE)
			{
				cout << "solver spring get " << endl;
			}
			int *springIndices = new int[buffers->springIndices.size()];
			float *springLengths = new float[buffers->springLengths.size()];
			float *springCoefficients = new float[buffers->springStiffness.size()];

			memcpy(springIndices, NvFlexMap(indicesBuffer, eNvFlexMapWait), buffers->springIndices.size());
			memcpy(springLengths, NvFlexMap(lengthsBuffer, eNvFlexMapWait), buffers->springLengths.size());
			memcpy(springCoefficients, NvFlexMap(coefficientsBuffer, eNvFlexMapWait), buffers->springStiffness.size());

			if (DEBUG_MODE)
			{
				cout << "copy back spring" << endl;
			}

			for (int i = 0; i < buffers->springIndices.size(); i++)
			{
				buffers->springIndices[i] = springIndices[i];
			}
			if (DEBUG_MODE)
			{
				cout << "indices back to buffer " << endl;
			}

			for (int i = 0; i < buffers->springLengths.size(); i++)
			{
				buffers->springLengths[i] = springLengths[i];
				buffers->springStiffness[i] = springCoefficients[i];
			}

			if (DEBUG_MODE)
			{
				cout << "value back to buffer " << endl;
			}

		}
	}
	 
}


void ofx_nvflex::set_params(float cohesion, float adhesion, float surfaceTension, float vorticityConfinement, float smoothing, float viscosity, float size, float g)
{
	// sim params
	g_params.gravity[0] = 0.0f;
	g_params.gravity[1] = g*1.5f;
	g_params.gravity[2] = 0.0f;

	g_params.wind[0] = 0.0f;
	g_params.wind[1] = 0.0f;
	g_params.wind[2] = 0.0f;
 
	g_params.radius = size;
	g_params.viscosity = viscosity;
	g_params.dynamicFriction = 0.45f;
	g_params.staticFriction = 0.45f;
	g_params.particleFriction = 0.45f;		// scale friction between particles by default
	g_params.freeSurfaceDrag = 0.0f;
	g_params.drag = 0.05f;
	g_params.lift = 0.0f;
	g_params.numIterations = 3;
	g_params.fluidRestDistance = g_params.radius*0.7;
	g_params.solidRestDistance = 0.0f;
	
	g_params.anisotropyScale = 1.0f;
	g_params.anisotropyMin = 0.1f;
	g_params.anisotropyMax = 2.0f;
	g_params.smoothing = smoothing;

	g_params.dissipation = 0.0f;
	g_params.damping = 0.0f;
	g_params.particleCollisionMargin = 0.015f;
	g_params.shapeCollisionMargin = 0.01f;
	g_params.collisionDistance = size*0.5f;
	 
	g_params.sleepThreshold = 0.0f;
	g_params.shockPropagation = 0.1f;
	g_params.restitution = 0.01f;

	g_params.maxSpeed = FLT_MAX;
	g_params.maxAcceleration = 500.0f;	// approximately 10x gravity
	
	g_params.relaxationMode = eNvFlexRelaxationGlobal;
	g_params.relaxationFactor = 0.25f;
	g_params.solidPressure = 1.0f;
	g_params.adhesion = adhesion;
	g_params.cohesion = cohesion;
	g_params.surfaceTension = surfaceTension;
	g_params.vorticityConfinement = vorticityConfinement*150.0f;
	g_params.buoyancy = 1.0f;
	g_params.diffuseThreshold = 100.0f;
	g_params.diffuseBuoyancy = 1.0f;
	g_params.diffuseDrag = 0.8f;
	g_params.diffuseBallistic = 16;
 
	g_params.diffuseLifetime = 2.0f;

	g_params.numPlanes = 1;
	(Vec4&)g_params.planes[0] = Vec4(0.0f, 1.0f, 0.0f, 0.0f);

 	//g_params.collisionDistance = 0.05f;
	//g_params.shapeCollisionMargin = 0.00001f;
}


// buffers 
SimBuffers::SimBuffers(NvFlexLibrary* l) :
	positions(l), ids(1), cols(1), restPositions(l), velocities(l), phases(l), densities(l),
	anisotropy1(l), anisotropy2(l), anisotropy3(l), normals(l), smoothPositions(l),
	diffusePositions(l), diffuseVelocities(l), diffuseIndices(l), activeIndices(l),
	shapeGeometry(l), shapePositions(l), shapeRotations(l), shapePrevPositions(l),
	shapePrevRotations(l), shapeFlags(l), rigidOffsets(l), rigidIndices(l), rigidMeshSize(l),
	rigidCoefficients(l), rigidRotations(l), rigidTranslations(l),
	rigidLocalPositions(l), rigidLocalNormals(l), inflatableTriOffsets(l),
	inflatableTriCounts(l), inflatableVolumes(l), inflatableCoefficients(l),
	inflatablePressures(l), springIndices(l), springLengths(l),
	springStiffness(l), triangles(l), triangleNormals(l), uvs(l) {


}

SimBuffers::~SimBuffers() {
	positions.destroy();
	restPositions.destroy();
	velocities.destroy();
	phases.destroy();
	densities.destroy();
	anisotropy1.destroy();
	anisotropy2.destroy();
	anisotropy3.destroy();
	normals.destroy();
	diffusePositions.destroy();
	diffuseVelocities.destroy();
	diffuseIndices.destroy();
	smoothPositions.destroy();
	activeIndices.destroy();

	// convexes
	shapeGeometry.destroy();
	shapePositions.destroy();
	shapeRotations.destroy();
	shapePrevPositions.destroy();
	shapePrevRotations.destroy();
	shapeFlags.destroy();

	// rigids
	rigidOffsets.destroy();
	rigidIndices.destroy();
	rigidMeshSize.destroy();
	rigidCoefficients.destroy();
	rigidRotations.destroy();
	rigidTranslations.destroy();
	rigidLocalPositions.destroy();
	rigidLocalNormals.destroy();

	// springs
	springIndices.destroy();
	springLengths.destroy();
	springStiffness.destroy();

	// inflatables
	inflatableTriOffsets.destroy();
	inflatableTriCounts.destroy();
	inflatableVolumes.destroy();
	inflatableCoefficients.destroy();
	inflatablePressures.destroy();

	// triangles
	triangles.destroy();
	triangleNormals.destroy();
	uvs.destroy();
}

void SimBuffers::MapBuffers() {

	positions.map();
	restPositions.map();
	velocities.map();
	phases.map();
	activeIndices.map();

	/*densities.map();
	anisotropy1.map();
	anisotropy2.map();
	anisotropy3.map();
	normals.map();
	diffusePositions.map();
	diffuseVelocities.map();
	diffuseIndices.map();
	smoothPositions.map();*/


	// convexes
	/*shapeGeometry.map();
	shapePositions.map();
	shapeRotations.map();
	shapePrevPositions.map();
	shapePrevRotations.map();
	shapeFlags.map();*/

	/*rigidOffsets.map();
	rigidIndices.map();
	rigidMeshSize.map();
	rigidCoefficients.map();
	rigidRotations.map();
	rigidTranslations.map();
	rigidLocalPositions.map();
	rigidLocalNormals.map();*/

	springIndices.map();
	springLengths.map();
	springStiffness.map();

	// inflatables
	/*inflatableTriOffsets.map();
	inflatableTriCounts.map();
	inflatableVolumes.map();
	inflatableCoefficients.map();
	inflatablePressures.map();*/

	triangles.map();
	triangleNormals.map();
	uvs.map();
}

void SimBuffers::UnmapBuffers() {

	// particles
	positions.unmap();
	restPositions.unmap();
	velocities.unmap();
	phases.unmap();
	activeIndices.unmap();

	/*densities.unmap();
	anisotropy1.unmap();
	anisotropy2.unmap();
	anisotropy3.unmap();
	normals.unmap();
	diffusePositions.unmap();
	diffuseVelocities.unmap();
	diffuseIndices.unmap();
	smoothPositions.unmap();*/


	// convexes
	/*shapeGeometry.unmap();
	shapePositions.unmap();
	shapeRotations.unmap();
	shapePrevPositions.unmap();
	shapePrevRotations.unmap();
	shapeFlags.unmap();*/

	// rigids
	/*rigidOffsets.unmap();
	rigidIndices.unmap();
	rigidMeshSize.unmap();
	rigidCoefficients.unmap();
	rigidRotations.unmap();
	rigidTranslations.unmap();
	rigidLocalPositions.unmap();
	rigidLocalNormals.unmap();*/

	// springs
	springIndices.unmap();
	springLengths.unmap();
	springStiffness.unmap();

	// inflatables
	/*inflatableTriOffsets.unmap();
	inflatableTriCounts.unmap();
	inflatableVolumes.unmap();
	inflatableCoefficients.unmap();
	inflatablePressures.unmap();*/

	// triangles
	triangles.unmap();
	triangleNormals.unmap();
	uvs.unmap();
}

void SimBuffers::InitBuffers() {

	positions.resize(0);
	velocities.resize(0);
	phases.resize(0);
	shapeGeometry.resize(0);
	shapePositions.resize(0);
	shapeRotations.resize(0);
	shapePrevPositions.resize(0);
	shapePrevRotations.resize(0);
	shapeFlags.resize(0);
	// rigids
	rigidOffsets.resize(0);
	rigidIndices.resize(0);
	rigidMeshSize.resize(0);
	rigidCoefficients.resize(0);
	rigidRotations.resize(0);
	rigidTranslations.resize(0);
	rigidLocalPositions.resize(0);
	rigidLocalNormals.resize(0);
}


// constructor/destructor
ofx_nvflex::ofx_nvflex()
{
	maxParticles = 100000;
	g_maxDiffuseParticles = 0;
	numDiffuse = 0;
	n = 50;
	solver = NULL;
	profile = false;
	numSubsteps =  8;
	cursor = 0;
}

ofx_nvflex::~ofx_nvflex()
{
	if (solver)
	{
		if (buffers)
			delete buffers;

		NvFlexDestroySolver(solver);
		NvFlexShutdown(library);
	}

	NvFlexDeviceDestroyCudaContext();
}