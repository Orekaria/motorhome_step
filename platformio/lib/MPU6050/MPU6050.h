#include <Arduino.h>
#include <Shared.h>
#include <Wire.h>

#ifndef MPU6050_H
#define MPU6050_H

class Mpu6050 {
private:
    uint8_t _intPin;
    void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);
    uint8_t readByte(uint8_t address, uint8_t subAddress);
public:
    Mpu6050(uint8_t intPin);
    ~Mpu6050();
    void detectMotionSetup();
    bool test();
};

#endif
