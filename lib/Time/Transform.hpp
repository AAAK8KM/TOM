#ifndef time_transform_hpp__
#define time_transform_hpp__

#include "EOP/EOP.hpp"
#include "Time/Time.hpp"
#include "constants.hpp"

template <EOPC DC> class EOPTimeDelta {
private:
  const DC &eop_;
  // TT - UTC, seconds, outside a leap second: TT - TAI plus TAI - UTC.
  static constexpr double defaultDttutc = 32.184 + 37.0;

public:
  EOPTimeDelta(const DC &eop) : eop_(eop) {}

  // UT1 - UTC, seconds. EOP is tabulated against MJD in UTC.
  template <TimeClass T>
    requires(same_scale<T, Scale::UTC>)
  double dut(const T &time) const noexcept {
    return eop_.dut(time.mjd());
  }

  // UT1 - UTC, seconds, for a time already in UT1. The table is indexed by
  // UTC, so the UTC of the same instant is found by iteration; dut moves by
  // under a second, so a handful of passes settle it.
  template <TimeClass T>
    requires(same_scale<T, Scale::UT1>)
  double dut(const T &time) const noexcept {
    const double ut1 = time.mjd();
    double utc = ut1;
    for (int i = 0; i < 10; i++)
      utc = ut1 - eop_.dut(utc) / secondsInDay;
    return eop_.dut(utc);
  }

  // TT - UT1, seconds. TT - UT1 = (TT - UTC) - (UT1 - UTC), and the UTC of
  // the instant is TT shifted back by TT - UTC.
  template <TimeClass T>
    requires(same_scale<T, Scale::TT>)
  double dttut(const T &time) const noexcept {
    const double utc = time.mjd() - defaultDttutc / secondsInDay;
    return defaultDttutc - eop_.dut(utc);
  }

  template <TimeClass T>
    requires(same_scale<T, Scale::UT1>)
  double dttut(const T &time) const noexcept {
    return defaultDttutc - dut(time);
  }

  template <TimeClass T>
    requires(same_scale<T, Scale::TT> || same_scale<T, Scale::TDB>)
  double dtdb(const T &) const noexcept {
    return 0;
  }
};

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
