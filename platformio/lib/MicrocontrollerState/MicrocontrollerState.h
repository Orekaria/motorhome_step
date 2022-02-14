#ifndef MicrocontrollerState_h
#define MicrocontrollerState_h

#include <Arduino.h>
#include <LowPower.h>
#include <Shared.h>

class MicrocontrollerState {

public:
    enum class CPUSpeed {
        Mhz16 = 0x00,
        Mhz8 = 0x01,
        Mhz4 = 0x02,
        unknown = 0x99,
    };
    CPUSpeed currentCPUSpeed = CPUSpeed::unknown;
    MicrocontrollerState();
    ~MicrocontrollerState();
    void setCPUSpeed(CPUSpeed newCPUSpeed);
    void high();
    void low();
    uint32_t toCPUTime(uint32_t timeToAdjust);
    void sleep();
private:
};

#endif
