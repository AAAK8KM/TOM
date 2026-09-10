#ifndef time_hpp__
#define time_hpp__

#include "constants.hpp"
#include <cmath>
#include <ostream>

enum class Scale { TT, TAI, UTC, UT1, TCG, TCB, TDB };

template <class TimeT, Scale Sc> class TimeBase;

template <class T>
concept TimeClass = requires {
  []<class U, Scale Sc>(const TimeBase<U, Sc> &) {}(std::declval<const T &>());
};

template <class T, Scale Sc>
concept same_scale = TimeClass<T> && (T::scale == Sc);

template <typename TimeT, Scale Sc>
class TimeBase // base time behaviour
{
public:
  static constexpr Scale scale = Sc;

protected:
  double jd1_;
  double jd2_;

  TimeBase(double jd1 = 0, double jd2 = 0) noexcept : jd1_(jd1), jd2_(jd2) {
    this->normalize();
  }
  template <TimeClass RT>
  TimeBase(const TimeBase<RT, Sc> &T) noexcept : jd1_(T.jd1()), jd2_(T.jd2()) {}

  void normalize() noexcept {
    double tmp;
    jd2_ += std::modf(jd1_, &tmp);
    jd1_ = tmp;
    jd2_ = std::modf(jd2_, &tmp);
    jd1_ += tmp;
  }

  template <TimeClass RT>
    requires(same_scale<RT, Sc>)
  TimeBase &operator+=(const RT &T) noexcept {
    this->jd1_ += T.jd1();
    this->jd2_ += T.jd2();
    this->normalize();
    return *this;
  }

  template <TimeClass RT>
    requires(same_scale<RT, Sc>)
  TimeBase &operator-=(const RT &T) noexcept {
    this->jd1_ -= T.jd1();
    this->jd2_ -= T.jd2();
    this->normalize();
    return *this;
  }

  TimeBase &operator+=(const double &seconds) noexcept {
    this->jd2_ += seconds / secondsInDay;
    this->normalize();
    return *this;
  }

  TimeBase &operator-=(const double &seconds) noexcept {
    this->jd2_ -= seconds / secondsInDay;
    this->normalize();
    return *this;
  }

public:
  double jd1() const noexcept { return jd1_; };
  double jd2() const noexcept { return jd2_; };

  auto operator<=>(const TimeBase &rhs) const noexcept = default;

  friend std::ostream &operator<<(std::ostream &os, const TimeBase &T) {
    os << "S:" << (int)scale << "jd:" << T.jd2() + T.jd1();
    return os;
  }
};

template <Scale Sc> class Time;

template <Scale Sc> class TimeDelta : public TimeBase<TimeDelta<Sc>, Sc> {
public:
  TimeDelta(double jd1 = 0, double jd2 = 0) noexcept
      : TimeBase<TimeDelta, Sc>(jd1, jd2) {}
  template <TimeClass RT>
    requires(same_scale<RT, Sc>)
  TimeDelta(const TimeBase<RT, Sc> &T) noexcept : TimeBase<TimeDelta, Sc>(T) {}

  // template <Scale S2> TimeDelta operator+=(const Time<S2> &T) = delete;
  // template <Scale S2> TimeDelta operator-=(const Time<S2> &T) = delete;

  template <TimeClass RT>
  friend RT operator+(const RT &lhs, const TimeDelta &rhs) noexcept {
    RT res(lhs);
    res += rhs;
    return res;
  }

  template <TimeClass RT>
    requires(same_scale<RT, Sc>)
  RT operator+(const RT &rhs) const noexcept
    requires(!std::same_as<RT, TimeDelta>)
  {
    return rhs + (*this);
  }

  template <TimeClass RT>
    requires(same_scale<RT, Sc>)
  friend RT operator-(const RT &lhs, const TimeDelta &rhs) noexcept {
    RT res(lhs);
    res -= rhs;
    return res;
  }

  TimeDelta &operator+=(const TimeDelta &D) noexcept {
    TimeBase<TimeDelta, Sc>::operator+=(D);
    return *this;
  }

  TimeDelta &operator-=(const TimeDelta &D) noexcept {
    TimeBase<TimeDelta, Sc>::operator-=(D);
    return *this;
  }

  TimeDelta &operator*=(const double &d) noexcept {
    this->jd1_ *= d;
    this->jd2_ *= d;
    this->normalize();
    return *this;
  }

  TimeDelta &operator/=(const double &d) noexcept {
    this->jd1_ /= d;
    this->jd2_ /= d;
    this->normalize();
    return *this;
  }

  TimeDelta operator*(const double &d) const noexcept {
    TimeDelta res(*this);
    res *= d;
    return res;
  }
  friend TimeDelta operator*(const double &d, const TimeDelta &D) noexcept {
    return D * d;
  }

  TimeDelta operator/(const double &d) const noexcept {
    TimeDelta res(*this);
    res /= d;
    return res;
  }
  friend TimeDelta operator/(const double &d, const TimeDelta &D) noexcept {
    return D / d;
  }
};

template <Scale Sc> class Time : public TimeBase<Time<Sc>, Sc> {
public:
  Time(double jd1 = 0, double jd2 = 0) noexcept
      : TimeBase<Time<Sc>, Sc>(jd1, jd2) {}

  Time &operator+=(const TimeDelta<Sc> &D) noexcept {
    TimeBase<Time, Sc>::operator+=(D);
    return *this;
  }

  Time &operator-=(const TimeDelta<Sc> &D) noexcept {
    TimeBase<Time, Sc>::operator-=(D);
    return *this;
  }

  Time operator-(const TimeDelta<Sc> &rhs) const noexcept {
    Time res(*this);
    res -= rhs;
    return res;
  }

  auto operator-(const Time &rhs) const noexcept {
    TimeDelta<Sc> res(*this);
    res -= TimeDelta<Sc>(rhs);
    return res;
  }

  Time operator+(const TimeDelta<Sc> &rhs) const noexcept {
    Time res(*this);
    res += rhs;
    return res;
  }
};

#endif
