#include <TM1638plus.h>

// GPIO I/O pins on the Arduino connected to strobe, clock, data,
//pick on any I/O you want.
#define  STROBE_TM 4
#define  CLOCK_TM 6
#define  DIO_TM 7
TM1638plus tm(STROBE_TM, CLOCK_TM , DIO_TM);

#define buttonPin 8
#define endDnPin 9
#define endUpPin 10
#define movDnOrEndPin 11
#define movUpOrEndPin 12
#define impulsPin 13


//States
enum Zustand {StopDN, MovingUP, StopUP, MovingDN} aktuellerZustand = StopDN;
uint8_t position = 50; //0 Open //1 fully closed
bool lastButtonpress = false;


unsigned long lastMillis = millis();
#define millisStep 500
void simstep() {
  uint8_t allButtons = tm.readButtons();
  boolean buttonpress = allButtons & 1 || digitalRead(buttonPin);
  boolean buttonFlankRising = buttonpress & !lastButtonpress;
  lastButtonpress = buttonpress;

  switch(aktuellerZustand){
    case StopDN:
      tm.displayText("STOP");
      if(buttonFlankRising)
        aktuellerZustand = MovingUP;
      break;
    case MovingUP:
      tm.displayText("nnUP");
      position --;
      if (position<= 0) {
        position = 0;
        aktuellerZustand = StopUP;
      }else if(buttonFlankRising)
        aktuellerZustand = StopUP;
      break;
    case StopUP:
      tm.displayText("STOP");
      if(buttonFlankRising)
        aktuellerZustand = MovingDN;
      break;
    case MovingDN:
      tm.displayText("nnDN");
      position ++;
      if (position>= 100) {
        position = 100;
        aktuellerZustand = StopDN;
      }else 
      if(buttonFlankRising)
        aktuellerZustand = StopDN;
      break; 
  }

  //tm.display7Seg(4, uint8_t value);
  tm.displayHex(5,position/100%10);
  tm.displayHex(6,position/10%10);
  tm.displayHex(7,position%10);

  //Ausgabe
  boolean endUp = false, endDn = false;
  if(position==0){
    endUp=true;
  }
  if(position==100){
    endDn = true;
  }

  boolean relayUp = false, relayDn = false;
  switch(aktuellerZustand){      
    case MovingUP:
      relayUp = true;
      break;
    case MovingDN:
      relayDn = true;
      break;      
  }

  digitalWrite(endDnPin, !endDn);
  tm.setLED(0, !endDn);
  digitalWrite(endUpPin, !endUp);
  tm.setLED(1, !endUp);
  digitalWrite(movUpOrEndPin, !relayUp&&!endUp);
  tm.setLED(2, !relayUp&&!endUp);
  digitalWrite(movDnOrEndPin, !relayDn&&!endDn);
  tm.setLED(3, !relayDn&&!endDn);
}




void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT);
  pinMode(endDnPin, OUTPUT);
  pinMode(endUpPin, OUTPUT);
  pinMode(movDnOrEndPin, OUTPUT);
  pinMode(movUpOrEndPin, OUTPUT);
  pinMode(impulsPin, OUTPUT);
}

boolean impulsRunning = false;
unsigned long lastImpulsMillis;
void loop() {
  if(lastMillis+ millisStep <=millis())
  {
    lastMillis += millisStep;
    simstep();
  }

  switch(aktuellerZustand){
    case MovingUP:
    case MovingDN:
      if(!impulsRunning){
        lastImpulsMillis = millis();
      }
      if(digitalRead(impulsPin))
      {
        if(lastImpulsMillis+ 100 <=millis()){
          digitalWrite(impulsPin,0);
          lastImpulsMillis = millis();
        }
      } else {
        if(lastImpulsMillis+ 25 <=millis()){
          digitalWrite(impulsPin,1);
          lastImpulsMillis = millis();
        }
      }
    
      impulsRunning = true;
      break;

    default:
      impulsRunning = false;
  }

}
