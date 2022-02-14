#ifndef PowerConsumption_h
#define PowerConsumption_h

#include <Arduino.h>
#include <LowPower.h>
#include <Shared.h>

class PowerConsumption {

public:
    enum class CPUSpeed {
        Mhz16 = 0x00,
        Mhz8 = 0x01,
        Mhz4 = 0x02,
        unknown = 0x99,
    };
    CPUSpeed currentCPUSpeed = CPUSpeed::unknown;
    PowerConsumption();
    ~PowerConsumption();
    void setCPUSpeed(CPUSpeed newCPUSpeed);
    void high();
    void low();
    uint32_t toCPUTime(uint32_t timeToAdjust);
    void sleep();
private:
};

#endif
