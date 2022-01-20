#include <Arduino.h>
#include <LowPower.h>
#include <Shared.h>
#include <Utils.h>

const char OPEN = 1;
const char CLOSE = 0;
const int INTERRUPT_0 = 0; // Arduino Nano pin 2
const int INTERRUPT_1 = 1; // Arduino Nano pin 3
const int SWITCH_CLOSE_PIN = 2;
const int SWITCH_OPEN_PIN = 3;
const int HALLPin = 5;
const int DOOR_OPEN_PIN = 8;
const int DOOR_CLOSE_PIN = 9;
const int STEP_TIME = 2000; // should be the time required by the step to finish
const int AUTO_CLOSE_AFTER = 7000;
const int AUTO_CLOSE_DISABLE_WINDOW = 2000; // if the open-step switch is pressed within this time after the STEP_TIME, the step is locked in the open position until close-step switch is pressed

volatile bool isStepOpened;
volatile bool isAutocloseActivated = false;
volatile bool isInAction = false;
volatile bool isStepLocked = false;

unsigned long startTime;
unsigned long startTimeAutoClose;

void powerDown() {
   Utils::log("powerDown" + CARRIAGE_RETURN);
   LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

#if DEBUG == false
void updateLedStatus() {
   Utils::log("updateLedStatus " + String(isStepOpened) + CARRIAGE_RETURN);
   if (isStepOpened) {
      digitalWrite(LED_BUILTIN, HIGH);
   } else {
      digitalWrite(LED_BUILTIN, LOW);
   }
}
#endif

void stepStatus(char desiredStatus) {
   if (desiredStatus == CLOSE) {
#if DEBUG == true
      Utils::log("stepStatus CLOSE" + CARRIAGE_RETURN);
#endif
      digitalWrite(DOOR_OPEN_PIN, HIGH);
      delay(10);
      digitalWrite(DOOR_CLOSE_PIN, LOW);
      isStepOpened = false;
      isAutocloseActivated = false;
   } else if (desiredStatus == OPEN) {
#if DEBUG == true
      Utils::log("stepStatus OPEN" + CARRIAGE_RETURN);
#endif
      digitalWrite(DOOR_CLOSE_PIN, HIGH);
      delay(10);
      digitalWrite(DOOR_OPEN_PIN, LOW);
      isStepOpened = true;
      isAutocloseActivated = true;
   }
   isInAction = true;
   startTime = millis();
   startTimeAutoClose = startTime;
}

void checkUserInput() {
   // read user input
   if (digitalRead(SWITCH_CLOSE_PIN)) {
      if (isStepOpened) {
         stepStatus(CLOSE);
      }
   } else if (digitalRead(SWITCH_OPEN_PIN)) { // if the open-step switch is pressed
      if (!isStepOpened) {
         stepStatus(OPEN);
      } else {
         startTimeAutoClose = millis(); // restart the timer if the open-step switch is pressed
         if (millis() - startTime > STEP_TIME && millis() - startTime < STEP_TIME + AUTO_CLOSE_DISABLE_WINDOW) { // ... if after the step has opened, and before the AUTO_CLOSE_DISABLE_WINDOW has passed, lock the step as opened
            isAutocloseActivated = false;
            Utils::log("isAutocloseActivated false" + CARRIAGE_RETURN);
         }
      }
   }
}

void doDelayedActions() {
   if (isInAction) { // if the step is being opened/closed
      Utils::log(".");
      if ((millis() - startTime) > STEP_TIME) { // ... and the time to open/close has expired
         Utils::log("isInAction expired" + CARRIAGE_RETURN);
         isInAction = false;
         digitalWrite(DOOR_OPEN_PIN, HIGH);
         digitalWrite(DOOR_CLOSE_PIN, HIGH);
      }
   } else if (isAutocloseActivated) {
      if ((millis() - startTimeAutoClose) > AUTO_CLOSE_AFTER) { // if the elapsed time has ended, auto-close the step
         Utils::log("isInAction auto-CLOSE" + CARRIAGE_RETURN);
         isAutocloseActivated = false;
         stepStatus(CLOSE);
      }
   }
}

void loop() {
   doDelayedActions();

   delay(100);

   if (!isAutocloseActivated && !isInAction) {
      powerDown();
   }
}

void setup() {
#if DEBUG == true
   Serial.begin(SERIAL_SPEED);
   Utils::log("Serial initialized" + CARRIAGE_RETURN);
#endif

   pinMode(LED_BUILTIN, OUTPUT);
   pinMode(HALLPin, INPUT);
   pinMode(SWITCH_OPEN_PIN, INPUT);
   pinMode(SWITCH_CLOSE_PIN, INPUT);
   pinMode(DOOR_OPEN_PIN, OUTPUT);
   pinMode(DOOR_CLOSE_PIN, OUTPUT);

   digitalWrite(LED_BUILTIN, LOW);
   digitalWrite(DOOR_OPEN_PIN, HIGH);
   digitalWrite(DOOR_CLOSE_PIN, LOW);

   // LOW to trigger the interrupt whenever the pin is low,
   // CHANGE to trigger the interrupt whenever the pin changes value
   // RISING to trigger when the pin goes from low to high,
   // FALLING for when the pin goes from high to low.
   attachInterrupt(digitalPinToInterrupt(SWITCH_OPEN_PIN), checkUserInput, RISING);
   attachInterrupt(digitalPinToInterrupt(SWITCH_CLOSE_PIN), checkUserInput, RISING);

   stepStatus(CLOSE);  // be sure that the step is closed when the arduino is initialized
}
