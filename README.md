# Replacement for the Step circuit in a vehicle

## Instructions
- Create the circuit
- Upload the code to the Arduino

### How to use
- Open button: to extend the step
  - the step will auto-close after 11 seconds
  - if the Open button is pushed again within 2 seconds after the step has extended, the auto-close is disabled
- Close button: to retract the step
- If the vehicle starts moving, an alarm will sound and the step will auto-close
- The step can be controlled by the user and the arduino indistinctly, thanks to the 2x relay configuration

## Bill of materials
- Arduino Nano 3
- 1x switching regulator at 4.1V
- 2x relays with NC/NO built-in connectors
- 1x MPU-6050 accelerometer to detect if the vehicle is moving
- 1x Active buzzer to alarm when the vehicle is moving and the step is being auto-closed
- 1x 10k resistors for the interrupt
- 3x 1N4007 diodes, 1x to block the current from reaching the regulator when connected to USB, 2x diodes to join the buttons 1 interrupt
- 1x 4700uF capacitor to prevent power drops

## Low power usage
  - Voltage 3.2V
  - The Arduino speed has been reduced from 16Mhz to 8Mhz. 3.3V requires 8Mhz. Also it consumes less power
  - The Arduino is put in deep sleep mode waiting for an interrupt to be fired
  - Arduino's pins are all set as INPUT/OUTPUT, because INPUT-PULLUP increases current consumption by 50-80nA per pin
  - Interrupts can be fired by both buttons and by the motion detector
  - The motion detector is powered via an Arduino GPIO so it can be switched off when not needed (step closed)
  - High efficiency switch regulator
  - Power led and regulator have been removed from the Arduino. Power led removed from the MPU

### How it lowers power usage:
Arduino is in sleep until an interrupt is fired (button pressed or motion detected)
After the step has been opened/closed, it goes back to sleep
MPU-6050 is powered up only when the step is permanently opened using the internal DMP (Digital Motion Procesor), which does not require the Arduino to poll data and which fires an interruption when it detects motion. Sensibility can be adjusted in the Shared.h file

## possible alternatives/improvements
- Use a SW-420 motion sensor instead of the MPU-6050
- Use an Arduino Pro Micro
- Use an Arduino Nano 33 IoT
- Use an ESP32 based board
- Use a MOSFET to power off the motion detector instead of an Arduino GPIO

## Breadboard prototype
![Schematic](images/breadboard_view1.jpg)
![Schematic](images/breadboard_view2.jpg)

## Links

I2Cdev: https://www.i2cdevlib.com/usage
Relay, NPN transistor: https://www.inventable.eu/controlar-rele-con-transistor/
MPU-6050:
  - DMP vs complementary filter: https://www.youtube.com/watch?v=2t-5CCyPJ74&ab_channel=geekmomprojects
  - Promising library: https://github.com/rambo/I2C. I would like to replace I2Cdev and FastWire with it, if i could find more information and would have a lot more energy to spend in this topic
