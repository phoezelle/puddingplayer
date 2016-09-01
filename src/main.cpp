#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
  
	ofSetupOpenGL(500 ,500, OF_FULLSCREEN);			// <-------- setup the GL context
  
  ofGLESWindowSettings settings;
  settings.setGLESVersion(2);
  ofCreateWindow(settings);
  ofRunApp(new ofApp);
}
