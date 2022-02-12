#include <Arduino.h>
#include <avr/sleep.h>

// #define DEBUG
#define SERIAL_SPEED 57600

const char CARRIAGE_RETURN = "\n";
const char OPEN = 1;
const char CLOSE = 0;
const int LEDPin = LED_BUILTIN;
const int SWITCH_OPEN_PIN = 3;
const int SWITCH_CLOSE_PIN = 2;
const int HALLPin = 5;
const int DOOR_OPEN_PIN = 8;
const int DOOR_CLOSE_PIN = 9;
const int STEP_TIME = 2000; // should be the time required by the step to finish
const int AUTO_CLOSE_AFTER = 7000;
const int AUTO_CLOSE_DISABLE_WINDOW = 2000; // if the open-step switch is pressed within this time after the STEP_TIME, the step is locked in the open position until close-step switch is pressed

bool isStepOpened;
bool isAutocloseActivated = false;
bool isInAction = false;
unsigned long startTime;
unsigned long startTimeAutoClose;
bool isStepLocked = false;

#ifdef DEBUG
void updateLedStatus() {
   if (isStepOpened) {
      digitalWrite(LEDPin, HIGH);
   } else {
      digitalWrite(LEDPin, LOW);
   }
}
#endif

void stepStatus(char desiredStatus) {
   if (desiredStatus == CLOSE) {
#ifdef DEBUG
      Serial.print("stepStatus CLOSE" + CARRIAGE_RETURN);
#endif
      digitalWrite(DOOR_OPEN_PIN, HIGH);
      delay(10);
      digitalWrite(DOOR_CLOSE_PIN, LOW);
      isStepOpened = false;
      isAutocloseActivated = false;
   } else if (desiredStatus == OPEN) {
#ifdef DEBUG
      Serial.print("stepStatus OPEN" + CARRIAGE_RETURN);
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
         digitalWrite(LEDPin, HIGH);
      } else {
         startTimeAutoClose = millis(); // restart the timer if the open-step switch is pressed
         if (millis() - startTime > STEP_TIME && millis() - startTime < STEP_TIME + AUTO_CLOSE_DISABLE_WINDOW) { // ... if after the step has opened, and before the AUTO_CLOSE_DISABLE_WINDOW has passed, lock the step as opened
            isAutocloseActivated = false;
         }
      }
   }
}

void loop() {
   checkUserInput();

   // do delayed actions
   if (isInAction) { // if the step is being opened/closed
      if ((millis() - startTime) > STEP_TIME) { // ... and the time to open/close has expired
         isInAction = false;
         digitalWrite(LEDPin, LOW);
         digitalWrite(DOOR_OPEN_PIN, HIGH);
         digitalWrite(DOOR_CLOSE_PIN, HIGH);
      }
   } else if (isAutocloseActivated) {
      if ((millis() - startTimeAutoClose) > AUTO_CLOSE_AFTER) { // if the elapsed time has ended, auto-close the step
         isAutocloseActivated = false;
         stepStatus(CLOSE);
      }
   }

#ifdef DEBUG
   updateLedStatus();
#endif
}

void setup() {
#ifdef DEBUG
   Serial.begin(SERIAL_SPEED);
   Serial.println("Serial initialized");
   Serial.println("set_sleep_mode(SLEEP_MODE_PWR_SAVE)");
#endif

   // There are five different sleep modes of power saving:
   // SLEEP_MODE_IDLE - the lowest power saving mode
   // SLEEP_MODE_ADC
   // SLEEP_MODE_PWR_SAVE
   // SLEEP_MODE_STANDBY
   // SLEEP_MODE_PWR_DOWN - the highest power saving mode
   set_sleep_mode(SLEEP_MODE_PWR_SAVE);
   sleep_enable();

   // sleep_cpu ();
   // noInterrupts ();

   pinMode(LEDPin, OUTPUT);
   pinMode(HALLPin, INPUT);
   pinMode(SWITCH_OPEN_PIN, INPUT);
   pinMode(SWITCH_CLOSE_PIN, INPUT);
   pinMode(DOOR_OPEN_PIN, OUTPUT);
   pinMode(DOOR_CLOSE_PIN, OUTPUT);

   digitalWrite(LEDPin, LOW);
   digitalWrite(DOOR_OPEN_PIN, HIGH);
   digitalWrite(DOOR_CLOSE_PIN, LOW);

   stepStatus(CLOSE);  // be sure that the step is closed when the arduino is initialized
}
