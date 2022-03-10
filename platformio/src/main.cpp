// Orekaria - main

#include <Arduino.h>
#include <Shared.h>
#include <Mpu6050.h>
#include <MicrocontrollerState.h>

enum class DStates {
   ON = 0x00,
   OFF = 0x01,
   OPEN,
   CLOSE,
   NA,
};

#define MPU_ON_OFF_PIN 11
#define SWITCH_CLOSE_PIN 4
#define SWITCH_OPEN_PIN 5
#define RELAY_OPEN_PIN 8
#define RELAY_CLOSE_PIN 9
#define RELAY_CLOSED HIGH
#define RELAY_OPENED LOW

#ifdef DEBUG
#define STEP_TIME 1000
#define AUTO_CLOSE_AFTER 7000
#else
#define STEP_TIME 2000 // should be the time required by the step to finish
#define AUTO_CLOSE_AFTER 13000 // the step will auto-close after this interval
#endif
#define AUTO_CLOSE_DISABLE_WINDOW 2000 // if the open-step switch is pressed within this time after the STEP_TIME, the step is locked in the open position until close-step switch is pressed

volatile bool isStepOpened = false;
volatile bool isInAction = false;

uint32_t startTime;
uint32_t _lastMotionDetectedTime = UINT32_MAX;

Mpu6050 mpu6050 = Mpu6050(INTERRUPT_MPU6050_PIN, MPU_ON_OFF_PIN);
MicrocontrollerState microcontrollerState = MicrocontrollerState();


void openCloseStep(DStates openOrClose) {
   switch (openOrClose) {
   case DStates::OPEN:
      if (!isStepOpened) {
         LOG("openCloseStep OPEN" + CARRIAGE_RETURN);
         isInAction = true;
         digitalWrite(RELAY_CLOSE_PIN, RELAY_CLOSED);
         delay(microcontrollerState.toCPUTime(10));
         digitalWrite(RELAY_OPEN_PIN, RELAY_OPENED);
         isStepOpened = true;
         startTime = millis();
      } else {
         if (isInAction) {
            LOG("ERROR: the flow is wrong if the step is already in action");
         }
      }
      break;
   case DStates::CLOSE:
      if (isStepOpened) {
         LOG("openCloseStep CLOSE" + CARRIAGE_RETURN);
         isInAction = true;
         startTime = millis();
         mpu6050.motionDetection(MotionDetectionState::OFF);
         digitalWrite(RELAY_OPEN_PIN, RELAY_CLOSED);
         delay(microcontrollerState.toCPUTime(10));
         digitalWrite(RELAY_CLOSE_PIN, RELAY_OPENED);
         isStepOpened = false;
      }
      break;
   default:
      LOG("ERROR: unknown openOrClose (" + String((int)openOrClose) + ")" + CARRIAGE_RETURN);
      break;
   }
}

volatile DStates _switchInput = DStates::NA;
void checkUserInput() {
   // read user input
   _switchInput = DStates::NA;
   if (digitalRead(SWITCH_CLOSE_PIN)) {
      _switchInput = DStates::CLOSE;
   } else if (digitalRead(SWITCH_OPEN_PIN)) { // if the open-step switch is pressed
      _switchInput = DStates::OPEN;
   }
}

void doDelayedActions() {
   if (isInAction) { // if the step is being opened/closed
      LOG(".");
      if ((millis() - startTime) > microcontrollerState.toCPUTime(STEP_TIME)) { // ... and the time to open/close has expired
         // release both relays
         LOG("isInAction expired" + CARRIAGE_RETURN);
         isInAction = false;
         digitalWrite(RELAY_OPEN_PIN, RELAY_CLOSED);
         digitalWrite(RELAY_CLOSE_PIN, RELAY_CLOSED);
         if (isStepOpened) {
            mpu6050.motionDetection(MotionDetectionState::ON); // motion activation is activated when the step is permanently extended
         }
      }
   }
}

