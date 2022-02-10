#include <PowerConsumption.h>
#include <Shared.h>

PowerConsumption::PowerConsumption() {
};

PowerConsumption::~PowerConsumption() {
};

void PowerConsumption::changeCPUSpeed(CPUSpeed newCPUSpeed) {
    if (currentCPUSpeed == newCPUSpeed) {
        return;
    }
    delay(10);
    CLKPR = 0x80; // enable change in clock frequency
    CLKPR = (uint8_t)newCPUSpeed;
    currentCPUSpeed = newCPUSpeed;
#ifdef DEBUG
    Serial.begin(SERIAL_SPEED * pow(2, (uint8_t)currentCPUSpeed));
    Serial.print("Power mode: ");
    switch (newCPUSpeed) {
    case CPUSpeed::Mhz8:
        Serial.print("LOW");
        break;
    case CPUSpeed::Mhz16:
        Serial.print("HIGH");
        break;
    default:
        Serial.print("ERROR");
    }
    Serial.println(" (" + String((uint8_t)currentCPUSpeed) + ")");
    if (currentCPUSpeed == CPUSpeed::Mhz16) {
    }
    Serial.flush();
#endif
}

void PowerConsumption::high() {
    this->changeCPUSpeed(CPUSpeed::Mhz16);
}

void PowerConsumption::low() {
    this->changeCPUSpeed(CPUSpeed::Mhz8);
}

uint32_t PowerConsumption::toCPUTime(uint32_t timeToAdjust) {
    return (uint32_t)(timeToAdjust / (uint8_t)(pow(2, (uint8_t)currentCPUSpeed)));
}
