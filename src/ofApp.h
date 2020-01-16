#pragma once

#include "ofMain.h" 
#include "ofxNvFlex.h"
#include "ofxGui.h"
#include "ofxRay.h" 

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		//callback
		void dswitch();
		void reset_cam();
		void rand_color();
		void reset_sim();

		// nflex
		ofx_nvflex* flex;

		// mouse old
		float olddx, olddy; 
		float height;

		// GUI
		ofxFloatSlider cohesion;
		ofxFloatSlider adhesion;
		ofxFloatSlider surfaceTension;
		ofxFloatSlider vorticityConfinement;
		ofxFloatSlider smoothing;
		ofxFloatSlider viscosity; 
		ofxFloatSlider rate;
		ofxFloatSlider size;
		ofxFloatSlider momentum ;
		ofxFloatSlider gravity;
		ofxFloatSlider stiffness;
		ofxColorSlider col;
		ofxToggle drawPP ;
		ofxLabel shortcuta; 
		ofxLabel shortcutb;
		ofxButton reset; 
		ofxButton display_mode_switch;
		bool bHide;
		ofxPanel gui;

		// 3D mode
		bool display_mode;
		ofMesh mesh;
		ofMesh mesh_springs;
		ofEasyCam cam;
		ofLight Light;
		ofNode lookAt; 

		int import_soft; 
		bool keyIsDown[255];

};

