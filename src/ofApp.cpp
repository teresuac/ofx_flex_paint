#include "ofApp.h"


void ofApp::setup(){
	ofSetVerticalSync(true);

	//INIT VARIABLES
	display_mode = true;  
	olddx = 0;
	olddy = 0;
	flex = new ofx_nvflex();
	import_soft = 0;
	height = 0;
	flex->init_flex();
	// GUI INIT --------------------------------------------
	//listener 
	reset.addListener(this, &ofApp::reset_sim);
	display_mode_switch.addListener(this, &ofApp::dswitch);

	gui.setup(); // most of the time you don't need a name
	gui.add(cohesion.setup("cohesion", .6, 0, 10));
	gui.add(adhesion.setup("adhesion", .2, 0, 10));
	gui.add(surfaceTension.setup("surfaceTension", 10, 0, 50));
	gui.add(vorticityConfinement.setup("vorticityConfinement", 5, 0, 50));
	gui.add(smoothing.setup("smoothing", 1, 0, 30));
	gui.add(viscosity.setup("viscosity", .5, 0, 10));
	gui.add(rate.setup("rate", 2, 0, 10));
	gui.add(size.setup("size", 2, 0, 10));
	gui.add(momentum.setup("momentum", .8, 0, 1));
	gui.add(gravity.setup("gravity", -15, 2, -35));
	gui.add(col.setup(ofColor(255, 255, 255, 255)));
	gui.add(stiffness.setup("spring stiffness", 1.0f, 0.0f, 20.0f));

	gui.add(reset.setup("reset"));
	gui.add(drawPP.setup("draw particles"));
	gui.add(shortcuta.setup(std::string("c : rand color")));
	gui.add(shortcutb.setup(std::string("r : reset cam")));
	bHide = false; 

	// ------------------------------------------------------
		
	// DISPLAY
	mesh.setMode(OF_PRIMITIVE_POINTS);
	glEnable(GL_POINT_SMOOTH); 
	glPointSize(2); 
	mesh_springs.setMode(OF_PRIMITIVE_LINES);
}

//--------------------------------------------------------------
void ofApp::update(){

	
}

