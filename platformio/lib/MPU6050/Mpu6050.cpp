#include <Mpu6050.h>
#include <Shared.h>
#include <I2Cdev.h>

Mpu6050::Mpu6050(uint8_t intPin) {
    _intPin = intPin;
    pinMode(_intPin, INPUT_PULLUP); // When you set the mode to INPUT_PULLUP, an internal resistor – inside the Arduino board – will be set between the digital pin 4 and VCC (5V). This resistor – value estimated between 20k and 50k Ohm – will make sure the state stays HIGH. When you press the button, the states becomes LOW
}

Mpu6050::~Mpu6050() {
}

void Mpu6050::detectMotionSetup() {
#define SIGNAL_PATH_RESET   0x68
#define INT_PIN_CFG         0x37
#define ACCEL_CONFIG        0x1C
#define MOT_THR             0x1F // Motion detection threshold bits [7:0]
#define MOT_DUR             0x20 // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define INT_ENABLE          0x38
#define INT_STATUS          0x3A
#define MOT_DETECT_CTRL     0x69 // 2-bit unsigned value. Specifies the additional power-on delay in ms applied to accelerometer data path modules.
#define PWR_MGMT            0x6B // sleepy time
#define PWR_GYROS           0x6C // gyros

    // I2Cdev::writeByte(MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x00); // ?
    I2Cdev::writeByte(MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x07); // Reset all internal signal paths in the MPU-6050;
    I2Cdev::writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0x01); // Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
    I2Cdev::writeByte(MPU6050_ADDRESS, MOT_THR, MOT_THR_SELECTED); // 8-bit unsigned value. Motion is detected when the absolute value of any of the accelerometer measurements exceeds this Motion detection threshold.
    I2Cdev::writeByte(MPU6050_ADDRESS, MOT_DUR, MOT_DUR_SELECTED); // The Motion detection interrupt is triggered when the Motion detection counter (depending on MOT_THR) reaches the time count in ms, specified in this register.
    I2Cdev::writeByte(MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15); // write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
    I2Cdev::writeByte(MPU6050_ADDRESS, INT_PIN_CFG, 0x80); // now INT pin is active low (old = A0)
    I2Cdev::writeByte(MPU6050_ADDRESS, INT_ENABLE, 0x40); // bit 6 (0x40), to enable motion detection interrupt.
    // deactivate Temp sensor and Gyros to lower power consumption
    I2Cdev::writeByte(MPU6050_ADDRESS, PWR_MGMT, 8); // 101000 - Cycle & disable TEMP SENSOR
    I2Cdev::writeByte(MPU6050_ADDRESS, PWR_GYROS, 7); // Disable Gyros
}

bool Mpu6050::test() {
    uint8_t  readData;
    // I2Cdev::writeByte(MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15);
    int8_t returnValue = I2Cdev::readByte(MPU6050_ADDRESS, MOT_DETECT_CTRL, (uint8_t*)&readData, 200);
    if (returnValue == -1) {
        LOG("ERROR: I2C - failed to connect to the MPU-6050" + CARRIAGE_RETURN);
        return false;
    }
    LOG("test result: " + String(returnValue) + ", value: " + String(readData, HEX) + CARRIAGE_RETURN);
    return true;
}
