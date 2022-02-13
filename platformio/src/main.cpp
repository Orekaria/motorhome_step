// Orekaria - main

#include <Arduino.h>
#include <Simple_MPU6050.h>
#include <Shared.h>
#include <Mpu6050.h>
#include <PowerConsumption.h>

enum class DStates {
   ON = 0x00,
   OFF = 0x01,
   OPEN,
   CLOSE,
   NA,
};

#define MOSFET_PIN 11
#define SWITCH_CLOSE_PIN 4
#define SWITCH_OPEN_PIN 5
#define RELAY_OPEN_PIN 8
#define RELAY_CLOSE_PIN 9
#ifdef DEBUG
#define STEP_TIME 1000
#define AUTO_CLOSE_AFTER 7000
#else
#define STEP_TIME 2000 // should be the time required by the step to finish
#define AUTO_CLOSE_AFTER 13000 // the step will auto-close after this interval
#endif
#define AUTO_CLOSE_DISABLE_WINDOW 2000 // if the open-step switch is pressed within this time after the STEP_TIME, the step is locked in the open position until close-step switch is pressed

volatile bool isStepOpened = true; // assume that the step is opened (extended) when the arduino boots up
volatile bool isAutocloseActivated = false;
volatile bool isInAction = false;
volatile bool isStepLocked = false;

unsigned long startTime;
unsigned long startTimeAutoClose;

Simple_MPU6050 mpu;
Mpu6050 mpu6050 = Mpu6050(INTERRUPT_MPU6050_PIN);

volatile bool _isSettingUpMotionDetector = false;
volatile bool _isMotionDetected = false;
void motionDetected() {
   if (_isSettingUpMotionDetector) {
      return;
   }
   _isMotionDetected = true;
   LOG("motion detected" + CARRIAGE_RETURN);
}

bool isMotionDetected() {
   return _isMotionDetected;
}

void resetIsMotionDetected() {
   _isMotionDetected = false;
}

PowerConsumption powerConsumtion = PowerConsumption();

void motionDetection(DStates onOrOff) {
   switch (onOrOff) {
   case DStates::ON:
   {
      if (_isSettingUpMotionDetector) {
         return;
      }
      _isSettingUpMotionDetector = true;
      digitalWrite(MOSFET_PIN, HIGH);
      delay(powerConsumtion.toCPUTime(200));

      bool isMotionDetectorPresent = mpu6050.test();

      if (isMotionDetectorPresent) {
         // be sure that the relays are not closed because activating the motion detection is not async and would keep the step in motion
         // if any relay is closed at this point, the flow of the program is wrong
         digitalWrite(RELAY_OPEN_PIN, LOW);
         digitalWrite(RELAY_CLOSE_PIN, LOW);

         //// MPU-6050
         // Setup the MPU
         mpu.Set_DMP_Output_Rate_Hz(10);           // Set the DMP output rate from 200Hz to 5 Minutes.
         //mpu.Set_DMP_Output_Rate_Seconds(10);    // Set the DMP output rate in Seconds
         //mpu.Set_DMP_Output_Rate_Minutes(5);     // Set the DMP output rate in Minute
         mpu.SetAddress(MPU6050_ADDRESS);          // Sets the address of the MPU.
         mpu.CalibrateMPU(30, false);              // Calibrates the accelerometer but not the gyros because we are disabling them later, to save power
         mpu.load_DMP_Image();                     // Loads the DMP image into the MPU and finish configuration.
         mpu6050.detectMotionSetup();

         attachInterrupt(digitalPinToInterrupt(INTERRUPT_MPU6050_PIN), motionDetected, FALLING);

         // tell the user that the motion detection is on
         digitalWrite(BUZZER_PIN, HIGH);
         delay(powerConsumtion.toCPUTime(100));
         digitalWrite(BUZZER_PIN, LOW);
      } else {
         digitalWrite(MOSFET_PIN, LOW);
         for (uint8_t i = 0; i < 3; i++) {
            digitalWrite(BUZZER_PIN, HIGH);
            delay(powerConsumtion.toCPUTime(50));
            digitalWrite(BUZZER_PIN, LOW);
            delay(powerConsumtion.toCPUTime(50));
         }
      }
      break;
   }
   case DStates::OFF:
   {
      detachInterrupt(digitalPinToInterrupt(INTERRUPT_MPU6050_PIN));
      delay(powerConsumtion.toCPUTime(10));
      digitalWrite(MOSFET_PIN, LOW);
      resetIsMotionDetected();
      break;
   }
   default:
      LOG("ERROR: unknown onOrOff (" + String((int)onOrOff) + ")" + CARRIAGE_RETURN);
      break;
   }
   _isSettingUpMotionDetector = false;
}

