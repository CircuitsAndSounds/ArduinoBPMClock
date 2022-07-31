#include <TM1637.h>


#define  ENCODER_OPTIMIZE_INTERRUPTS
#include <Wire.h>
#include <Encoder.h>

Encoder myEnc(2, 3);
float oldPosition1  = -999;
float newPosition1  = -999;
float oldPosition2  = -999;
float newPosition2  = -999;
float oldPosition3  = -999;
float newPosition3  = -999;

byte enc_switch_in = 0;
unsigned long enc_switch_timer = 0;
bool enc_switch_latch = false;
byte enc_switch_counter = 0;
byte enc_switch = 0;
byte toggle_switch = 0;//0=EXT,1=ON
static int DOUBLE_CLICK_TIME = 500;

float BPM = 100.0;
float delayPerBeat = 0.0;
float msPerMin = 1000.0 * 60.0; //1000ms x 60s = 60,000ms per min
float bpmDelay = 0.0;

unsigned long currentTime = 0;
unsigned long previousTime = 0;        // will store last time LED was updated
unsigned long counter = 0;        // will store last time LED was updated

const int clockLed = 7;
const int clockLedDiv2 = 8;
const int clockLedDiv4 = 9;
const int clockLedDiv8 = 10;
const int clockLedDiv16 = 11;

int clockState = LOW;             // clockState used to set the LED
int clockStateDiv2 = LOW;             // clockState used to set the LED
int clockStateDiv4 = LOW;             // clockState used to set the LED
int clockStateDiv8 = LOW;             // clockState used to set the LED
int clockStateDiv16 = LOW;             // clockState used to set the LED


int prog = 0;

// Instantiation and pins configurations
// Pin 5 - > DIO
// Pin 4 - > CLK
TM1637 tm(4, 5);

void setup() {

  tm.begin();
  tm.setBrightness(4);

  pinMode(6, INPUT_PULLUP); //Encoder Button
  pinMode(clockLed, OUTPUT);
  pinMode(clockLedDiv2, OUTPUT);
  pinMode(clockLedDiv4, OUTPUT);
  pinMode(clockLedDiv8, OUTPUT);
  pinMode(clockLedDiv16, OUTPUT);

  Serial.begin(9600);


}


void loop() {
  hardwareCheck();
  segDisplay();
  clockOut();
  reset();
}


void hardwareCheck() {

  enc_switch_in  = !digitalRead(6);


  if ((enc_switch_in == true) && (enc_switch_latch == false)) {  //Count Times Button Pressed
    enc_switch_counter++;
    enc_switch_timer = millis();
    enc_switch_latch = true;
  }
  else if (enc_switch_in == false) {
    enc_switch_latch = false;
  }


  if ((millis() >= enc_switch_timer + DOUBLE_CLICK_TIME) && (enc_switch_in == false)) {  //Reset Count when no Button Pressed
    enc_switch_counter = 0;
  }


  if ((enc_switch_in == false) && (enc_switch_counter == 0)) {
    enc_switch = 0;
  }

  else if ((enc_switch_in == true) && (enc_switch_counter == 1)) {
    enc_switch = 1;

  }

  else if ((enc_switch_in == true) && (enc_switch_counter >= 2)) {
    enc_switch = 2;
  }


  if (enc_switch  == 0) {//GLOBAL BPM COARSE

    newPosition1 = myEnc.read();
    if ( (newPosition1 - 3) / 4  > oldPosition1 / 4) {
      oldPosition1 = newPosition1;
      BPM = BPM - 5;
    }

    else if ( (newPosition1 + 3) / 4  < oldPosition1 / 4 ) {
      oldPosition1 = newPosition1;
      BPM = BPM + 5;
    }

    if ( BPM < 60) {
      BPM = 160;
    }
    else if ( BPM >= 161 ) {
      BPM = 60;
    }
  }



  else if (enc_switch  == 1) {//GLOBAL BPM FINE

    newPosition1 = myEnc.read();
    if ( (newPosition1 - 3) / 4  > oldPosition1 / 4) {
      oldPosition1 = newPosition1;
      BPM = BPM - 1;
    }

    else if ( (newPosition1 + 3) / 4  < oldPosition1 / 4 ) {
      oldPosition1 = newPosition1;
      BPM = BPM + 1;
    }

    if ( BPM < 60) {
      BPM = 160;
    }
    else if ( BPM >= 161 ) {
      BPM = 60;
    }
  }



  else if (enc_switch  == 2) {//GLOBAL BPM FINE
    counter = 0;
    newPosition2 = myEnc.read();
    if ( (newPosition2 - 3) / 4  > oldPosition2 / 4) {
      oldPosition1 = newPosition2;
      oldPosition2 = newPosition2;
      prog = prog - 1;
    }

    else if ( (newPosition2 + 3) / 4  < oldPosition2 / 4 ) {
      oldPosition1 = newPosition2;
      oldPosition2 = newPosition2;
      prog = prog + 1;
    }

    if ( prog < 0) {
      prog = 4;
    }
    else if ( prog >= 5 ) {
      prog = 0;
    }
  }

}



