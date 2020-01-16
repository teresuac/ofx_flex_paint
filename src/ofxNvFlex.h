#pragma once

#include "ofMain.h" 
#include "ofxNvFlex_data.h"
#include "NvFlexExt.h"
#include "core/cloth.h"
#include "ofMath.h"

class ofx_nvflex {

public :

	ofx_nvflex();
	~ofx_nvflex();

	void init_flex();
	//void init_brush();
	
	void emit_particles(float x, float y, float dirx, float diry,float odx, float ody, float rate, Vec3 c);
	void update(int s);
	void set_params(float cohesion, float adhesion, float surfaceTension, float vorticityConfinement, float smoothing, float viscosity, float size,float g);
	void CreateSpringGrid(Vec3 lower, int dx, int dy, int dz, float radius, int phase, float stretchStiffness, float bendStiffness, float shearStiffness, Vec3 velocity, float invMass);
	void CreateSpring(int i, int j, float stiffness, float give = 0.0f);
	void create_softbody_old(Instance instance, int group);
	void CreateSpringBrush(Vec3 lower, int dx, int dy, int dz, float radius, int phase, float stretchStiffness, float bendStiffness, float shearStiffness, Vec3 velocity, float invMass);

	NvFlexLibrary* library;  
	NvFlexSolver* solver;  
	NvFlexSolverDesc g_solverDesc;
	SimBuffers*  buffers;
	NvFlexParams g_params;
	NvFlexCollisionGeometry geo; 

	int maxParticles ;
	int g_maxDiffuseParticles ;
	int numDiffuse;
	int n ; 
	int cursor;
	int g_maxNeighborsPerParticle;
	int g_maxContactsPerParticle;
	int activeParticles;

	bool profile;
	int numSubsteps;
	float dt;
	int x, y; 
	float mRadius; 

	NvFlexBuffer *indicesBuffer; // = NvFlexAllocBuffer(library, buffers->springIndices.size(), sizeof(int), eNvFlexBufferHost);
	NvFlexBuffer *lengthsBuffer; //  = NvFlexAllocBuffer(library, buffers->springLengths.size(), sizeof(float), eNvFlexBufferHost);
	NvFlexBuffer *coefficientsBuffer; // 

	std::vector<Instance> mInstances;
	void create_softbody(Instance instance, int group=0 );

private:
	std::vector<RenderingInstance> mRenderingInstances;
	bool plasticDeformation;

};



