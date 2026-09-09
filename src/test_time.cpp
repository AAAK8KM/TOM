#include "Time.hpp"

int main() {
  Time<Scale::TT> T1(0, 0), T2(1, 0), T3(0, 1);
  auto DT = T2 - T1;
  auto T4 = T3 + DT;
  return 0;
}