//--------------------------------------------------------------
void ofApp::draw(){ 
	 
	if (ofGetElapsedTimef() > 1)
	{
		float fps = ofGetFrameRate();
		flex->dt = 1.0 / 25;  
		float dx = mouseX - ofGetPreviousMouseX();
		float dy = mouseY - ofGetPreviousMouseY();
		ofColor cc = col;
		mesh.clear();
		mesh_springs.clear();
		height  = 0.975f*height+13*0.025f; 
		if (keyIsDown['a'])
		{
			height -= 1.50f;
			height = ofClamp(height, 1.3f, 20.0f);
		}
		//cout << "map buffers start" << endl;
		flex->buffers->MapBuffers(); 
		//cout << "map buffers done" << endl;

		if (drawPP)
		{
			glm::vec3 const mouse_sample = cam.screenToWorld({ ofGetMouseX(), ofGetMouseY(), 0.0f });
			ofxRay::Ray const ray(cam.getPosition(), mouse_sample - cam.getPosition());
			ofxRay::Plane const plane({ 0, 0, 0 }, { 0, 1, 0 });

			glm::vec3 mouse;
			plane.intersect(ray, mouse);

			 flex->emit_particles(ofClamp(mouse.x, 1, 1020), ofClamp(mouse.z, 1, 760), dx*0.1f, dy*0.1f, olddx, olddx, rate, Vec3(cc.r, cc.g, cc.b));

			// emit softbody
			if (import_soft == 0)
			{
				std::cout << "create softbody" << endl;
				flex->create_softbody(Instance("brush.obj"));
				import_soft = 1;
				std::cout << "end create softbody " << endl;
			}

			olddx = dx;
			olddy = dy;
 
			srand(0.0f);

			for (int i = 0; i < flex->buffers->positions.size(); i++) {
				
				float x = flex->buffers->positions[i].x ;
				float y = flex->buffers->positions[i].z ;
				
				flex->buffers->velocities[i].x *= momentum ;
				flex->buffers->velocities[i].y *= momentum ;
				flex->buffers->velocities[i].z *= momentum ;
				
				// move brush
				if(i%35==34&&i<35*100)
				{	
					flex->buffers->positions[i].x = flex->buffers->restPositions[i].x + mouse.x;
					flex->buffers->positions[i].y =  flex->buffers->restPositions[i].y*0.0f + height + mouse.y ;
					flex->buffers->positions[i].z = flex->buffers->restPositions[i].z + mouse.z;
					flex->buffers->positions[i].w = 0.0f;
				} 
				
				//mesh
				ofVec3f pos(x, flex->buffers->positions[i].y , y);
				mesh.addVertex(pos);
				
				float c = ( (flex->buffers->ids[i])%1000)*0.001*0.5+0.5;
				Vec3 cc = flex->buffers->cols[i];
				mesh.addColor(ofColor(cc.x,cc.y,cc.z)*c);
			}
			/*
			for (int j = 0; j < flex->buffers->springStiffness.size(); j++)
			{
				flex->buffers->springStiffness[j] = stiffness;
			}*/

			for (int i = 0; i < flex->buffers->springIndices.size(); i++) {
				int indice = flex->buffers->springIndices[i];
				float x = flex->buffers->positions[indice].x;
				float y = flex->buffers->positions[indice].y;
				float z = flex->buffers->positions[indice].z;

				//mesh
				ofVec3f pos(x, y, z);
				mesh_springs.addVertex(pos);

			}

			/*cout << "pos " << flex->buffers->positions.size()  << endl ;
			cout << "active " << flex->buffers->activeIndices.size() << endl;
			cout << "springIndices " << flex->buffers->springIndices.size() << endl;
			cout << "active " << flex->buffers->activeIndices[0] << " " << flex->buffers->activeIndices[10] << endl;


			cout << "stop edit draw" << endl ;*/

		}
		
			flex->set_params(  cohesion,   adhesion,   surfaceTension,   vorticityConfinement,   smoothing,   viscosity,   size, gravity);
		
			//unmap solver get 
			flex->update(import_soft == 1);

			/*std::cout << endl << "set spring buffers" << flex->buffers->springIndices.size()*0.5 << " " << flex->buffers->springIndices.size() << endl;
			std::cout << endl << "set spring buffers" << flex->buffers->springStiffness.size() << endl;
			std::cout << endl << "set spring buffers" << flex->buffers->springLengths.size() << endl;*/

			 
	
		import_soft = 2;
		
	}

	//mode 3D
	if (ofGetElapsedTimef() < 4.0f)
	{
		reset_cam();
	}
	 
	cam.begin() ;
	mesh_springs.draw();
	mesh.draw() ;
	cam.end()   ;
	 
	if (!bHide) {
		gui.draw() ;
	}

	
 
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	keyIsDown[key] = true;
	if (key == 'c')
	{
		rand_color();
	}
	else if (key == 'r')
	{
		reset_cam();
	}
 
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	keyIsDown[key] = false;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	 // flex->emit_particles(x, y);

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::rand_color() {
	srand(time(NULL));
	col = ofColor((rand() % 500) *255.0 / 500.0, ((rand() + 5000) % 500) * 255.0 / 500.0, ((rand() + 2000) % 500)*255.0 / 500.0);

}

void ofApp::reset_cam() {
	//cam.setPosition(ofPoint(ofGetWidth() / 2, 750, ofGetHeight() / 2));
	//cam.setPosition(ofPoint(20, 100, 20));
	cam.setPosition(ofPoint(ofGetWidth() / 2+100, 350, ofGetHeight() / 2)+100);
	lookAt.setPosition(ofPoint(ofGetWidth() / 2, 0, ofGetHeight() / 2));
	//lookAt.setPosition(0, 0, 0);
	cam.lookAt(lookAt);
	 
}

void ofApp::dswitch() {
	display_mode = !display_mode;
}

void ofApp::reset_sim() {
	flex->init_flex();
	import_soft = 0;
}