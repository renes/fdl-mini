#include <Servo.h>

// common values
const int ANALOG_ON = 255;
const int ANALOG_OFF =  0;
const int TRIGGER_PUSHED = 1;
const int ESC_MOTOR_SPEED_STOP = 1000;

// PINS
const int ESC_1_PIN = 10;
const int ESC_2_PIN =  9;

const int PUSHER_SWITCH_FRONT_PIN =  A3;
const int PUSHER_SWITCH_BACK_PIN = A4;

const int TRIGGER_SWITCH_PIN = 6;
const int FAST_MODE_SWITCH_PIN =  7;

const int PUSHER_MOTOR_PIN_1 = A5;
const int PUSHER_MOTOR_PIN_2 = A6;

enum fireMode {
  SINGLE,
  AUTO
};

enum state {
  Idle,
  Shooting,
  WAITING_DETRIGGER
};

Servo ESC1;
Servo ESC2;

int escMotorRampUpTimeInMs = 300; // delay between ESC starting and pusher starting
int motorSpeed = 2000; // 1000 - 2000

state currentState = Idle;
fireMode currentFireMode = SINGLE;
boolean waitingDetrigger = false;

void setup() {

  ESC1.attach(ESC_1_PIN);
  ESC2.attach(ESC_2_PIN);

  ESC1.writeMicroseconds(1000);
  ESC2.writeMicroseconds(1000);

  pinMode(TRIGGER_SWITCH_PIN, INPUT);
  pinMode(FAST_MODE_SWITCH_PIN, INPUT);

  Serial.begin(9600);

  Serial.println("Booting done ...");

}

void loop() {

  int triggerValue = digitalRead(TRIGGER_SWITCH_PIN);

  // single shot was fired, waiting for the trigger button to be released to prevent another shot to be fired
  if (waitingDetrigger == true && triggerValue == 0) {
    waitingDetrigger = false;
  }

  if (waitingDetrigger == true) {
    return;
  }

  // set fire mode, SINGLE or AUTO
  int mode = digitalRead(FAST_MODE_SWITCH_PIN);

  if (mode == 1) {
    currentFireMode = AUTO;
  } else {
    currentFireMode = SINGLE;
  }

  // let's start the fun, trigger was pushed
  if (triggerValue == TRIGGER_PUSHED && currentState == Idle) {

    if (currentFireMode == SINGLE) {
      fireSingleShot();
      waitingDetrigger = true;
    }

    if (currentFireMode == AUTO) {
      currentState = Shooting;
      Serial.println("Auto Mode ON - Start firing");
      startFiring();
    }
  }

  // trigger is released, let's stop
  if (triggerValue != TRIGGER_PUSHED && currentState == Shooting) {
    stopFiring();
    currentState = Idle;
  }

}

boolean backPusherSwitchHit() {
  return analogRead(PUSHER_SWITCH_BACK_PIN) > 50;
}

boolean backPusherSwitchFront() {
  return analogRead(PUSHER_SWITCH_FRONT_PIN) > 50;
}

void fireSingleShot() {
  Serial.println("Auto Mode OFF - Start single shot");
  startFiring();
  stopFiring();
}

void startFiring() {
  StartESC();
  delay(escMotorRampUpTimeInMs);
  ForwardPusher();
}

void stopFiring() {

  while (backPusherSwitchFront() == false) {

  }

  delay(5);
  StopPusher();

  unsigned long timeStartedToWait = millis();
  while (backPusherSwitchHit() == false) {
    Serial.println("checking switch");
    // if back end stop not reached give it a short hit to go there
    if (millis() > timeStartedToWait + 100) {
      Serial.println("move forward");
      ForwardPusher();
      delay(1);
      StopPusher();
      timeStartedToWait = millis();
    }
  }

  StopESC();

  Serial.println("Stopped firing");
}

void StartESC() {
  ESC1.writeMicroseconds(motorSpeed);
  ESC2.writeMicroseconds(motorSpeed);
  Serial.println("Started ESC");
}

void StopESC() {
  ESC1.writeMicroseconds(ESC_MOTOR_SPEED_STOP);
  ESC2.writeMicroseconds(ESC_MOTOR_SPEED_STOP);
  Serial.println("Stopped ESC");
}


void ForwardPusher()
{
  analogWrite(PUSHER_MOTOR_PIN_1, ANALOG_ON);
  analogWrite(PUSHER_MOTOR_PIN_2, ANALOG_OFF);
  Serial.println("Started pusher");
}

void StopPusher()
{
  analogWrite(PUSHER_MOTOR_PIN_1, ANALOG_OFF);
  analogWrite(PUSHER_MOTOR_PIN_2, ANALOG_OFF);
  Serial.println("Stopped pusher");
}
