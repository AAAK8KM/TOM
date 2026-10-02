#ifndef deltahandler_hpp__
#define deltahandler_hpp__

#include "EOP.hpp"
#include "Time.hpp"
#include "constants.hpp"

template <EOPC DC> class EOPTimeDelta {
private:
  const DC &eop_;
  static constexpr double defaultDttutc =
      (32.184 + 37.0) / secondsInDay; // TT - TAI + TAI - UTC

public:
  EOPTimeDelta(const DC &eop) : eop_(eop) {}

  template <TimeClass T>
    requires(same_scale<T, Scale::UTC>)
  double dut(const T &time) const noexcept {
    return eop_.dut(time.jd());
  }

  template <TimeClass T>
    requires(same_scale<T, Scale::TT>)
  double dttut(const T &time) const noexcept {
    return eop_.dut(time.jd() - defaultDttutc) + defaultDttutc;
  }

  template <TimeClass T>
    requires(same_scale<T, Scale::UT1>)
  double dut(const T &time) const noexcept {
    T temp = time;
    for (int i = 0; i < 10; i++)
      temp = time - eop_.dut(temp.jd());
    return eop_.dut(temp.jd());
  }

  template <TimeClass T>
    requires(same_scale<T, Scale::UT1>)
  double dttut(const T &time) const noexcept {
    return dut(time) + defaultDttutc;
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