void segDisplay() {

  if (enc_switch == 0) {
    if (BPM <= 99) {
      tm.display(BPM, false, false, 2);
    }

    else if (BPM <= 160) {
      tm.display(BPM, false, false, 1);
    }
  }


  else if (enc_switch == 1) {
    if (BPM <= 99) {
      tm.display(BPM, false, false, 2);
    }

    else if (BPM <= 160) {
      tm.display(BPM, false, false, 1);
    }
  }



  else if (enc_switch == 2) {

    switch (prog) {

      case 0:
        tm.display("1,2,4,8,16")->scrollLeft(150);
        break;

      case 1:
        tm.display("1,3,5,7,9")->scrollLeft(150);
        break;

      case 2:
        tm.display("1,2,3,4,5")->scrollLeft(150);
        break;

      case 3:
        tm.display("1,3,5,8,13")->scrollLeft(150);
        break;

      case 4:
        tm.display("1,2,4,8,16-F")->scrollLeft(150);
        break;

      default:
        tm.display("1,2,4,8,16")->scrollLeft(150);
        break;

    }
  }
}



void clockOut() {

  bpmDelay = (msPerMin / BPM) / 2;

  currentTime = millis();

  if (currentTime - previousTime >= bpmDelay) { //MASTER BPM
    counter++;
    // save the last time you blinked the LED
    previousTime = currentTime;
    // if the LED is off turn it on and vice-versa:
    clockState = !clockState;
    // set the LED with the clockState of the variable:
    digitalWrite(clockLed, clockState);


    switch (prog) {

      case 0:
        if (counter % 2 == 0) { //BPM DIVIDED BY 2
          // if the LED is off turn it on and vice-versa:
          clockStateDiv2 = !clockStateDiv2;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv2, clockStateDiv2);
        }


        if (counter % 4 == 0) { //BPM DIVIDED BY 4
          // if the LED is off turn it on and vice-versa:
          clockStateDiv4 = !clockStateDiv4;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv4, clockStateDiv4);
        }


        if (counter % 8 == 0) { //BPM DIVIDED BY 8
          // if the LED is off turn it on and vice-versa:
          clockStateDiv8 = !clockStateDiv8;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv8, clockStateDiv8);
        }


        if (counter % 16 == 0) { //BPM DIVIDED BY 16
          // if the LED is off turn it on and vice-versa:
          clockStateDiv16 = !clockStateDiv16;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv16, clockStateDiv16);
        }
        break;


      case 1:
        if (counter % 3 == 0) { //BPM DIVIDED BY 3
          // if the LED is off turn it on and vice-versa:
          clockStateDiv2 = !clockStateDiv2;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv2, clockStateDiv2);
        }


        if (counter % 5 == 0) { //BPM DIVIDED BY 5
          // if the LED is off turn it on and vice-versa:
          clockStateDiv4 = !clockStateDiv4;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv4, clockStateDiv4);
        }


        if (counter % 7 == 0) { //BPM DIVIDED BY 7
          // if the LED is off turn it on and vice-versa:
          clockStateDiv8 = !clockStateDiv8;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv8, clockStateDiv8);
        }


        if (counter % 9 == 0) { //BPM DIVIDED BY 9
          // if the LED is off turn it on and vice-versa:
          clockStateDiv16 = !clockStateDiv16;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv16, clockStateDiv16);
        }
        break;

      case 2:
        if (counter % 2 == 0) { //BPM DIVIDED BY 2
          // if the LED is off turn it on and vice-versa:
          clockStateDiv2 = !clockStateDiv2;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv2, clockStateDiv2);
        }


        if (counter % 3 == 0) { //BPM DIVIDED BY 3
          // if the LED is off turn it on and vice-versa:
          clockStateDiv4 = !clockStateDiv4;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv4, clockStateDiv4);
        }


        if (counter % 4 == 0) { //BPM DIVIDED BY 4
          // if the LED is off turn it on and vice-versa:
          clockStateDiv8 = !clockStateDiv8;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv8, clockStateDiv8);
        }


        if (counter % 5 == 0) { //BPM DIVIDED BY 5
          // if the LED is off turn it on and vice-versa:
          clockStateDiv16 = !clockStateDiv16;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv16, clockStateDiv16);
        }
        break;



      case 3:
        if (counter % 3 == 0) { //BPM DIVIDED BY 3
          // if the LED is off turn it on and vice-versa:
          clockStateDiv2 = !clockStateDiv2;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv2, clockStateDiv2);
        }


        if (counter % 5 == 0) { //BPM DIVIDED BY 5
          // if the LED is off turn it on and vice-versa:
          clockStateDiv4 = !clockStateDiv4;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv4, clockStateDiv4);
        }


        if (counter % 8 == 0) { //BPM DIVIDED BY 8
          // if the LED is off turn it on and vice-versa:
          clockStateDiv8 = !clockStateDiv8;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv8, clockStateDiv8);
        }


        if (counter % 13 == 0) { //BPM DIVIDED BY 13
          // if the LED is off turn it on and vice-versa:
          clockStateDiv16 = !clockStateDiv16;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv16, clockStateDiv16);
        }
        break;


      default:
        if (counter % 2 == 0) { //BPM DIVIDED BY 2
          // if the LED is off turn it on and vice-versa:
          clockStateDiv2 = !clockStateDiv2;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv2, clockStateDiv2);
        }


        if (counter % 4 == 0) { //BPM DIVIDED BY 4
          // if the LED is off turn it on and vice-versa:
          clockStateDiv4 = !clockStateDiv4;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv4, clockStateDiv4);
        }


        if (counter % 8 == 0) { //BPM DIVIDED BY 8
          // if the LED is off turn it on and vice-versa:
          clockStateDiv8 = !clockStateDiv8;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv8, clockStateDiv8);
        }


        if (counter % 16 == 0) { //BPM DIVIDED BY 16
          // if the LED is off turn it on and vice-versa:
          clockStateDiv16 = !clockStateDiv16;
          // set the LED with the clockState of the variable:
          digitalWrite(clockLedDiv16, clockStateDiv16);
        }
        break;

    }
  }
}

void reset() {

  if (counter == 0) {

    currentTime = millis();

    clockStateDiv2 = LOW;
    clockStateDiv4 = LOW;
    clockStateDiv8 = LOW;
    clockStateDiv16 = LOW;

    digitalWrite(clockLedDiv2, clockStateDiv2);
    digitalWrite(clockLedDiv4, clockStateDiv4);
    digitalWrite(clockLedDiv8, clockStateDiv8);
    digitalWrite(clockLedDiv16, clockStateDiv16);

  }
}


