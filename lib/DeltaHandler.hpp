#ifndef deltahandler_hpp__
#define deltahandler_hpp__

#include "Time.hpp"
#include "constants.hpp"

class DummyDelta {
private:
  double dut_;   // UT1 - UTC, seconds
  double dttut_; // TT - UT1, seconds
  double dtdb_;  // TDB - TT, seconds

public:
  static constexpr double defaultDut = 0.0;
  static constexpr double defaultDttut = 32.184 + 37.0; // TT - TAI + TAI - UTC
  static constexpr double defaultDtdb = 0.0;

  DummyDelta(double dut = defaultDut, double dttut = defaultDttut,
             double dtdb = defaultDtdb) noexcept
      : dut_(dut), dttut_(dttut), dtdb_(dtdb) {}

  // UT1 - UTC, argument is UTC or UT1
  template <TimeClass T>
    requires(same_scale<T, Scale::UTC> || same_scale<T, Scale::UT1>)
  double dut(const T &) const noexcept {
    return dut_;
  }

  // TT - UT1, argument is TT or UT1
  template <TimeClass T>
    requires(same_scale<T, Scale::TT> || same_scale<T, Scale::UT1>)
  double dttut(const T &) const noexcept {
    return dttut_;
  }

  // TDB - TT, argument is TT or TDB
  template <TimeClass T>
    requires(same_scale<T, Scale::TT> || same_scale<T, Scale::TDB>)
  double dtdb(const T &) const noexcept {
    return dtdb_;
  }

  void set_dut(double dut) noexcept { dut_ = dut; }
  void set_dttut(double dttut) noexcept { dttut_ = dttut; }
  void set_dtdb(double dtdb) noexcept { dtdb_ = dtdb; }
};

#endif
