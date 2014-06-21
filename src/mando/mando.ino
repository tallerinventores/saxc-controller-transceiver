#include <Bounce2.h>

#define UpPin 8
#define LeftPin 9
#define RightPin 10
#define DownPin 11
// X A0
// Y A1
#define MousePin A2
#define StartPin A3
#define SelectPin A4

//rc commands
#define RC_ACK 0x10
#define RC_SYNC 0x11
#define RC_GET 0x12
#define RC_LIGHT_ON 0x13
#define RC_LIGHT_OFF 0x14
#define RC_CLAXON_ON 0x15
#define RC_CLAXON_OFF 0x16
#define RC_FORW 0x17
#define RC_REV 0x18
#define RC_SPEED_0 0x20
#define RC_SPEED_1 0x21
#define RC_SPEED_2 0x22
#define RC_SPEED_3 0x23
#define RC_SPEED_4 0x24
#define RC_SPEED_5 0x25
#define RC_SPEED_6 0x26
#define RC_SPEED_7 0x27
#define RC_SPEED_8 0x28
#define RC_SPEED_9 0x29
#define RC_SET_STEERING_L7 0x31
#define RC_SET_STEERING_L6 0x32 
#define RC_SET_STEERING_L5 0x33
#define RC_SET_STEERING_L4 0x34
#define RC_SET_STEERING_L3 0x35
#define RC_SET_STEERING_L2 0x36
#define RC_SET_STEERING_L1 0x37
#define RC_SET_STEERING_LR 0x38
#define RC_SET_STEERING_R1 0x39
#define RC_SET_STEERING_R2 0x3A
#define RC_SET_STEERING_R3 0x3B
#define RC_SET_STEERING_R4 0x3C
#define RC_SET_STEERING_R5 0x3D
#define RC_SET_STEERING_R6 0x3E
#define RC_SET_STEERING_R7 0x3F

Bounce UpObj = Bounce();
Bounce LeftObj = Bounce();
Bounce RightObj = Bounce();
Bounce DownObj = Bounce();
Bounce MouseObj = Bounce();
Bounce StartObj = Bounce();
Bounce SelectObj = Bounce();

#define GM_REFRESH 100

byte function = 0;

/*
  board A: GND - UP - LEFT - RIGHT - DOWN - VCC
  board B: GND - X - Y -THUMB - A - B - VCC
  xbee: 0013A200 XXXXXXXX
*/

