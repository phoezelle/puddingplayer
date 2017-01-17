//
//  checkremote.h
//  puddingplayer
//
//  Created by SuperPierre on 17/01/2017.
//
//

#include <cstdlib>
#include "ofMain.h"
#include <string>

#ifndef puddingplayer_checkremote_h
#define puddingplayer_checkremote_h

class Checkremote : public ofThread {
  
  //ofThreadChannel<int> myThreadChannel;
  
  public :
  
  
  // the thread function
  void threadedFunction() {
    
    // start
    unsigned long starttime = ofGetElapsedTimeMillis();
    while(isThreadRunning()) {
      
      string input="";
      //log("read cin",USR);
      getline(cin, input);
      if(input.length()>3){
        std::cout << "cin = " << input << std::endl;
        if(input.compare(0, 6, "OKDATA")==0  && ofGetElapsedTimeMillis()-starttime > 1500){
          starttime = ofGetElapsedTimeMillis();
          std::cout << "receive order : " << input.substr(7,1) << std::endl;
          //myThreadChannel.send(std::move(ofToInt(input.substr(7,1))));
          lock();
          order = ofToInt(input.substr(7,1));
          unlock();
        }
      }
    }
    
    // done
  }
          int order = 0;
  
};

#endif
