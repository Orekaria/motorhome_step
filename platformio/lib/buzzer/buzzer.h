#ifndef Buzzer_h
#define Buzzer_h

#include <Arduino.h>
#include <Shared.h>

enum BuzzerType {
   ACTIVE,
   PASSIVE,
};

class Buzzer {
private:
public:
   Buzzer();
   Buzzer(uint8_t intPin, BuzzerType buzzerType, uint16_t frequency);
   ~Buzzer();
   void beep(uint16_t beepTime);
};

#endif
