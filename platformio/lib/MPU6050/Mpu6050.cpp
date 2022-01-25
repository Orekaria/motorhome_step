#include <Mpu6050.h>
#include <Shared.h>
#include <Wire.h>

Mpu6050::Mpu6050(uint8_t intPin) {
    _intPin = intPin;
    pinMode(_intPin, INPUT_PULLUP); // When you set the mode to INPUT_PULLUP, an internal resistor – inside the Arduino board – will be set between the digital pin 4 and VCC (5V). This resistor – value estimated between 20k and 50k Ohm – will make sure the state stays HIGH. When you press the button, the states becomes LOW
}

Mpu6050::~Mpu6050() {
}

/*    Example for using write byte
      Configure the accelerometer for self-test
      writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0xF0); // Enable self test on all three axes and set accelerometer range to +/- 8 g */
void Mpu6050::writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
    Wire.begin();
    Wire.beginTransmission(address); // Initialize the Tx buffer
    Wire.write(subAddress); // Put slave register address in Tx buffer
    Wire.write(data); // Put data in Tx buffer
    Wire.endTransmission(); // Send the Tx buffer
}

// example showing using readbytev   ----    readByte(MPU6050_ADDRESS, GYRO_CONFIG);
uint8_t Mpu6050::readByte(uint8_t address, uint8_t subAddress) {
    uint8_t data; // `data` will store the register data
    Wire.beginTransmission(address); // Initialize the Tx buffer
    Wire.write(subAddress); // Put slave register address in Tx buffer
    Wire.endTransmission(false); // Send the Tx buffer, but send a restart to keep connection alive
    Wire.requestFrom(address, (uint8_t)1); // Read one byte from slave register address
    data = Wire.read(); // Fill Rx buffer with result
    return data; // Return data read from slave register
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

    writeByte(MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x07); // Reset all internal signal paths in the MPU-6050;
    writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0x01); // Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
    writeByte(MPU6050_ADDRESS, MOT_THR, MOT_THR_SELECTED); // 8-bit unsigned value. Motion is detected when the absolute value of any of the accelerometer measurements exceeds this Motion detection threshold.
    writeByte(MPU6050_ADDRESS, MOT_DUR, MOT_DUR_SELECTED); // The Motion detection interrupt is triggered when the Motion detection counter (depending on MOT_THR) reaches the time count in ms, specified in this register.
    writeByte(MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15); // write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
    writeByte(MPU6050_ADDRESS, INT_PIN_CFG, 0x80); // now INT pin is active low (old = A0)
    writeByte(MPU6050_ADDRESS, INT_ENABLE, 0x40); // bit 6 (0x40), to enable motion detection interrupt.
}
