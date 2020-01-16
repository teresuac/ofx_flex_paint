#pragma once
#include "ofxGui.h"
#include "ofMain.h" 
#include "ofxNvFlex.h"


namespace mUtil
{
	void rand_color(ofxColorSlider &col) {
		srand(time(NULL));
		col = ofColor((rand() % 500) *255.0 / 500.0, ((rand() + 5000) % 500) * 255.0 / 500.0, ((rand() + 2000) % 500)*255.0 / 500.0);

	}

	void reset_cam(ofEasyCam &cam, ofNode lookAt) {
		cam.setPosition(ofPoint(ofGetWidth() / 2, 700, ofGetHeight() / 2));
		lookAt.setPosition(ofPoint(ofGetWidth() / 2, 0, ofGetHeight() / 2));
		cam.lookAt(lookAt);
	}

	void resetf(ofx_nvflex &a) {
		a->init_flex();
	}

	void dswitch() {
		display_mode = !display_mode;
	}
}
//-------------------------------------------------------------------------------------------------------------
