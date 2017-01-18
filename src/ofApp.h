#pragma once

#include "ofMain.h"
#include "checkremote.h"


class ofApp : public ofBaseApp{
  
public:
  
  Checkremote thread;
  
  void setup();
  void update();
  void draw();
  void exit();
  
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
  
  void checkREMOTE();
  
  int ledG_value;
  int ledG_state;
  uint64_t ledG_time_on;
  uint64_t ledG_time_period;
  
  int ledR_state;
  int ledR_value;
  uint64_t ledR_time_on;
  uint64_t ledR_time_period;
  //ofThreadChannel<int> myThreadChannel;
  void checkLed();
  void flash_ledG(int f);
  void flash_ledR(int f);
  

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
  

  
  //VIDEOPLAYER
  ofVideoPlayer 		*fingerMovie;
  ofVideoPlayer     *fingerMovie2;
  int nvideo;
  int gochangevideo;
  bool isplayground;
  void play();
  void playNext();
  void pausePlay();
  void playBackground();
  void changeBackground();
  void nextScene();
  void restartScene();
  void clearVideo(int n);
  
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
