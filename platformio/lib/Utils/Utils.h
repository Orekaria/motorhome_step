#include <Shared.h>
#include <Arduino.h>

#ifndef UTILS_H
#define UTILS_H

class Utils {
private:
public:
    Utils();
    ~Utils();
    static float roundToDot5(float f);
#if DEBUG == true
    static void log(String s);
#endif
};

#endif
