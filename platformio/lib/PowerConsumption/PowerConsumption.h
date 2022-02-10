#include <Arduino.h>

#ifndef POWERCONSUMPTION_H
#define POWERCONSUMPTION_H

class PowerConsumption {
    enum class CPUSpeed {
        Mhz16 = 0x00,
        Mhz8 = 0x01,
        unknown = 0x99,
    };

private:
    void changeCPUSpeed(CPUSpeed newCPUSpeed);
public:
    CPUSpeed currentCPUSpeed = CPUSpeed::unknown;
    PowerConsumption();
    ~PowerConsumption();
    void high();
    void low();
    uint32_t toCPUTime(uint32_t timeToAdjust);
};

#endif
