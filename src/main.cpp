#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
  
	ofSetupOpenGL(1200 ,800, OF_FULLSCREEN);			// <-------- setup the GL context
  
  ofGLESWindowSettings settings;
  settings.setGLESVersion(2);
  ofCreateWindow(settings);
  ofRunApp(new ofApp);
}
