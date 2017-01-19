#include <stddef.h>
#include <avr/pgmspace.h>

#include "nRF24L01.h"
#include "RF24.h"
#include <SPI.h>

#define LED 4
#define VIB 7
#define PUSH1 2
#define PUSH2 3
#define PUSH3 6
#define OUT 5
#define RF24_CE 9
#define RF24_CSN 10
#define MOSI 11
#define MISO 12
#define CLK 13

byte codepin[4]={A2,A3,A4,A5};
byte codevalue[4]={8,4,2,1};


//BATTERY ANALOG CORECTION
#define MASTER_ABATT_5 598
#define MASTER_ABATT_4 589
#define MASTER_ABATT_3 580
#define MASTER_ABATT_2 577
#define MASTER_ABATT_1 564
#define MASTER_ABATT_OFF 540

#define CODE_ACK_WAITING 3
#define CODE_ACK_OK 9
#define CODE_ACK_VALIDATION 5

const uint64_t sendingPrefix = 0xF6FEE60000LL;
RF24 radio(RF24_CE,RF24_CSN);
int channel = 0;

volatile byte p1_state = 0;
volatile byte p2_state = 0;
volatile byte p3_state = 0;

long p1_time_push;
long p2_time_push;
long p3_time_push;

byte p1_long = 0;
byte p2_long = 0;
byte p3_long = 0;
byte p2_superlong = 0;


byte vib_value = LOW;
byte vib_state = 0;
long vib_time_on = millis();
int vib_time_period = 50;

byte led_state = 0;
byte led_value = LOW;
long led_time_on = millis();
int led_time_period = 200;

byte needcheckbatterie = 0;

long speeddown =1;
byte startup = 1;


void setup() {
  //init serial
  Serial.begin(115200);
  Serial.print("start pudding remote");
  
  //pin mode
  pinMode(LED, OUTPUT);
  pinMode(VIB, OUTPUT);
  pinMode(RF24_CE, OUTPUT);
  pinMode(RF24_CSN, OUTPUT);
  pinMode(PUSH1, INPUT);
  pinMode(PUSH2, INPUT);
  pinMode(PUSH3, INPUT);
  for(byte i=0;i<4;i++){
    pinMode(codepin[i],OUTPUT);
    digitalWrite(codepin[i],LOW);
    pinMode(codepin[i],INPUT);
  }
  

  //init radio
  radio.begin();
  radio.setRetries(1,6); // 6 try delay 500us each
  radio.setPayloadSize(8);
  radio.enableAckPayload();
  radio.setChannel(122);
  //radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
  if (radio.isPVariant()) Serial.print("radio OK ");
  
  delay(200);
  //get remote channel
  for(byte i=0;i<4;i++){
    byte count = 0;
    for(byte j=0;j<10;j++){
      if(digitalRead(codepin[i])==HIGH) count++;
      delay(1);
    }
    Serial.print(codepin[i],DEC);
    Serial.print(":");
    Serial.print(count,DEC);
    Serial.print(" ");
    if(count==10) channel+=codevalue[i];
  }
  
  Serial.print("ch");
  Serial.print(channel,DEC);
  updateSendingAddress(channel);
  flash_led(channel);

  //interrupt for two first push
  attachInterrupt(0, p1, CHANGE);
  attachInterrupt(1, p2, CHANGE);

  getBatteryStatus();
  flash_vib(1);
  
}

void slowdown(){
  CLKPR = 0x80;    // Tell the AtMega we want to change the system clock
  CLKPR = 0x05;    // 1/32 prescaler = 250KHz for a 16MHz crystal
  speeddown = 32;
}

void speedup(){
      CLKPR = 0x80;    // Tell the AtMega we want to change the system clock
      CLKPR = 0x0;
      speeddown =1;
}

long myTime (long t){
  return (long)(1.0f*t)/speeddown;
}

