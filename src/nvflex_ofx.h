#pragma once
#include "ofMain.h" 
#include <NvFlex.h>
#include <vector>
#include <core/types.h>
#include <core/maths.h>
#include <core/platform.h>
#include <core/perlin.h>
#include <NvFlexExt.h>
#include <NvFlexDevice.h>

#include <stddef.h>
#include "vector_types.h"

struct SimBuffers
{
	NvFlexVector<Vec4> positions;
	vector<int> ids;
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

class ofx_nvflex {

public :

	ofx_nvflex();
	~ofx_nvflex();

	void init_flex();
	void emit_particles(float x, float y, float dirx, float diry,float odx, float ody, float rate);

	void update(float x, float y,float dirx, float diry, float odx, float ody,float rate);
	void updateb();

	void set_params(float cohesion, float adhesion, float surfaceTension, float vorticityConfinement, float smoothing, float viscosity, float size);

	NvFlexLibrary* library;  
	NvFlexSolver* solver;  
	NvFlexSolverDesc g_solverDesc;
	SimBuffers*  buffers;
	NvFlexParams g_params;

	int maxParticles ;
	int g_maxDiffuseParticles ;
	int numDiffuse;
	int n ; // obsolete
	int cursor;
	int g_maxNeighborsPerParticle;
	int g_maxContactsPerParticle;
	int activeParticles;

	bool profile;
	int numSubsteps;
	float dt;
	int x, y; 
};

