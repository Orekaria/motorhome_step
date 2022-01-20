#include <utils.h>

Utils::Utils(/* args */)
{
}

Utils::~Utils()
{
}

float Utils::roundToDot5(float f) {
  const static signed char ROUND_5_DELTA[5] = { 0, -1, -2, 2, 1 };  // difference to the "rounded to nearest 5" value
  int a = (int)truncf(f * 10.0);
  // a = a + (ROUND_5_DELTA[abs(a) % 5] * (a >= 0 ? 1 : -1));
  if (a >= 0) {
    a = a + ROUND_5_DELTA[a % 5];
  } else {
    a = a - ROUND_5_DELTA[abs(a) % 5];
  }
  return a / 10.0;
}

// float roundToDot5(float f) {
//   // Round to .5; For positive numbers Multiply by 2, add 0.5, truncate, divide by 2.
//   long a = round(f * 10);
//   return round(a >= 0 ? (a + 2) / 5 * 5 : (a - 2) / 5 * 5) / 10;
//   // const static signed char round5delta[5] = { 0, -1, -2, 2, 1 };  // difference to the "rounded to nearest 5" value
//   // return a + round5delta[a % 5];
// }

#if DEBUG == true
void Utils::log(String msg) {
  Serial.print(msg);
}
#endif