void setup() {
  boolean Sync = false;
  
  pinMode(UpPin, INPUT);
  pinMode(LeftPin, INPUT);
  pinMode(RightPin, INPUT);
  pinMode(DownPin, INPUT);
  pinMode(MousePin, INPUT);
  pinMode(StartPin, INPUT);
  pinMode(SelectPin, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  UpObj.attach(UpPin);
  UpObj.interval(5);
  LeftObj.attach(LeftPin);
  LeftObj.interval(5);
  RightObj.attach(RightPin);
  RightObj.interval(5);
  DownObj.attach(DownPin);
  DownObj.interval(5);
  MouseObj.attach(MousePin);
  MouseObj.interval(5);
  StartObj.attach(StartPin);
  StartObj.interval(5);
  SelectObj.attach(SelectPin);
  SelectObj.interval(5);
  Serial.begin(9600);
  
  if (digitalRead(StartPin) && digitalRead(SelectPin)) {
    function = 1;
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
  }
  else {
    function = 2;
    while (!Sync) {
      while (!Serial.available()) {
        Serial.print("??");
        delay(500);
      }
      if (Serial.read() == '!') Sync = true;
    }
    digitalWrite(13, HIGH);
  }
}

void loop() {
  static boolean RC_pushed = false;
  static boolean RC_fedge = false;
  static boolean RC_lights = false;
  char GM_Values[20];
  byte X = 0;
  byte Y = 0;
  static unsigned long GM_Update = 0;
  static unsigned long RC_sync = 0;
  
  UpObj.update();
  LeftObj.update();
  RightObj.update();
  DownObj.update();
  MouseObj.update();
  StartObj.update();
  SelectObj.update();

  if (function == 2) {
    //polling values
    GM_Values[0] = '[';
    if (UpObj.read()) GM_Values[1] = 'U';
    else GM_Values[1] = 'u';
    GM_Values[2] = ',';
    if (LeftObj.read()) GM_Values[3] = 'L';
    else GM_Values[3] = 'l';
    GM_Values[4] = ',';
    if (RightObj.read()) GM_Values[5] = 'R';
    else GM_Values[5] = 'r';
    GM_Values[6] = ',';
    if (DownObj.read()) GM_Values[7] = 'D';
    else GM_Values[7] = 'd';
    GM_Values[8] = ',';
    if (StartObj.read()) GM_Values[9] = 'A';
    else GM_Values[9] = 'a';
    GM_Values[10] = ',';
    if (SelectObj.read()) GM_Values[11] = 'B';
    else GM_Values[11] = 'b';
    GM_Values[12] = ',';
    if (MouseObj.read()) GM_Values[13] = 'M';
    else GM_Values[13] = 'm';
    GM_Values[14] = ',';
    GM_Values[15] = map(analogRead(A0),0,1023,64,82);
    GM_Values[16] = ',';
    GM_Values[17] = map(analogRead(A1),0,1023,64,82);
    GM_Values[18] = ']';
    GM_Values[19] = 0x00;
    if (GM_Update + GM_REFRESH < millis()) {
      GM_Update = millis();
      //send values
      Serial.println(GM_Values);
      /*
      ## Send frame ##
      '['
      'u' | 'U'
      ','
      'l' | 'L'
      ','
      'r' | 'R'
      ','
      'd' | 'D'
      ','
      'a' | 'A'
      ','
      'b' | 'B'
      ','
      'm' | 'M'
      ','
      '@' - 'R'
      ','
      '@' - 'R'
      ']'
      0x0D
      0x0A
      */
    }
  }
  else if (function == 1) {
    //sync signal
    if ((RC_sync + 750) < millis()) {
      RC_sync = millis();
      Serial.write(RC_SYNC);
    }
    //claxon
    if (SelectObj.read()) Serial.write(RC_CLAXON_ON);
    else Serial.write(RC_CLAXON_OFF);
    //lights
    if (StartObj.read()) RC_pushed = true;
    else {
      if (RC_pushed) {
        RC_pushed = false;
        RC_fedge = true;
      }
    }
    if (RC_fedge) {
      RC_fedge = false;
      RC_lights = !RC_lights;
    }
    if (RC_lights) Serial.write(RC_LIGHT_ON);
    else Serial.write(RC_LIGHT_OFF);
    //movement
    X = map(analogRead(A0),0,1023,0,14);
    switch (X) {
      case 0:
        Serial.write(RC_SET_STEERING_L7);
        break;
      case 1:
        Serial.write(RC_SET_STEERING_L6);
        break;
      case 2:
        Serial.write(RC_SET_STEERING_L5);
        break;
      case 3:
        Serial.write(RC_SET_STEERING_L4);
        break;
      case 4:
        Serial.write(RC_SET_STEERING_L3);
        break;
      case 5:
        Serial.write(RC_SET_STEERING_L2);
        break;
      case 6:
        Serial.write(RC_SET_STEERING_L1);
        break;
      case 7:
        Serial.write(RC_SET_STEERING_LR);
        break;
      case 8:
        Serial.write(RC_SET_STEERING_R1);
        break;
      case 9:
        Serial.write(RC_SET_STEERING_R2);
        break;
      case 10:
        Serial.write(RC_SET_STEERING_R3);
        break;
      case 11:
        Serial.write(RC_SET_STEERING_R4);
        break;
      case 12:
        Serial.write(RC_SET_STEERING_R5);
        break;
      case 13:
        Serial.write(RC_SET_STEERING_R6);
        break;
      case 14:
        Serial.write(RC_SET_STEERING_R7);
        break;
    }
    Y = map(analogRead(A1),0,1023,19,0);
    switch (Y) {
      case 0:
      case 1:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_9);
        break;
      case 2:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_8);
        break;
      case 3:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_7);
        break;
      case 4:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_6);
        break;
      case 5:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_5);
        break;
      case 6:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_4);
        break;
      case 7:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_3);
        break;
      case 8:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_2);
        break;
      case 9:
        Serial.write(RC_REV);
        Serial.write(RC_SPEED_1);
        break;
      case 10:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_0);
        break;
      case 11:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_1);
        break;
      case 12:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_2);
        break;
      case 13:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_3);
        break;
      case 14:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_4);
        break;
      case 15:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_5);
        break;
      case 16:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_6);
        break;
      case 17:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_7);
        break;
      case 18:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_8);
        break;
      case 19:
        Serial.write(RC_FORW);
        Serial.write(RC_SPEED_9);
    }
  }
}
