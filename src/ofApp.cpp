#include "ofApp.h"
#include "wiringPi.h"


void ofApp::log(string log,int level,bool color){
  if(level<=loglevel){
    if(color) cout << "\033[32m";
    if(level==WARNING) cout << "\x1b[31m";
    cout << log;
    if(color || level==WARNING) cout << "\x1b[0m";
    cout << endl;
    if(level<=USR)firstMessage+=log+"\n";
  }
}

void ofApp::definePlateform(){
  ofTargetPlatform myRun = ofGetTargetPlatform();
  if (myRun==OF_TARGET_LINUXARMV7L || myRun==OF_TARGET_LINUXARMV6L){
    pi=true;
    fingerMovie.setPixelFormat(OF_PIXELS_NATIVE);
    rootDirectory="/tmp/stick";
    log("RASPI DETECT path from usb stick", USR);
    
  } else {
    pi=false;
    rootDirectory="";
    log("MAC DETECT", USR);
    
  }
}

void ofApp::usbstickMount(){
  ofDirectory myDir;
  if(myDir.doesDirectoryExist(rootDirectory)){
  } else {
    ofSystem("sudo mkdir "+rootDirectory);
    log("create mountpoint "+rootDirectory, USR);
  }
  if (myDir.listDir(rootDirectory)>1) {
    log("USB stick already mount", USR);
  } else {
    ofSystem("sudo mount -o rw /dev/sda1 "+rootDirectory);
    if(myDir.listDir(rootDirectory)>1) {
      log("mount usb stick", USR);
    }else {
      log("problem for mounting usb stick", WARNING);
    }
  }
  rootDirectory+="/";
}



//--------------------------------------------------------------
void ofApp::setup(){
  
  firstlaunch=true;
  loglevel=USR;
  pi=true;
  currentVideo=-1;
  currentScene=0;
  currentBackground=0;
  
  log("STARTUP",USR,true);
  
  ofHideCursor();
  ofBackground(0,0,0);
	ofSetVerticalSync(true);
  
  definePlateform();
  if (pi) {
    usbstickMount();
    initGPIO();
  }
  ofBackground(0,0,0);
  getPref();
  setcurrentCharacter(0);
  checkNetwork();


  
  
  playBackground();
  
	// Uncomment this to show movies with alpha channels
	// fingerMovie.setPixelFormat(OF_PIXELS_RGBA);

}


void ofApp::getPref(){
  
  posX=0;
  posY=0;
  width=ofGetScreenWidth();
  height=ofGetScreenHeight();
  log("screen size "+ofToString(ofGetWindowWidth())+" x "+ofToString(ofGetWindowHeight())+" / "+ofToString(ofGetScreenWidth())+" x "+ofToString(ofGetScreenHeight()),USR);
  loglevel=DEBUG;
  
  ofBuffer buffer = ofBufferFromFile(rootDirectory+"config.txt");
	if(buffer.size()) {
    for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
      string line = *it;
      if(line.empty() == false) {
        string attribute = ofTrim(ofSplitString(line, "=")[0]);
        string value = ofTrim(ofSplitString(line, "=")[1]);
        
        if (attribute.compare("positionX")==0) posX=ofToInt(value);
        if (attribute.compare("positionY")==0) posY=ofToInt(value);
        if (attribute.compare("largeur")==0) width=ofToInt(value);
        if (attribute.compare("hauteur")==0) height=ofToInt(value);
        if (attribute.compare("loglevel")==0) loglevel=ofToInt(value);
        if (attribute.compare("dossier_personnage")==0) characterList =  ofSplitString(value, ",");
        if (attribute.compare("dossier_background")==0) backgroundDirectory = value;
        log("read param "+attribute+" = "+value, DEBUG);
      }else {
        break;
      }
    }
	}else{
    log(rootDirectory+"config.txt file not found",WARNING);
  }
  
  ofSetWindowShape(width, height);
}


void ofApp::setcurrentCharacter(int _character){
    currentCharacter = _character;
    log("set character n" + ofToString(currentCharacter) + " -> directory " + characterList[currentCharacter], USR);
    buildMediaList();
}


