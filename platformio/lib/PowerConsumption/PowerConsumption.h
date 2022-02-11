#include <Arduino.h>

#ifndef POWERCONSUMPTION_H
#define POWERCONSUMPTION_H

class PowerConsumption {

public:
    enum class CPUSpeed {
        Mhz16 = 0x00,
        Mhz8 = 0x01,
        unknown = 0x99,
    };
    CPUSpeed currentCPUSpeed = CPUSpeed::unknown;
    PowerConsumption();
    ~PowerConsumption();
    void setCPUSpeed(CPUSpeed newCPUSpeed);
    void high();
    void low();
    uint32_t toCPUTime(uint32_t timeToAdjust);
private:
};

#endif
