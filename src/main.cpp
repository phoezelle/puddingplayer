#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
  
	ofSetupOpenGL(1920 ,1280, OF_FULLSCREEN);			// <-------- setup the GL context
  
  ofGLESWindowSettings settings;
  settings.setGLESVersion(2);
  ofCreateWindow(settings);
  ofRunApp(new ofApp);
}
