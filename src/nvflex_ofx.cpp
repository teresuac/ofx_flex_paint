#include "nvflex_ofx.h"
#include <stdlib.h>

/*	positions.resize(0);
velocities.resize(0);
phases.resize(0);
*/

// buffers 
SimBuffers::SimBuffers(NvFlexLibrary* l) :
	positions(l),ids(1), restPositions(l), velocities(l), phases(l), densities(l),
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
	densities.map();
	anisotropy1.map();
	anisotropy2.map();
	anisotropy3.map();
	normals.map();
	diffusePositions.map();
	diffuseVelocities.map();
	diffuseIndices.map();
	smoothPositions.map();
	activeIndices.map();

	// convexes
	shapeGeometry.map();
	shapePositions.map();
	shapeRotations.map();
	shapePrevPositions.map();
	shapePrevRotations.map();
	shapeFlags.map();

	rigidOffsets.map();
	rigidIndices.map();
	rigidMeshSize.map();
	rigidCoefficients.map();
	rigidRotations.map();
	rigidTranslations.map();
	rigidLocalPositions.map();
	rigidLocalNormals.map();

	springIndices.map();
	springLengths.map();
	springStiffness.map();

	// inflatables
	inflatableTriOffsets.map();
	inflatableTriCounts.map();
	inflatableVolumes.map();
	inflatableCoefficients.map();
	inflatablePressures.map();

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
	densities.unmap();
	anisotropy1.unmap();
	anisotropy2.unmap();
	anisotropy3.unmap();
	normals.unmap();
	diffusePositions.unmap();
	diffuseVelocities.unmap();
	diffuseIndices.unmap();
	smoothPositions.unmap();
	activeIndices.unmap();

	// convexes
	shapeGeometry.unmap();
	shapePositions.unmap();
	shapeRotations.unmap();
	shapePrevPositions.unmap();
	shapePrevRotations.unmap();
	shapeFlags.unmap();

	// rigids
	rigidOffsets.unmap();
	rigidIndices.unmap();
	rigidMeshSize.unmap();
	rigidCoefficients.unmap();
	rigidRotations.unmap();
	rigidTranslations.unmap();
	rigidLocalPositions.unmap();
	rigidLocalNormals.unmap();

	// springs
	springIndices.unmap();
	springLengths.unmap();
	springStiffness.unmap();

	// inflatables
	inflatableTriOffsets.unmap();
	inflatableTriCounts.unmap();
	inflatableVolumes.unmap();
	inflatableCoefficients.unmap();
	inflatablePressures.unmap();

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
}

//error
void ErrorCallback(NvFlexErrorSeverity, const char* msg, const char* file, int line)
{
	printf("Flex: %s - %s:%d\n", msg, file, line);
}

// constructor/destructor
ofx_nvflex::ofx_nvflex()
{
	
	maxParticles = 50000;
	g_maxDiffuseParticles = 0;
	numDiffuse = 0;
	n = 50; 
	solver = NULL;
	profile=false;
	numSubsteps=1;

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

	library = NvFlexInit(NV_FLEX_VERSION, ErrorCallback, &desc);

	cursor = 0;
	/*
	// clean
	if (solver)
	{

		if (g_buffers)
			delete g_buffers;

		NvFlexDestroySolver(g_solver);
		g_solver = NULL;

		if (g_triangleCollisionMesh) {
			delete g_triangleCollisionMesh;
			g_triangleCollisionMesh = NULL;
		}

	}*/

	// buffer : new / map / init
	buffers = new SimBuffers(library);
	buffers->MapBuffers();
	buffers->InitBuffers();
	 
	 

//	set_params();

	NvFlexSetSolverDescDefaults(&g_solverDesc);

	//ClearShapes();


	maxParticles = 50000;

	g_maxDiffuseParticles = 0;
	g_maxNeighborsPerParticle = 196;
	g_maxContactsPerParticle = 8;

	g_solverDesc.featureMode = eNvFlexFeatureModeSimpleFluids;
	g_solverDesc.maxParticles = maxParticles;
	g_solverDesc.maxDiffuseParticles = 0;
	g_solverDesc.maxNeighborsPerParticle = g_maxNeighborsPerParticle;
	g_solverDesc.maxContactsPerParticle = g_maxContactsPerParticle;

	solver = NvFlexCreateSolver(library, &g_solverDesc);


}


