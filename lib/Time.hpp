#ifndef time_hpp__
#define time_hpp__

#include "constants.hpp"
#include <cmath>

enum class Scale { TT, TAI, UTC, UT1, TCG, TCB, TDB };

template <class TimeT, Scale S> class TimeBase;

template <class T>
concept TimeClass = requires {
  []<class U, Scale S>(const TimeBase<U, S> &) {}(std::declval<const T &>());
};

template <class T, Scale S>
concept same_scale = TimeClass<T> && (T::scale == S);

template <typename TimeT, Scale S>
class TimeBase // base time behaviour
{
public:
  static constexpr Scale scale = S;

protected:
  double jdInt_;
  double jdFrac_;

  TimeBase(double jdInt = 0, double jdFrac = 0) noexcept
      : jdInt_(jdInt), jdFrac_(jdFrac) {
    this->normalize();
  }
  template <TimeClass RT>
  TimeBase(const TimeBase<RT, S> &T) noexcept
      : jdInt_(T.jdInt()), jdFrac_(T.jdFrac()) {}

  void normalize() noexcept {
    double tmp;
    jdFrac_ += std::modf(jdInt_, &tmp);
    jdInt_ = tmp;
    jdFrac_ = std::modf(jdFrac_, &tmp);
    jdInt_ += tmp;
  }

  template <TimeClass RT>
    requires(same_scale<RT, S>)
  TimeBase &operator+=(const RT &T) noexcept {
    this->jdInt_ += T.jdInt();
    this->jdFrac_ += T.jdFrac();
    this->normalize();
    return *this;
  }

  template <TimeClass RT>
    requires(same_scale<RT, S>)
  TimeBase &operator-=(const RT &T) noexcept {
    this->jdInt_ -= T.jdInt();
    this->jdFrac_ -= T.jdFrac();
    this->normalize();
    return *this;
  }

  TimeBase &operator+=(const double &seconds) noexcept {
    this->jdFrac_ += seconds / secondsInDay;
    this->normalize();
    return *this;
  }

  TimeBase &operator-=(const double &seconds) noexcept {
    this->jdFrac_ -= seconds / secondsInDay;
    this->normalize();
    return *this;
  }

public:
  double jdInt() const noexcept { return jdInt_; };
  double jdFrac() const noexcept { return jdFrac_; };

  auto operator<=>(const TimeBase &rhs) const noexcept = default;
};

template <Scale S> class Time;

template <Scale S> class TimeDelta : public TimeBase<TimeDelta<S>, S> {
public:
  TimeDelta(double jdInt = 0, double jdFrac = 0) noexcept
      : TimeBase<TimeDelta, S>(jdInt, jdFrac) {}
  template <TimeClass RT>
    requires(same_scale<RT, S>)
  TimeDelta(const TimeBase<RT, S> &T) noexcept : TimeBase<TimeDelta, S>(T) {}

  // template <Scale S2> TimeDelta operator+=(const Time<S2> &T) = delete;
  // template <Scale S2> TimeDelta operator-=(const Time<S2> &T) = delete;

  template <TimeClass RT>
  friend RT operator+(const RT &lhs, const TimeDelta &rhs) noexcept {
    RT res(lhs);
    res += rhs;
    return res;
  }

  template <TimeClass RT>
    requires(same_scale<RT, S>)
  friend RT operator+(const TimeDelta &lhs, const RT &rhs) noexcept
    requires(!std::same_as<RT, TimeDelta>)
  {
    return rhs + lhs;
  }

  template <TimeClass RT>
    requires(same_scale<RT, S>)
  friend RT operator-(const RT &lhs, const TimeDelta &rhs) noexcept {
    RT res(lhs);
    res -= rhs;
    return res;
  }

  TimeDelta &operator+=(const TimeDelta &D) noexcept {
    TimeBase<TimeDelta, S>::operator+=(D);
    return *this;
  }

  TimeDelta &operator-=(const TimeDelta &D) noexcept {
    TimeBase<TimeDelta, S>::operator-=(D);
    return *this;
  }

  TimeDelta &operator*=(const double &d) noexcept {
    this->jdInt_ *= d;
    this->jdFrac_ *= d;
    this->normalize();
    return *this;
  }

  TimeDelta &operator/=(const double &d) noexcept {
    this->jdInt_ /= d;
    this->jdFrac_ /= d;
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

template <Scale S> class Time : public TimeBase<Time<S>, S> {
public:
  Time(double jdInt = 0, double jdFrac = 0) noexcept
      : TimeBase<Time<S>, S>(jdInt, jdFrac) {}

  Time &operator+=(const TimeDelta<S> &D) noexcept {
    TimeBase<Time, S>::operator+=(D);
    return *this;
  }

  Time &operator-=(const TimeDelta<S> &D) noexcept {
    TimeBase<Time, S>::operator-=(D);
    return *this;
  }

  Time operator-(const TimeDelta<S> &rhs) const noexcept {
    Time res(*this);
    res -= rhs;
    return res;
  }

  auto operator-(const Time &rhs) const noexcept {
    TimeDelta<S> res(*this);
    res -= TimeDelta<S>(rhs);
    return res;
  }

  Time operator+(const TimeDelta<S> &rhs) const noexcept {
    Time res(*this);
    res += rhs;
    return res;
  }
};

#endif
