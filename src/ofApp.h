#pragma once

#include "ofMain.h"

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
  
  //SETUP AND PREF
  void definePlateform();
  void usbstickMount();
  void buildMediaList();
  void getPref();
  void checkNetwork();
  void setcurrentCharacter(int _character);
  
  bool pi,firstlaunch;
  int posX,posY,width,height,loglevel;
  string firstMessage;
  vector <string> characterList;
  vector <vector <string>> mediaList;
  vector <string> backgroundList;
  
  //GPIO
  void initGPIO();
  void checkGPIO();
  void getAnalogBattery();
  

#define PIN_NUM 3
#define PUSH1 1
#define PUSH2 4
#define PUSH3 5

#define LEDR 0
#define LEDG 2
  
  int laststate[PIN_NUM];
  int pin[PIN_NUM];
  uint64_t startpushtime[PIN_NUM];
  bool longpush[PIN_NUM];
  
  
  //RF24
  void initRF24();
  void setAdressRF24();
  bool getMessageRF24();
  void sendConfirm();
  
  uint64_t ackT, lastReceivedMessage;
  int counterGoodMessage;
  
  //VIDEOPLAYER
  ofVideoPlayer 		fingerMovie;
  void play();
  void playNext();
  void pausePlay();
  void playBackground();
  void changeBackground();
  void nextScene();
  void restartScene();
  
  int maxScene, currentScene, currentVideo, currentBackground, currentCharacter,nbMedia;
  string rootDirectory,mediaDirectory,backgroundDirectory;
  
  //UTILITY
  void log(string log,int level,bool color=false);
  
#define WARNING 0
#define USR 1
#define DEBUG 2
#define RAW 3
  

  
  //void checkinputandplayVideo();
  //void playVideoInPath();
  
  
  
  
  
  
};
