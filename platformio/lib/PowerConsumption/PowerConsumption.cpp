#include <PowerConsumption.h>
#include <Shared.h>

PowerConsumption::PowerConsumption() {
};

PowerConsumption::~PowerConsumption() {
};

void PowerConsumption::setCPUSpeed(CPUSpeed newCPUSpeed) {
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
    case CPUSpeed::Mhz4:
        Serial.print("4Mhz");
        break;
    case CPUSpeed::Mhz8:
        Serial.print("8Mhz");
        break;
    case CPUSpeed::Mhz16:
        Serial.print("16Mhz");
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
    this->setCPUSpeed(CPUSpeed::Mhz8); // 3.3V => max 8
}

void PowerConsumption::low() {
    this->setCPUSpeed(CPUSpeed::Mhz8); // most of the time the Arduino is sleeping, where the clock is stopped. Reducing speed, to save very little power, at the cost of responsiveness, does not look like a good idea
}

uint32_t PowerConsumption::toCPUTime(uint32_t timeToAdjust) {
    return (uint32_t)(timeToAdjust / (uint8_t)(pow(2, (uint8_t)currentCPUSpeed)));
}
