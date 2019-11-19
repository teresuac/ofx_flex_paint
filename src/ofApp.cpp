#include "ofApp.h"
#include "ofMath.h"


//--------------------------------------------------------------


void ofApp::setup(){

	flex = new ofx_nvflex();
	flex->init_flex();
	
	olddx = 0;
	olddy = 0;
}

//--------------------------------------------------------------
void ofApp::update(){


 
}
 

//--------------------------------------------------------------
void ofApp::draw(){ 
	if (ofGetElapsedTimef() > 1)
	{
		float fps = ofGetFrameRate();
		flex->dt = 1.0 / 25; // fps;
		float dx = mouseX - ofGetPreviousMouseX();
		float dy = mouseY - ofGetPreviousMouseY();

		flex->update(ofClamp(mouseX, 1, 1020), ofClamp(mouseY, 1, 760),dx,dy, olddx, olddx);

		olddx = dx;
		olddy = dy;

		// std::cout << mouseX << " " << mouseY << endl;

		ofSetColor(0, 0, 0);
		for (int i = 0; i < flex->buffers->positions.size(); i++) {
			float x = flex->buffers->positions[i].x;
			float y = flex->buffers->positions[i].z;
			flex->buffers->positions[i].y *= 0.06;
		
			flex->buffers->velocities[i].x *= 0.985;
			flex->buffers->velocities[i].y *= 0.985;
			flex->buffers->velocities[i].z *= 0.985;
			ofRect(ofClamp(x, 1, 1024), ofClamp(y, 1, 750), 2, 2);
		}

		flex->updateb();
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
