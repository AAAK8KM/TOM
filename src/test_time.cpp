#include "Time.hpp"
#include <iostream>

int main() {
  Time<Scale::TT> T1(0, 0), T2(1, 0), T3(0, 1);
  auto DT = T2 - T1;
  auto T4 = T3 + DT;
  auto T5 = 2 * DT + T4;
  T1 += DT / 4;
  T3 -= DT;
  DT += DT;
  DT -= DT;

  std::cout << T5 << std::endl;
  return 0;
}
