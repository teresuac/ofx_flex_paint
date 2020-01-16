#pragma once


#include <NvFlex.h>
#include <vector>
#include <core/core.h>
#include <core/types.h>
#include <core/maths.h>
#include <core/platform.h>
#include <core/perlin.h>
#include <NvFlexExt.h>
#include <NvFlexDevice.h>
#include <core/mesh.h>
#include <stddef.h>
//#include <vector_types.h>

struct SimBuffers
{
	NvFlexVector<Vec4> positions;
	vector<int> ids;
	vector<Vec3> cols;
	NvFlexVector<Vec4> restPositions;
	NvFlexVector<Vec3> velocities;
	NvFlexVector<int> phases;
	NvFlexVector<float> densities;
	NvFlexVector<Vec4> anisotropy1;
	NvFlexVector<Vec4> anisotropy2;
	NvFlexVector<Vec4> anisotropy3;
	NvFlexVector<Vec4> normals;
	NvFlexVector<Vec4> smoothPositions;
	NvFlexVector<Vec4> diffusePositions;
	NvFlexVector<Vec4> diffuseVelocities;
	NvFlexVector<int> diffuseIndices;
	NvFlexVector<int> activeIndices;

	// convexes
	NvFlexVector<NvFlexCollisionGeometry> shapeGeometry;
	NvFlexVector<Vec4> shapePositions;
	NvFlexVector<Quat> shapeRotations;
	NvFlexVector<Vec4> shapePrevPositions;
	NvFlexVector<Quat> shapePrevRotations;
	NvFlexVector<int> shapeFlags;

	// rigids
	NvFlexVector<int> rigidOffsets;
	NvFlexVector<int> rigidIndices;
	NvFlexVector<int> rigidMeshSize;
	NvFlexVector<float> rigidCoefficients;
	NvFlexVector<Quat> rigidRotations;
	NvFlexVector<Vec3> rigidTranslations;
	NvFlexVector<Vec3> rigidLocalPositions;
	NvFlexVector<Vec4> rigidLocalNormals;

	// inflatables
	NvFlexVector<int> inflatableTriOffsets;
	NvFlexVector<int> inflatableTriCounts;
	NvFlexVector<float> inflatableVolumes;
	NvFlexVector<float> inflatableCoefficients;
	NvFlexVector<float> inflatablePressures;

	// springs
	NvFlexVector<int> springIndices;
	NvFlexVector<float> springLengths;
	NvFlexVector<float> springStiffness;

	NvFlexVector<int> triangles;
	NvFlexVector<Vec3> triangleNormals;
	NvFlexVector<Vec3> uvs;


	SimBuffers(NvFlexLibrary* l);
	~SimBuffers();

	void MapBuffers();
	void UnmapBuffers();
	void InitBuffers();

};

struct Instance
{
	Instance(const char* mesh) :
		mFile(mesh),
		mColor(0.5f, 0.5f, 1.0f),

		mScale(1.0f),
		mTranslation(0.0f, 0.0f, 0.0f),
		stretchStiffness(1.0f),
		bendStiffness(0.5f),
		tetherStiffness(0.0f),
		tetherGive(0.0f),
		pressure(0.0f)
		
	{}

	const char* mFile;
	Vec3 mColor;

	Vec3 mScale;
	Vec3 mTranslation;

	float stretchStiffness;
	float bendStiffness;
	float tetherStiffness;
	float tetherGive;
	float pressure ;
	/*
	float mClusterSpacing;
	float mClusterRadius;
	float mClusterStiffness;

	float mLinkRadius;
	float mLinkStiffness;

	float mGlobalStiffness;

	float mSurfaceSampling;
	float mVolumeSampling;

	float mSkinningFalloff;
	float mSkinningMaxDistance;

	float mClusterPlasticThreshold;
	float mClusterPlasticCreep;*/
};

struct RenderingInstance
{
	Mesh* mMesh;
	std::vector<int> mSkinningIndices;
	std::vector<float> mSkinningWeights;
	std::vector<Vec3> mRigidRestPoses;
	Vec3 mColor;
	int mOffset;
};


// read OBJ to mesh
struct VertexKey
{
	VertexKey() : v(0), vt(0), vn(0) {}

	uint32_t v, vt, vn;

	bool operator == (const VertexKey& rhs) const
	{
		return v == rhs.v && vt == rhs.vt && vn == rhs.vn;
	}

	bool operator < (const VertexKey& rhs) const
	{
		if (v != rhs.v)
			return v < rhs.v;
		else if (vt != rhs.vt)
			return vt < rhs.vt;
		else
			return vn < rhs.vn;
	}
};