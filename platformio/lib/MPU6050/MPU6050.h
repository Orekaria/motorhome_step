#ifndef Mpu6050_h
#define Mpu6050_h

#include <Arduino.h>
#include <Shared.h>
#include <I2Cdev.h>
#include <Simple_MPU6050.h>

enum MotionDetectionState {
    OFF,
    ON,
};

class Mpu6050 {
private:
    Simple_MPU6050 mpu; // for some reason that I cannot understand now, this object must be instantiated at the begining. If not, the MPU fails to calibrate
    uint8_t _intPin;
    uint8_t _onOffPin;
    bool _isSettingUpMotionDetector = false;
    void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);
    uint8_t readByte(uint8_t address, uint8_t subAddress);
    uint32_t toCPUTime(uint32_t t);
public:
    Mpu6050(uint8_t intPin, uint8_t onOffPin);
    ~Mpu6050();
    void detectMotionSetup();
    void activateMotionDetector();
    bool test();
    static void motionDetected();
    bool isMotionDetected();
    void motionDetection(MotionDetectionState onOrOff);
};

#endif
