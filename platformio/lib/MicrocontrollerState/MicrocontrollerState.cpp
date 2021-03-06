#include <MicrocontrollerState.h>

MicrocontrollerState::MicrocontrollerState() {
};

MicrocontrollerState::~MicrocontrollerState() {
};

void MicrocontrollerState::setCPUSpeed(CPUSpeed newCPUSpeed) {
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

void MicrocontrollerState::high() {
    this->setCPUSpeed(CPUSpeed::Mhz8); // 3.3V => max 8. // at 3.3V, it is recommended to lower the speed of the CPU. timers must be adjusted using the toCPUTime function
}

void MicrocontrollerState::low() {
    this->setCPUSpeed(CPUSpeed::Mhz8); // most of the time the Arduino is sleeping, where the clock is stopped. Reducing speed, to save very little power, at the cost of responsiveness, does not look like a good idea
}

uint32_t MicrocontrollerState::toCPUTime(uint32_t timeToAdjust) {
    return (uint32_t)(timeToAdjust / (uint8_t)(pow(2, (uint8_t)currentCPUSpeed)));
}

void MicrocontrollerState::sleep() {
    LOG("power DOWN" + CARRIAGE_RETURN);
    delay(10); // some time to end pending tasks
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
