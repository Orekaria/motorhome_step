// global variables

// #define DEBUG // uncomment to activate DEBUG mode

#define SERIAL_SPEED (uint32_t) 9600 // Serial monitor speed. Set the same value in platformio.ini

#define CARRIAGE_RETURN (String) "\n\r"

#define INTERRUPT_BUTTONS_PIN 3 // Arduino nano pin 2 and 3 can be selected
#define INTERRUPT_MPU6050_PIN 2 // pin used for waking up when MPU6050 detects motion
#define BUZZER_PIN 12           // pin for the buzzer
#define BUZZER_FREQUENCY 2000

#define MOT_THR_SELECTED 4 // motion is detected when any of the accelerometer messures exceeds these threshold
#define MOT_DUR_SELECTED 200 // ms that the motion must continue before the interrupt is raised

#ifdef DEBUG
#define LOG(s) Serial.print(s); Serial.flush();
#else
#define LOG(s)
#endif

//when nothing connected to AD0 then address is 0x68
#define ADO 0
#if ADO
#define MPU6050_ADDRESS     0x69 // Device address when ADO = 1
#else
#define MPU6050_ADDRESS     0x68 // Device address when ADO = 0
#endif
