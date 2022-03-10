// global variables

#define DEBUG // uncomment to activate DEBUG mode

#define SERIAL_SPEED (uint32_t) 9600 // Serial monitor speed. Set the same value in platformio.ini

#define CARRIAGE_RETURN (String) "\n\r"

#define INTERRUPT_BUTTONS_PIN 3 // Arduino nano pin 2 and 3 can be selected
#define INTERRUPT_MPU6050_PIN 2 // pin used for waking up when MPU6050 detects motion
#define BUZZER_PIN 12           // pin for the buzzer
#define BUZZER_FREQUENCY 2000

#define MOT_CALIBRATE false               // calibration true will calibrate the MPU when powered on but will take some seconds
#define MOT_ENABLE_GYROS false            // activate gyros for enhanced motion detection but it will increase the power consumption by 1.7mA!
// 6, 75  se autoretracta a veces cuando se cierra la puerta o se enciende la nevera
// 5, 150 no se cierra cuando me pongo en marcha, hasta que no hay algunos baches
#define MOT_THR_SELECTED (uint8_t) 4      // motion is detected when any of the accelerometer messures exceeds these threshold
#define MOT_DUR_SELECTED (uint8_t) 100    // ms that the motion must continue before the interrupt is raised
#define MOT_MIN_INTERVAL (uint16_t) 5000  // in ms, motion has to be detected twice in this period to be counted. filters motions which source is inside the vehicle, e.g. when the frigde turns on or when the door is closed

#ifdef DEBUG
#define LOG(s) Serial.print(s); Serial.flush();
#else
#define LOG(s)
#endif

// when nothing is connected to AD0, the address is 0x68
#define ADO 0
#if ADO
#define MPU6050_ADDRESS     0x69 // Device address when ADO = 1
#else
#define MPU6050_ADDRESS     0x68 // Device address when ADO = 0
#endif
