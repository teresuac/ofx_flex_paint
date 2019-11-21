#include "ofApp.h"
#include "ofMath.h"



//--------------------------------------------------------------

void ofApp::resetf() {
	flex->init_flex();
}
void ofApp::dswitch() {
	display_mode = !display_mode;
}

void ofApp::setup(){
	ofSetVerticalSync(true);

	//INIT VARIABLES
	display_mode = false; 

	//Setup
	flex = new ofx_nvflex();
	flex->init_flex();
	
	olddx = 0;
	olddy = 0;

	// GUI INIT --------------------------------------------

	//listener 
	reset.addListener(this, &ofApp::resetf);
	display_mode_switch.addListener(this, &ofApp::dswitch);

	gui.setup(); // most of the time you don't need a name
	gui.add(cohesion.setup("cohesion", .9, 0, 10));
	gui.add(adhesion.setup("adhesion", .91, 0, 10));
	gui.add(surfaceTension.setup("surfaceTension", 5, 0, 50));
	gui.add(vorticityConfinement.setup("vorticityConfinement", 8, 0, 50));
	gui.add(smoothing.setup("smoothing", 31, 0, 30));
	gui.add(viscosity.setup("viscosity", .51, 0, 10));
	gui.add(rate.setup("rate", 1.5, 0, 10));
	gui.add(size.setup("size", 1.5, 0, 10));
	gui.add(momentum.setup("momentum", .94, 0, 1));
	gui.add(reset.setup("reset"));
	gui.add(display_mode_switch.setup("display_3D"));
	bHide = false; 
	// ------------------------------------------------------

	 mesh.setMode(OF_PRIMITIVE_POINTS);
	 
	//ofEnableDepthTest();
	glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	glPointSize(2); // make the points bigger
 
	// back
	meshb.setMode(OF_PRIMITIVE_LINE_STRIP);
	//meshb.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	meshb.addVertex(ofVec3f(0, 0, 0));
	meshb.addVertex(ofVec3f(0, 0, 768));
	meshb.addVertex(ofVec3f(1024, 0, 768));
	meshb.addVertex(ofVec3f(1024, 0, 0));
	meshb.addVertex(ofVec3f(0, 0, 0));

	//cam
	cam.setOrientation(ofVec3f(-90, 0, 0));
	cam.setPosition(512, 500, 368);
	Light.setup();
	Light.setPosition(-200, 200,100);

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

		flex->update(ofClamp(mouseX, 1, 1020), ofClamp(mouseY, 1, 760),dx,dy, olddx, olddx,rate);

		olddx = dx;
		olddy = dy;

		 // MODE draw rectangle
		ofSetColor(0, 0, 0);
		if (display_mode == true)
		{
			mesh.clear();
			//cam.begin();
			//meshb.draw();
		}
		srand(0.0f);

		for (int i = 0; i < flex->buffers->positions.size(); i++) {
			float x = flex->buffers->positions[i].x;
			float y = flex->buffers->positions[i].z;
			flex->buffers->positions[i].y *= 0.06;
		
			flex->buffers->velocities[i].x *= momentum;
			flex->buffers->velocities[i].y *= momentum;
			flex->buffers->velocities[i].z *= momentum;

			if (display_mode == false)
			{
				float c = ((flex->buffers->ids[i]) % 1000)*0.001;
				ofSetColor(c * 255, c * 255, c*255);
				ofRect(ofClamp(x, 1, 1024), ofClamp(y, 1, 750), 2, 2);
			}
			else
			{
				ofVec3f pos(x, flex->buffers->positions[i].y , y);
				mesh.addVertex(pos);
			
				float c = ( (flex->buffers->ids[i])%1000)*0.001;
				mesh.addColor(ofFloatColor(c,c,c));
				 
			}
		}

		flex->set_params(  cohesion,   adhesion,   surfaceTension,   vorticityConfinement,   smoothing,   viscosity,   size);
		flex->updateb();
	}


	//mode 3D
	if (display_mode == true)
	{
		 cam.begin() ;
		 meshb.draw();
		 mesh.draw() ;


		cam.end()   ;
	}

	if (!bHide) {
		gui.draw() ;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
