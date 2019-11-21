#include "ofApp.h"
#include "ofMath.h"



//--------------------------------------------------------------

void ofApp::rand_color() {
	srand(time(NULL));
	col=ofColor(  (rand() % 500) *255.0/500.0,  ((rand()+5000) % 500) * 255.0/500.0,  ((rand()+2000) % 500)*255.0/500.0);
 
}

void ofApp::reset_cam() {
	cam.setPosition(ofPoint(ofGetWidth() / 2, 700, ofGetHeight() / 2));
	lookAt.setPosition(ofPoint(ofGetWidth() / 2, 0, ofGetHeight() / 2));
	cam.lookAt(lookAt);
}
void ofApp::resetf() {
	flex->init_flex();
}
void ofApp::dswitch() {
	display_mode = !display_mode;
}

void ofApp::setup(){
	ofSetVerticalSync(true);

	//INIT VARIABLES
	display_mode = true; // 3D Mesh 

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
	gui.add(cohesion.setup("cohesion", .6, 0, 10));
	gui.add(adhesion.setup("adhesion", .9, 0, 10));
	gui.add(surfaceTension.setup("surfaceTension", 10, 0, 50));
	gui.add(vorticityConfinement.setup("vorticityConfinement", 5, 0, 50));
	gui.add(smoothing.setup("smoothing", 1, 0, 30));
	gui.add(viscosity.setup("viscosity", .5, 0, 10));
	gui.add(rate.setup("rate", 2, 0, 10));
	gui.add(size.setup("size", 2, 0, 10));
	gui.add(momentum.setup("momentum", .94, 0, 1));
	gui.add(col.setup(ofColor(255, 255, 255, 255)));
	gui.add(reset.setup("reset"));
	//gui.add(display_mode_switch.setup("display_3D"));
	gui.add(shortcuta.setup(std::string("c : rand color")));
	gui.add(shortcutb.setup(std::string("r : reset cam")));
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
	reset_cam();

	Light.setup();
	Light.setPosition(-200, 200,100);
	rand_color();
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
		flex->update(ofClamp(mouseX, 1, 1020), ofClamp(mouseY, 1, 760),dx,dy, olddx, olddx,rate,Vec3(cc.r,cc.g,cc.b));

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
			flex->buffers->positions[i].y *= 0.99;
		
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
			
				float c = ( (flex->buffers->ids[i])%1000)*0.001*0.5+0.5;
				Vec3 cc = flex->buffers->cols[i];
				mesh.addColor(ofColor(cc.x,cc.y,cc.z)*c);
				 
			}
		}

		flex->set_params(  cohesion,   adhesion,   surfaceTension,   vorticityConfinement,   smoothing,   viscosity,   size);
		flex->updateb();
	}

	//mode 3D
	if (ofGetElapsedTimef() < 4.0f)
	{
		reset_cam();
	}
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
void ofApp::keyPressed(int key) {
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