void ofApp::buildMediaList(){
  log("Build media list",USR);
  mediaList.clear();
  ofDirectory myDir;
  if(!myDir.doesDirectoryExist(rootDirectory+"movies")){
    ofSystem("sudo mkdir "+rootDirectory+"movies");
    log("create movies directory", USR,true);
  }
  mediaDirectory = "movies/"+characterList[currentCharacter];
  if(!myDir.doesDirectoryExist(rootDirectory+mediaDirectory)){
    ofSystem("sudo mkdir "+rootDirectory+mediaDirectory);
    log("create " + mediaDirectory + " directory", USR,true);
  }
  ofDirectory mediaDir(rootDirectory+mediaDirectory);
  mediaDir.allowExt("mp4");
  mediaDir.allowExt("mov");
  mediaDir.allowExt("avi");
  nbMedia = mediaDir.listDir();
  mediaDir.sort();
  int lastScene=-1;
  vector <string> scene;
  if (nbMedia>0) {
    log("find " + ofToString(nbMedia) + " movie files in " + mediaDirectory,DEBUG);
    for (int i=0; i<nbMedia; i++) {
      string media = mediaDir.getName(i);
      
      int numScene = ofToInt(ofTrim(ofSplitString(media, "_")[0]));
      if(lastScene == -1) lastScene = numScene;
      int numVideo = ofToInt(ofTrim(ofSplitString(media, "_")[1]));
      if(numScene != lastScene) {
        log("new scene",DEBUG);
        lastScene = numScene;
        mediaList.push_back(scene);
        scene.clear();
      }
      log(media,DEBUG);
      scene.push_back(media);
    }
    mediaList.push_back(scene);
  }else{
    log("no media found in " + mediaDirectory,WARNING);
  }
  
  
  ofDirectory backgroundDir(rootDirectory+mediaDirectory+"/"+backgroundDirectory);
  backgroundDir.allowExt("mp4");
  backgroundDir.allowExt("mov");
  backgroundDir.allowExt("avi");
  int nbBackground = backgroundDir.listDir();
  if (nbBackground>0) {
    log("find " + ofToString(nbBackground) + " background movie files in " + mediaDirectory+"/"+backgroundDirectory,DEBUG);
    for (int i=0; i<nbBackground; i++) {
      string media = backgroundDir.getName(i);
      backgroundList.push_back(media);
    }
  }else{
    log("no background media found in " + mediaDirectory+"/"+backgroundDirectory,WARNING);
  }
}

void ofApp::checkNetwork(){
  cout << "checking network";
  if(pi) {
    string ip_eth = ofTrim(ofSystem("ifconfig eth0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'"));
    string ip_wlan = ofTrim(ofSystem("ifconfig wlan0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'"));
    string hostname = ofTrim(ofSystem("cat /etc/hostname"));
    log("network on pi name "+hostname+" : eth "+ip_eth+" ; wlan "+ip_wlan, USR);
  }else {
    string ip_en1 = ofSystem("ifconfig en1 | grep 'inet ' | cut -dt -f2 | awk '{ print $1}'");
    log("network on mac : en1 "+ip_en1, USR);
  }
}


void ofApp::initGPIO(){
  
  pin[0] = PUSH1;
  pin[1] = PUSH2;
  pin[2] = PUSH3;
  
  if(wiringPiSetup() == -1){
    log("Error on wiringPi setup\n",WARNING);
  }else {
    for (int i=0; i<PIN_NUM; i++) {
      pinMode(pin[i],INPUT);
      pullUpDnControl(pin[i],PUD_DOWN);
      laststate[i]=digitalRead(pin[i]);
    }
  }
}

void ofApp::getAnalogBattery(){
  
}

void ofApp::initRF24(){
  
}

void ofApp::setAdressRF24(){
  
}

bool ofApp::getMessageRF24(){
  return false;
  
}

void ofApp::playNext(){
  if (currentVideo==-1){
    log("first time play",DEBUG);
    currentVideo=0;
    play();
    return;
  }
  if (mediaList[currentScene][currentVideo] == mediaList.back().back()) {
    log("return to first scene",DEBUG);
    currentScene=0;currentVideo=0;
  }else{
  if(mediaList[currentScene][currentVideo] == mediaList[currentScene].back()) {
    log("go next scene",DEBUG);
      currentScene++;currentVideo=0;
  }else{
    log("go next video",DEBUG);
    currentVideo++;
  }
  }
  play();
}

void ofApp::play(){
  if(fingerMovie.isPlaying())fingerMovie.stop();
string videoPath = rootDirectory+mediaDirectory+"/"+ mediaList[currentScene][currentVideo];
log("play video ("+ofToString(currentScene)+"/"+ofToString(currentVideo)+") "+videoPath, USR,true);
fingerMovie.load(videoPath);
fingerMovie.setLoopState(OF_LOOP_NONE);
fingerMovie.play();
}