void loop() {
  // put your main code here, to run repeatedly:
  if(p1_state==1){
    Serial.print("p1p");
    p1_time_push=millis();
    p1_state++;
  }
  if(!p1_long && p1_state==2 && millis()-p1_time_push>myTime(2000)){
    Serial.print("p1l");
    p1_long=1;
    flash_led(1);
  }
  if(p2_state==1){
    Serial.print("p2p");
    p2_time_push=millis();
    p2_state++;
  }
  if(!p2_superlong && !p2_long && p2_state==2 && millis()-p2_time_push>myTime(2000)){
    Serial.print("p2l");
    p2_long=1;
    flash_led(1);
  }
  if(!p2_superlong && p2_state==2 && millis()-p2_time_push>myTime(10000)){
    Serial.print("p2sl");
    p2_superlong=1;
    p2_long=0;
    flash_led(3);
  }
  if(p1_state==3){
    Serial.print("p1r");
    if(p1_long==0){sendMessage(1); }else{ sendMessage(2); }
    p1_state=0;p1_long=0;
  }
  if(p2_state==3){
    Serial.print("p2r");
    if(p2_superlong==1){ sendMessage(8); }else{
      if(p2_long==0){ sendMessage(3); }else{ sendMessage(4); }
    }
    p2_state=0;p2_long=0;p2_superlong=0;
  }

  
  if((p3_state==0 && !digitalRead(PUSH3)) || (p3_state==2 && digitalRead(PUSH3))){
    if(p3_state==0)p3_state=1;
    if(p3_state==2)p3_state=3;
  }
  if(p3_state==1){
    Serial.print("p3p");
    p3_time_push=millis();
    p3_state++;
  }
  if(!p3_long && p3_state==2 && millis()-p3_time_push>myTime(2000)){
    Serial.print("p3l");
    p3_long=1;
    flash_led(1);
  }
  if(p3_state==3){
    Serial.print("p3r");
    if(p3_long==0){ sendMessage(5); }else{ sendMessage(6); }
    p3_state=0;p3_long=0;
  }
  
  if(led_state > 0 && millis() - led_time_on > myTime(led_time_period)){
    Serial.print("led");Serial.print(led_state,DEC);Serial.print("-");Serial.print(led_value,DEC);
    led_time_on = millis();
    led_state--;
    led_value = !led_value;
    digitalWrite(LED, led_value);
  }
  if(led_state==0){
    led_value=0;
    digitalWrite(LED, led_value);
  }
  
  if(vib_state > 0 && millis() - vib_time_on > myTime(vib_time_period)){
    Serial.print("vib");Serial.print(vib_state,DEC);
    vib_time_on = millis();
    vib_state--;
    vib_value = !vib_value;
    digitalWrite(VIB, vib_value);
  }
  if(vib_state==0){
    vib_value=0;
    digitalWrite(VIB, vib_value);
  }

 if(needcheckbatterie==1){
  if(millis()%myTime(1000)>myTime(700)){
  needcheckbatterie=0;
  getBatteryStatus();
  }
 }
  if(needcheckbatterie==0){
  if(millis()%myTime(1000)<myTime(200))needcheckbatterie=1;
 }

 if(startup==1 ){
  if(millis()>myTime(10000)){startup=0;
  slowdown();
  }
 }
  
}



void p1()
{
  //Serial.print("p1");
  if(p1_state==0)p1_state=1;
  if(p1_state==2)p1_state=3;
}
void p2()
{
  //Serial.print("p2");
  if(p2_state==0)p2_state=1;
  if(p2_state==2)p2_state=3;
}

void updateSendingAddress(byte suffix){
    uint64_t address = sendingPrefix | suffix;
    radio.openWritingPipe(address);
}

void sendMessage(int tmessage){
  /*if(sendMessageProc(tmessage)){
    sendMessageProc(9);
  }*/
  speedup();
  delay(1);
  radio.powerUp();
  delay(10);
  if(sendMessageSimple(tmessage)){
    sendMessageSimple(9);
  }
  radio.powerDown();
}

bool sendMessageProc(int tmessage){
  uint64_t message=0x0000000000LL;
  message |= tmessage;
  byte noConnect=0;
  while(1){
  Serial.print ("send ");Serial.print (tmessage);
  boolean connect = radio.write(&message, sizeof(message));
  byte result = 0;
  if(connect){
     if (radio.isAckPayloadAvailable()) {
        uint64_t received=0;
        radio.read(&received, sizeof(received));
        result=lowByte(received)&0x0F;
        Serial.print ("ack");Serial.print (result,DEC);
        }
      }
   switch (result) {
    case CODE_ACK_WAITING :
      Serial.print ("SEND-OK");
    break;
    case CODE_ACK_OK :
       Serial.print ("SEND-ENDING");
       if(tmessage>0 && tmessage<9)flash_vib(2);  
       //flush ack
       return true;
    break;
    default:
    noConnect++;
    if(noConnect>20){
       Serial.print ("SEND-ABORT");
       return false;
    }
   }
  }
}

bool sendMessageSimple(int tmessage){
  uint64_t message=0x0000000000LL;
  message |= tmessage;
  byte noConnect=0;
  while(1){
  Serial.print ("send ");Serial.print (tmessage);
  boolean connect = radio.write(&message, sizeof(message));
  if(connect){
     byte result = 0;
     if (radio.isAckPayloadAvailable()) {
        uint64_t received=0;
        radio.read(&received, sizeof(received));
        result=lowByte(received)&0x0F;
        Serial.print ("ack");Serial.print (result,DEC);
        if (result==tmessage){
          flash_vib(2);
          return true;
          
          }
        }
  }
    noConnect++;
    if(noConnect>20){
       Serial.print ("SEND-ABORT");
       return false;
    }
   }
  
}

void flash_led(byte f){
  Serial.print ("startflashled");Serial.print(f,DEC);
  led_state=2*f;
}

void flash_vib(byte f){
  Serial.print ("startflashvib");Serial.print(f,DEC);
  vib_state=2*f;
}


// *************************************************************************************
//          card utility
// *************************************************************************************

int getAnalogBatteryValue(){
  int bat = analogRead(A0);
  Serial.print("b");
  Serial.print(bat,DEC);
  return bat;
}

byte getBatteryStatus() {
  int bat = getAnalogBatteryValue();
  if (bat > MASTER_ABATT_5)
    return 5;
  if (bat > MASTER_ABATT_4)
    return 4;
  if (bat > MASTER_ABATT_3)
    return 3;
  if (bat > MASTER_ABATT_2)
    return 2;
  if (bat > MASTER_ABATT_1)
    flash_led(2);
    return 1;
  if (bat < MASTER_ABATT_OFF)
    flash_led(2);
    return 255;
  return 0;
}
