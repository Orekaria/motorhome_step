// Orekaria - main

#include <Arduino.h>
#include <Simple_MPU6050.h>
#include <LowPower.h>
#include <Shared.h>
#include <Mpu6050.h>
// #include <Thermostat.h>

#define OPEN 1
#define CLOSE 0
const int SWITCH_CLOSE_PIN = 4;
const int SWITCH_OPEN_PIN = 5;
const int DOOR_OPEN_PIN = 8;
const int DOOR_CLOSE_PIN = 9;
#ifdef DEBUG
const int STEP_TIME = 1000;
const int AUTO_CLOSE_AFTER = 7000;
#else
const int STEP_TIME = 2000; // should be the time required by the step to finish
const int AUTO_CLOSE_AFTER = 13000; // the step will auto-close after this interval
#endif
const int AUTO_CLOSE_DISABLE_WINDOW = 2000; // if the open-step switch is pressed within this time after the STEP_TIME, the step is locked in the open position until close-step switch is pressed

volatile bool isStepOpened;
volatile bool isAutocloseActivated = false;
volatile bool isInAction = false;
volatile bool isStepLocked = false;

unsigned long startTime;
unsigned long startTimeAutoClose;

Simple_MPU6050 mpu;
Mpu6050 mpu6050 = Mpu6050(INTERRUPT_MPU6050_PIN);

void powerDown() {
   LOG("power DOWN" + CARRIAGE_RETURN);
   LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void openCloseStep(int openOrClose) {
   switch (openOrClose) {
   case OPEN:
      if (!isStepOpened) {
         LOG("openCloseStep OPEN" + CARRIAGE_RETURN);
         digitalWrite(DOOR_CLOSE_PIN, HIGH);
         delay(10);
         digitalWrite(DOOR_OPEN_PIN, LOW);
         isStepOpened = true;
         isAutocloseActivated = true;
      } else {
         startTimeAutoClose = millis(); // restart the timer if the open-step switch is pressed
         if (millis() - startTime > STEP_TIME && millis() - startTime < STEP_TIME + AUTO_CLOSE_DISABLE_WINDOW) { // ... if after the step has opened, and before the AUTO_CLOSE_DISABLE_WINDOW has passed, lock the step as opened
            isAutocloseActivated = false;
            LOG("isAutocloseActivated false" + CARRIAGE_RETURN);
         }
      }
      break;
   case CLOSE:
      if (isStepOpened) {
         LOG("openCloseStep CLOSE" + CARRIAGE_RETURN);
         digitalWrite(DOOR_OPEN_PIN, HIGH);
         delay(10);
         digitalWrite(DOOR_CLOSE_PIN, LOW);
         isStepOpened = false;
         isAutocloseActivated = false;
      }
      break;
   default:
      LOG("ERROR: desired status unknown");
      break;
   }
   isInAction = true;
   startTime = millis();
   startTimeAutoClose = startTime;
}

void checkUserInput() {
   // read user input
   if (digitalRead(SWITCH_CLOSE_PIN)) {
      openCloseStep(CLOSE);
   } else if (digitalRead(SWITCH_OPEN_PIN)) { // if the open-step switch is pressed
      openCloseStep(OPEN);
   }
}

void doDelayedActions() {
   if (isInAction) { // if the step is being opened/closed
      LOG(".");
      if ((millis() - startTime) > STEP_TIME) { // ... and the time to open/close has expired
         LOG("isInAction expired" + CARRIAGE_RETURN);
         isInAction = false;
         digitalWrite(DOOR_OPEN_PIN, HIGH);
         digitalWrite(DOOR_CLOSE_PIN, HIGH);
      }
   } else if (isAutocloseActivated) {
      if ((millis() - startTimeAutoClose) > AUTO_CLOSE_AFTER) { // if the elapsed time has ended, auto-close the step
         LOG("isInAction auto-CLOSE" + CARRIAGE_RETURN);
         isAutocloseActivated = false;
         openCloseStep(CLOSE);
      }
   }
}

volatile bool _isMotionDetected = false;
void motionDetected() {
   _isMotionDetected = true;
}

bool isMotionDetected() {
   return _isMotionDetected;
}

void resetIsMotionDetected() {
   _isMotionDetected = false;
}

void loop() {
   doDelayedActions();

   delay(200);
   LOG("_");

   if (!isInAction) {
      if (!isAutocloseActivated) {
         digitalWrite(BUZZER_PIN, LOW);
         powerDown();
      }
      digitalWrite(BUZZER_PIN, LOW);
      if (isMotionDetected()) {
         resetIsMotionDetected();
         LOG(CARRIAGE_RETURN + "motion detected" + CARRIAGE_RETURN);
         if (isStepOpened) {
            digitalWrite(BUZZER_PIN, HIGH);
            openCloseStep(CLOSE);  // be sure that the step is closed when the vehicle is moving
         }
      }
   }
}

void setup() {
#ifdef DEBUG
   Serial.begin(SERIAL_SPEED);
   LOG("Serial initialized" + CARRIAGE_RETURN);
#endif

   pinMode(LED_BUILTIN, OUTPUT);
   pinMode(BUZZER_PIN, OUTPUT);
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

   pinMode(INTERRUPT_BUTTONS_PIN, INPUT);
   attachInterrupt(digitalPinToInterrupt(INTERRUPT_BUTTONS_PIN), checkUserInput, RISING);

   //// MPU-6050
   // join I2C bus (I2Cdev library doesn't do this automatically)
   Wire.begin();
   Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
   // Setup the MPU
   mpu.Set_DMP_Output_Rate_Hz(10);           // Set the DMP output rate from 200Hz to 5 Minutes.
   //mpu.Set_DMP_Output_Rate_Seconds(10);   // Set the DMP output rate in Seconds
   //mpu.Set_DMP_Output_Rate_Minutes(5);    // Set the DMP output rate in Minute
   mpu.SetAddress(MPU6050_ADDRESS); //Sets the address of the MPU.
   mpu.CalibrateMPU();                      // Calibrates the MPU.
   mpu.load_DMP_Image();                    // Loads the DMP image into the MPU and finish configuration.
   mpu6050.detectMotionSetup();
   pinMode(INTERRUPT_MPU6050_PIN, INPUT_PULLUP); // When you set the mode to INPUT_PULLUP, an internal resistor – inside the Arduino board – will be set between the digital pin 4 and VCC (5V). This resistor – value estimated between 20k and 50k Ohm – will make sure the state stays HIGH. When you press the button, the states becomes LOW
   attachInterrupt(digitalPinToInterrupt(INTERRUPT_MPU6050_PIN), motionDetected, FALLING);

   // check the buzzer
   digitalWrite(BUZZER_PIN, HIGH);
   delay(100);
   digitalWrite(BUZZER_PIN, LOW);

   openCloseStep(CLOSE);  // be sure that the step is closed when the arduino is initialized
}