void  ofx_nvflex::emit_particles(float x, float y, float dirx, float diry, float odx, float ody, float rate) {

	float invMass = 1 / 1.0 ;

	if (cursor >= maxParticles - 1000)
	{
		// shift
		int num = buffers->positions.size();
		for (int i = 0; i < num ; ++i)
		{
			if (i < num - 1000)
			{
				buffers->positions[i] = buffers->positions[i + 1000];
				buffers->velocities[i] = buffers->velocities[i + 1000];
				buffers->phases[i] = buffers->phases[i + 1000];
				buffers->ids[i] = buffers->ids[i+1000] ;
			}	 
		}

		buffers->positions.resize(num - 1000);
		buffers->velocities.resize(num - 1000);
		buffers->phases.resize(num - 1000);
		buffers->activeIndices.resize(num - 1000);
		buffers->ids.resize(num - 1000);
		cursor -= 1000;
	}
	
	int phase =  NvFlexMakePhase(0, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid);
	int nump = 400 * rate; 
	for (int i = 0; i < nump; i++)
	{
		srand(time(NULL) + i);
		float mix = ((rand() + 2000) % 100)*0.01;

		Vec3 position = Vec3(float(x + 0.05*(rand() % 100 - 50)), float(0), float(y + 0.05*((rand() + 10) % 100 - 50))) - ((Vec3(dirx, 0, diry))*mix * 1); // +RandomUnitVector() * 4;

		Vec3 velocity = Vec3((dirx*(0.6) + odx * (0.4)) * 5 + 0.002*((rand() + 100) % 100 - 50), 0, (diry*(0.6) + ody * (0.4)) * 5 + 0.002*((rand() + 200) % 100 - 50))*(((rand() + 4000) % 100)*0.01); // +RandomUnitVector() * 4;;

		if (cursor < maxParticles - 1) {
				buffers->positions.push_back(Vec4(position.x, position.y, position.z, invMass));
				buffers->ids.push_back( (rand() + 2000) % 1000);
				buffers->velocities.push_back(velocity);
				buffers->phases.push_back(phase);
				buffers->activeIndices.push_back(cursor);
				cursor++;		 
		}
	 
	}
};

void ofx_nvflex::update(float x, float y, float dirx, float diry, float odx, float ody, float rate)
{
	activeParticles = NvFlexGetActiveCount(solver);

	buffers->MapBuffers();

	emit_particles(x, y,dirx,diry, odx, ody,rate);

	//render 
}

void ofx_nvflex::updateb( )
{

	buffers->UnmapBuffers();

	NvFlexCopyDesc copyDesc;
	copyDesc.dstOffset = 0;
	copyDesc.srcOffset = 0;

	 
	if (buffers->activeIndices.size()) {

		copyDesc.elementCount =  buffers->activeIndices.size();

		NvFlexSetActive( solver,  buffers->activeIndices.buffer, &copyDesc);
		NvFlexSetActiveCount( solver,  buffers->activeIndices.size());
	}
	 

	if (buffers->positions.size()) {

		copyDesc.elementCount = buffers->positions.size();

		NvFlexSetParticles(solver, buffers->positions.buffer, &copyDesc);
		NvFlexSetVelocities(solver, buffers->velocities.buffer, &copyDesc);
		NvFlexSetPhases(solver, buffers->phases.buffer, &copyDesc);

	}

	//setShapes();
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

	 NvFlexSetParams(solver, &g_params);
	 NvFlexUpdateSolver(solver, 0.04, numSubsteps, profile);

	if (buffers->positions.size()) {
		copyDesc.elementCount = buffers->positions.size();
		NvFlexGetParticles(solver, buffers->positions.buffer, &copyDesc);
		NvFlexGetVelocities(solver, buffers->velocities.buffer, &copyDesc);
	}

	activeParticles = NvFlexGetActiveCount(solver);
}


void ofx_nvflex::set_params(float cohesion, float adhesion, float surfaceTension, float vorticityConfinement, float smoothing, float viscosity, float size)
{
	// sim params
	g_params.gravity[0] = 0.0f;
	g_params.gravity[1] = -9.8f;
	g_params.gravity[2] = 0.0f;

	g_params.wind[0] = 0.0f;
	g_params.wind[1] = 0.0f;
	g_params.wind[2] = 0.0f;
 
	g_params.radius = size;
	g_params.viscosity = viscosity;
	g_params.dynamicFriction = 0.1f;
	g_params.staticFriction = 0.0f;
	g_params.particleFriction = 0.2f;		// scale friction between particles by default
	g_params.freeSurfaceDrag = 0.0f;
	g_params.drag = 113.20f;
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
	g_params.particleCollisionMargin = 0.01f;
	g_params.shapeCollisionMargin = 0.0f;
	g_params.collisionDistance = 0.0f;
	 
 
	g_params.sleepThreshold = 0.0f;
	g_params.shockPropagation = 0.0f;
	g_params.restitution = 0.001f;

	g_params.maxSpeed = FLT_MAX;
	g_params.maxAcceleration = 100.0f;	// approximately 10x gravity
	

	g_params.relaxationMode = eNvFlexRelaxationLocal;
	g_params.relaxationFactor = 1.0f;
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

	g_params.numPlanes =1;
	g_params.planes[0]  == Vec4(0.0f, 1.0f, 0.0f,-0.1f);

	//g_params.collisionDistance = 0.05f;
	//g_params.shapeCollisionMargin = 0.00001f;
}