void ofApp::pausePlay(){
  log("play/pause video ", USR);
  fingerMovie.setPaused(!fingerMovie.isPaused());
}

void ofApp::playBackground(){
  if(fingerMovie.isPlaying())fingerMovie.stop();
  string videoPath = rootDirectory+mediaDirectory+"/"+backgroundDirectory+"/"+ backgroundList[currentBackground];
  log("play backgroung video "+videoPath, USR);
  fingerMovie.load(videoPath);
  fingerMovie.setLoopState(OF_LOOP_NONE);
  fingerMovie.play();
}

void ofApp::changeBackground(){
  log("change background ", USR);
  if(backgroundList[currentBackground]==backgroundList.back()){
    currentBackground=0;
  } else {
    currentBackground++;
  }
}

void ofApp::nextScene(){
  if(mediaList[currentScene][currentVideo] != mediaList.back()[currentVideo]){
    log("go next scene ", USR);
    currentVideo=0;
    currentScene++;
    play();
  }else{
    log("no next scene ", USR);
  }
}

void ofApp::restartScene(){
  log("restart scene ", USR);
    currentVideo=0;
    play();
  
}


void ofApp::checkGPIO(){
  for (int i=0; i<PIN_NUM; i++) {
    int gpio_state = digitalRead(pin[i]);
    if(gpio_state!=laststate[i]){
      laststate[i]=gpio_state;
      if (gpio_state==0) {
      if(!longpush[i]){
        log("short push release wiring pi "+ofToString(pin[i]), DEBUG);

      if(i==0){
        playNext();
      }
      if(i==1){
        pausePlay();
      }
      if(i==2){
        nextScene();
      }
        }else{
          log("long push release wiring pi "+ofToString(pin[i]), DEBUG);

          if(i==1){
            changeBackground();
          }
          if(i==2){
            restartScene();
          }
        }
      }else{
        log("push up wiring pi "+ofToString(pin[i]), DEBUG);
        startpushtime[i]=ofGetElapsedTimeMillis();
      }
      longpush[i]=false;
      
    }
    if(gpio_state!=0 && !longpush[i] && ofGetElapsedTimeMillis() - startpushtime[i] > 3000){
      longpush[i]=true;
      log("longmode wiring pi "+ofToString(pin[i]), DEBUG);
    }
    
  }
}




  
//--------------------------------------------------------------
void ofApp::update(){
  fingerMovie.update();
  if(pi)checkGPIO();
}

//--------------------------------------------------------------
void ofApp::draw(){
  
  //checkinputandplayVideo();
  
	ofSetHexColor(0xFFFFFF);
  
  fingerMovie.draw(posX,posY,width+128,height+72);
  ofSetHexColor(0xff3300);
  
  if(ofGetElapsedTimeMillis()<60000){
    ofDrawBitmapString(firstMessage,100,420);
  }
  
  if(fingerMovie.getIsMovieDone()){
    log("end video",DEBUG);
    playBackground();
  }
}







//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
  switch(key){
    case 'a':
      playNext();
      break;
    case 'b':
      playBackground();
      break;
    case 'c':
      changeBackground();
      break;
    case 'p':
      pausePlay();
      break;
    case 's':
      nextScene();
      break;
    case 'r':
      restartScene();
      break;
    
      
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


/*
void ofApp::checkinputandplayVideo(){
  if(firstlaunch){
  lastpath="inconnu";
  if (!fingerMovie.isPlaying()){
    playVideoInPath();
  }
    firstlaunch=false;
  }
}

void ofApp::playVideoInPath() {
  ofDirectory myDir;
  if(myDir.doesDirectoryExist(rootDirectory+"movies/"+ lastpath)){
    int maxVideo = myDir.listDir(rootDirectory+"movies/" + lastpath);
    cout << ofToString(maxVideo) << " files in path : " << rootDirectory << "movies/" << lastpath <<  endl;
    int selected = (int) ofRandom(maxVideo);
    cout << "\x1b[32mtry read video "<< selected << " : " << myDir.getPath(selected) << "\x1b[0m" << endl;
    
    fingerMovie.load(myDir.getPath(selected));
    fingerMovie.setLoopState(OF_LOOP_NONE);
    // cout << "video size" << fingerMovie.getHeight() << "/" << fingerMovie.getWidth() << endl;
    fingerMovie.play();
    
  }
}
 */


