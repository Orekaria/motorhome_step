#include <Buzzer.h>

static uint8_t _buzzerPin = -1;
static BuzzerType _buzzerType;
static uint16_t _buzzerFrequency;

Buzzer::Buzzer() {
   if (_buzzerPin == -1) {
      LOG("Error: Buzzer class must be initialized with all the parameters before calling this method" + CARRIAGE_RETURN);
   }
}

Buzzer::Buzzer(uint8_t buzzerPin, BuzzerType buzzerType, uint16_t frequency) {
   _buzzerPin = buzzerPin;
   _buzzerType = buzzerType;
   _buzzerFrequency = frequency;

   pinMode(_buzzerPin, OUTPUT);
}

Buzzer::~Buzzer() {

}

void Buzzer::beep(uint16_t beepTime) {
   switch (_buzzerType) {
   case BuzzerType::ACTIVE:
      digitalWrite(_buzzerPin, HIGH);
      delay(beepTime);
      digitalWrite(_buzzerPin, LOW);
      break;
   case BuzzerType::PASSIVE:
      tone(_buzzerPin, _buzzerFrequency);
      delay(beepTime);
      noTone(_buzzerPin);
      break;
   default:
      LOG("Error: unknown buzzerType (" + String(_buzzerType) + ")" + CARRIAGE_RETURN);
      break;
   }
}
