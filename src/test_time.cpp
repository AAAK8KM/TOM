#include "Time.hpp"
#include "TimeScaleDFS.hpp"
#include "constants.hpp"
#include <iostream>
#include <ranges>

template <class T> void print_type() {
  std::cout << __PRETTY_FUNCTION__ << '\n';
}

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

  std::cout << conv_resolve_p<Scale::UTC>((uint8_t)Scale::TCB) << '\n';
  print_type<decltype(conv_resolve_op<Scale::UTC, Scale::TCB>())>();
  return 0;
}
