#pragma once

#include "ofMain.h" 
#include "nvflex_ofx.h"
#include "ofxGui.h"
#include "of3DGraphics.h"

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
		void resetf();
		void dswitch();
		void reset_cam();
		void rand_color();

		// nflex
		ofx_nvflex* flex;

		// mouse old
		float olddx, olddy; 


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
		ofxColorSlider col;
		ofxLabel shortcuta; 
		ofxLabel shortcutb;
		ofxButton reset; 
		ofxButton display_mode_switch;
		bool bHide;
		ofxPanel gui;

		// 3D mode
		bool display_mode;
		ofMesh mesh;
		ofMesh meshb;
		ofEasyCam cam;
		ofLight Light;
		//of3dGraphics a;

		ofNode lookAt; 
};

