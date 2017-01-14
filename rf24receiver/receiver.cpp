//
//  receiver.cpp
//  puddingplayer
//
//  Created by SuperPierre on 14/01/2017.
//
//

#include "receiver.h"


#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
const uint64_t receiveingPrefix = 0xF6FEE60000LL;
uint8_t data[32];

RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_4MHZ);


int main(int argc, char** argv){
  
  int fd;
  char * myfifo = "/tmp/myfifo";
  /* create the FIFO (named pipe) */
  mkfifo(myfifo, 0666);
  /* write "Hi" to the FIFO */
  fd = open(myfifo, O_WRONLY);
  
  std::cout << "start" << '\n';
  radio.begin();
  radio.setRetries(1,6);
  radio.setPayloadSize(8);
  radio.enableAckPayload();
  radio.setChannel(122);
  //radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  uint64_t address = receiveingPrefix | 8;
  radio.openReadingPipe(1,address);
  delay(1000);
  radio.printDetails();
  if (radio.isPVariant()) std::cout << "radio OK" << '\n';
  radio.startListening();
  
  while(1){
    delay(1);
    while(radio.available()){
      radio.read(&data,32);
      cout << "data : " ;
      for(int i = 0; i<32;i++){
        cout << (int)data[i] << '-';
      }
      cout << endl;
    }
  }
  
}