void openCloseStep(DStates openOrClose) {
   switch (openOrClose) {
   case DStates::OPEN:
      if (!isStepOpened) {
         LOG("openCloseStep OPEN" + CARRIAGE_RETURN);
         isInAction = true;
         digitalWrite(RELAY_CLOSE_PIN, LOW);
         delay(powerConsumtion.toCPUTime(10));
         digitalWrite(RELAY_OPEN_PIN, HIGH);
         isStepOpened = true;
         startTime = millis();
         startTimeAutoClose = millis();
         isAutocloseActivated = true;
      } else {
         startTimeAutoClose = millis(); // restart the timer if the open-step switch is pressed again
         if ((millis() - startTime) > powerConsumtion.toCPUTime(STEP_TIME + 100) && (millis() - startTime < powerConsumtion.toCPUTime(STEP_TIME + AUTO_CLOSE_DISABLE_WINDOW))) { // ... if the user has pushed the button after the step has just opened, and before the AUTO_CLOSE_DISABLE_WINDOW has passed, lock the step as opened
            if (isAutocloseActivated) {
               if (isInAction) {
                  LOG("ERROR: the flow is wrong if the step is already in action");
               } else {
                  isAutocloseActivated = false;
                  motionDetection(DStates::ON); // motion activation is activated when the step is permanently extended
                  LOG("isAutocloseActivated false" + CARRIAGE_RETURN);
               }
            }
         }
      }
      break;
   case DStates::CLOSE:
      if (isStepOpened) {
         LOG("openCloseStep CLOSE" + CARRIAGE_RETURN);
         isInAction = true;
         startTime = millis();
         motionDetection(DStates::OFF);
         digitalWrite(RELAY_OPEN_PIN, LOW);
         delay(powerConsumtion.toCPUTime(10));
         digitalWrite(RELAY_CLOSE_PIN, HIGH);
         isStepOpened = false;
         isAutocloseActivated = false;
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
      if ((millis() - startTime) > powerConsumtion.toCPUTime(STEP_TIME)) { // ... and the time to open/close has expired
         LOG("isInAction expired" + CARRIAGE_RETURN);
         isInAction = false;
         // release both relays
         digitalWrite(RELAY_OPEN_PIN, LOW);
         digitalWrite(RELAY_CLOSE_PIN, LOW);
      }
   } else if (isAutocloseActivated) {
      if ((millis() - startTimeAutoClose) > powerConsumtion.toCPUTime(AUTO_CLOSE_AFTER)) { // if the elapsed time has ended, auto-close the step
         LOG("isInAction auto-CLOSE" + CARRIAGE_RETURN);
         openCloseStep(DStates::CLOSE);
         isAutocloseActivated = false;
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

   delay(powerConsumtion.toCPUTime(100));
   LOG("_");

   if (!isInAction) {
      digitalWrite(BUZZER_PIN, LOW);
      if (!isAutocloseActivated) {
         powerConsumtion.low();
         powerConsumtion.sleep(); // only will wake up by an interruption
          // the execution continues here after the interruption has been processed
         powerConsumtion.high();
      }
      if (isMotionDetected()) {
         resetIsMotionDetected();
         digitalWrite(BUZZER_PIN, HIGH);
         if (isStepOpened) {
            openCloseStep(DStates::CLOSE);  // be sure that the step is closed when the vehicle is moving
         }
      }
   }
}

void setup() {
   powerConsumtion.high();

   pinMode(SWITCH_OPEN_PIN, INPUT_PULLUP);
   pinMode(SWITCH_CLOSE_PIN, INPUT_PULLUP);
   pinMode(INTERRUPT_BUTTONS_PIN, INPUT);
   pinMode(INTERRUPT_MPU6050_PIN, INPUT_PULLUP); // When you set the mode to INPUT_PULLUP, an internal resistor – inside the Arduino board – will be set between the digital pin 4 and VCC (5V). This resistor – value estimated between 20k and 50k Ohm – will make sure the state stays HIGH. When you press the button, the states becomes LOW
   pinMode(MOSFET_PIN, OUTPUT);

   pinMode(BUZZER_PIN, OUTPUT);
   pinMode(RELAY_OPEN_PIN, OUTPUT);
   pinMode(RELAY_CLOSE_PIN, OUTPUT);

   digitalWrite(MOSFET_PIN, LOW);
   digitalWrite(RELAY_OPEN_PIN, LOW);
   digitalWrite(RELAY_CLOSE_PIN, LOW);

   digitalWrite(BUZZER_PIN, HIGH);
   delay(powerConsumtion.toCPUTime(50));
   digitalWrite(BUZZER_PIN, LOW);

   // LOW to trigger the interrupt whenever the pin is low,
   // CHANGE to trigger the interrupt whenever the pin changes value
   // RISING to trigger when the pin goes from low to high,
   // FALLING for when the pin goes from high to low.
   attachInterrupt(digitalPinToInterrupt(INTERRUPT_BUTTONS_PIN), checkUserInput, RISING);

   openCloseStep(DStates::CLOSE);  // be sure that the step is closed when the arduino is powered up
}
