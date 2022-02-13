#include <PowerConsumption.h>

PowerConsumption::PowerConsumption() {
};

PowerConsumption::~PowerConsumption() {
};

void PowerConsumption::setCPUSpeed(CPUSpeed newCPUSpeed) {
    if (currentCPUSpeed == newCPUSpeed) {
        return;
    }
    delay(10); // some time to end pending tasks
    CLKPR = 0x80; // enable change in clock frequency
    CLKPR = (uint8_t)newCPUSpeed;
    currentCPUSpeed = newCPUSpeed;
#ifdef DEBUG
    Serial.begin(SERIAL_SPEED * pow(2, (uint8_t)currentCPUSpeed));
    LOG("Power mode: ");
    switch (newCPUSpeed) {
    case CPUSpeed::Mhz4:
        LOG("4Mhz");
        break;
    case CPUSpeed::Mhz8:
        LOG("8Mhz");
        break;
    case CPUSpeed::Mhz16:
        LOG("16Mhz");
        break;
    default:
        LOG("ERROR");
    }
    LOG(" (" + String((uint8_t)currentCPUSpeed) + ")" + CARRIAGE_RETURN);
    if (currentCPUSpeed == CPUSpeed::Mhz16) {
    }
    Serial.flush();
#endif
}

void PowerConsumption::high() {
    this->setCPUSpeed(CPUSpeed::Mhz8); // 3.3V => max 8. // at 3.3V, it is recommended to lower the speed of the CPU. timers must be adjusted using the toCPUTime function
}

void PowerConsumption::low() {
    this->setCPUSpeed(CPUSpeed::Mhz8); // most of the time the Arduino is sleeping, where the clock is stopped. Reducing speed, to save very little power, at the cost of responsiveness, does not look like a good idea
}

unsigned long PowerConsumption::toCPUTime(unsigned long timeToAdjust) {
    return (unsigned long)(timeToAdjust / (uint8_t)(pow(2, (uint8_t)currentCPUSpeed)));
}

void PowerConsumption::sleep() {
    LOG("power DOWN" + CARRIAGE_RETURN);
    delay(10); // some time to end pending tasks
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