void loop() {
   if (_switchInput == DStates::NA) {
      doDelayedActions();
   } else {
      switch (_switchInput) {
      case DStates::NA:
         // do nothing
         break;
      case DStates::CLOSE:
         openCloseStep(DStates::CLOSE);
         break;
      case DStates::OPEN:
         openCloseStep(DStates::OPEN);
         break;
      default:
         LOG("ERROR: unknown _switchInput (" + String((int)_switchInput) + ")" + CARRIAGE_RETURN);
         break;
      }
   }
   _switchInput = DStates::NA;

   delay(microcontrollerState.toCPUTime(100));
   LOG("_");

   if (!isInAction) {
      microcontrollerState.low();
      microcontrollerState.sleep(); // only will wake up by an interruption
       // the execution continues here after the interruption has been processed
      microcontrollerState.high();

      if (mpu6050.isMotionDetected()) {
         if (millis() < _lastMotionDetectedTime + MOT_MIN_INTERVAL) {
            _lastMotionDetectedTime = UINT32_MAX;
            for (uint8_t i = 0; i < 5; i++) {
               tone(BUZZER_PIN, BUZZER_FREQUENCY);
               delay(microcontrollerState.toCPUTime(500));
               noTone(BUZZER_PIN);
               delay(microcontrollerState.toCPUTime(500));
            }
            openCloseStep(DStates::CLOSE);  // be sure that the step is closed when the vehicle is moving
         } else {
            _lastMotionDetectedTime = millis();
         }
      }
   }
}

void setup() {
   microcontrollerState.high();

   pinMode(BUZZER_PIN, OUTPUT);

   tone(BUZZER_PIN, BUZZER_FREQUENCY);
   delay(microcontrollerState.toCPUTime(50));
   noTone(BUZZER_PIN);

   // initialize pins for minimum current consumption
   // INPUT_PULLUP protects the pins but also can create small current spikes up to 1.1mA if, e.g. the pins are touched
   // OUTPUT/INPUT decreases current to minimum: 20nA at 3.2V. 60nA at 3.3V. 88nA at 4V.
   uint8_t initialState = OUTPUT;
   pinMode(0, INPUT); // RX
   pinMode(1, INPUT); // TX
   pinMode(13, OUTPUT);
   for (uint8_t a = 2; a <= 12; a++) {
      pinMode(a, initialState);
   }
   pinMode(A0, initialState);
   pinMode(A1, initialState);
   pinMode(A2, initialState);
   pinMode(A3, initialState);
   pinMode(A4, initialState);
   pinMode(A5, initialState);
   pinMode(A6, initialState);
   pinMode(A7, initialState);

   // When you set the mode to INPUT_PULLUP, an internal resistor – inside the Arduino board – will be set between the digital pin 4 and VCC. This resistor – value estimated between 20k and 50k Ohm – will make sure the state stays HIGH. When you press the button, the states becomes LOW
   // INPUT_PULLUP increases the current draw by 50-90nA if a diode is not added between the pin and the signal source
   pinMode(SWITCH_OPEN_PIN, INPUT);
   pinMode(SWITCH_CLOSE_PIN, INPUT);
   pinMode(INTERRUPT_BUTTONS_PIN, INPUT);
   pinMode(INTERRUPT_MPU6050_PIN, INPUT);

   pinMode(RELAY_OPEN_PIN, OUTPUT);
   pinMode(RELAY_CLOSE_PIN, OUTPUT);

   digitalWrite(RELAY_OPEN_PIN, RELAY_CLOSED);
   digitalWrite(RELAY_CLOSE_PIN, RELAY_CLOSED);

   // HIGH to trigger the interrupt whenever the pin is high,
   // LOW to trigger the interrupt whenever the pin is low,
   // CHANGE to trigger the interrupt whenever the pin changes value
   // RISING to trigger when the pin goes from low to high,
   // FALLING for when the pin goes from high to low.
   attachInterrupt(digitalPinToInterrupt(INTERRUPT_BUTTONS_PIN), checkUserInput, RISING);

   isStepOpened = true; // assume that the step is opened (extended) when the arduino boots up
   openCloseStep(DStates::CLOSE);  // be sure that the step is closed when the arduino is powered up
}
