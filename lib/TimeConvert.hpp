#ifndef timeconvert_hpp__
#define timeconvert_hpp__

#include "Time.hpp"
#include "TimeScaleDFS.hpp"

template <class T, Scale NewS> struct rebind_s;

template <template <Scale> class T, Scale NewS, Scale OldS>
struct rebind_s<T<OldS>, NewS> {
  using type = T<NewS>;
};

template <TimeClass T, Scale NewS>
using rebind = typename rebind_s<T, NewS>::type;

template <class T>
concept TimeDeltaHandler =
    requires(T delta, double d, Time<Scale::TT> T1, Time<Scale::TDB> T2) {
      { delta.dut(d) } -> std::same_as<double>;
      { delta.dttut(d) } -> std::same_as<double>;
      { delta.dtdb(T1) } -> std::same_as<double>;
      { delta.dtdb(T2) } -> std::same_as<double>;
    };

template <TimeDeltaHandler D> class Converter {
private:
  template <Scale To, Scale Middle, Scale... MiddleT, Scale From,
            template <Scale> class Tc>
    requires(TimeClass<Tc<From>>)
  Tc<To> convert(const Tc<From> &T) {
    return convert<To>(convert<Middle, MiddleT...>(T));
  };

  template <Scale To, Scale From, Scale... Scs, template <Scale> class Tc>
    requires(TimeClass<Tc<From>>)
  Tc<To> convert(const Tc<From> &T, op_seq<Scs...>) {
    return convert<Scs...>(T);
  };

  template <TimeClass T> class autoconvert {
  protected:
    const T &TimeIn_;
    const Converter &cv;

  public:
    autoconvert(const T &Time) : TimeIn_(Time) {};
    template <Scale Sc> operator rebind<T, Sc>() noexcept {
      return cv.convert<Sc>(TimeIn_);
    }
    autoconvert(const autoconvert &) = delete;
    autoconvert(autoconvert &&) = delete;
    autoconvert &operator=(const autoconvert &) = delete;
    autoconvert &operator=(autoconvert &&) = delete;
  };

public:
  D delts_;
  Converter(const D &delts) : delts_(delts) {}
  Converter(D &&delts) : delts_(delts) {}
  template <Scale To, Scale From, template <Scale> class Tc>
    requires(TimeClass<Tc<From>>)
  Tc<To> convert(const Tc<From> &Time) {
    double jd1, jd2;

    if constexpr (To == From) {
      return {Time.jd1(), Time.jd2()};
    }

    // TT -> TAI
    else if constexpr (To == Scale::TAI && From == Scale::TT) {
      iauTttai(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    }

    // TAI -> TT
    else if constexpr (To == Scale::TT && From == Scale::TAI) {
      iauTaitt(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    }

    // UTC -> TAI
    else if constexpr (To == Scale::TAI && From == Scale::UTC) {
      iauUtctai(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    }

    // TAI -> UTC
    else if constexpr (To == Scale::UTC && From == Scale::TAI) {
      iauTaiutc(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    }

    // UTC -> UT1
    else if constexpr (To == Scale::UT1 && From == Scale::UTC) {
      iauUtcut1(Time.jd1(), Time.jd2(), delts_.dut(Time.jd1), &jd1, &jd2);

      return {jd1, jd2};
    }

    // UT1 -> UTC
    else if constexpr (To == Scale::UTC && From == Scale::UT1) {
      iauUt1utc(Time.jd1(), Time.jd2(), delts_.dut(Time.jd1), &jd1, &jd2);

      return {jd1, jd2};
    }

    // TT -> UT1
    else if constexpr (To == Scale::UT1 && From == Scale::TT) {
      iauTtut1(Time.jd1(), Time.jd2(), delts_.dttut(Time.jd1), &jd1, &jd2);

      return {jd1, jd2};
    }

    // UT1 -> TT
    else if constexpr (To == Scale::TT && From == Scale::UT1) {
      iauUt1tt(Time.jd1(), Time.jd2(), delts_.dttut(Time.jd1), &jd1, &jd2);

      return {jd1, jd2};
    }

    // TT -> TCG
    else if constexpr (To == Scale::TCG && From == Scale::TT) {
      iauTttcg(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    }

    // TCG -> TT
    else if constexpr (To == Scale::TT && From == Scale::TCG) {
      iauTcgtt(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    }

    // TT -> TDB
    else if constexpr (To == Scale::TDB && From == Scale::TT) {
      iauTttdb(Time.jd1(), Time.jd2(), delts_.dtdb(Time), &jd1, &jd2);

      return {jd1, jd2};
    }

    // TDB -> TT
    else if constexpr (To == Scale::TT && From == Scale::TDB) {
      iauTdbtt(Time.jd1(), Time.jd2(), delts_.dtdb(Time), &jd1, &jd2);

      return {jd1, jd2};
    }

    // TDB -> TCB
    else if constexpr (To == Scale::TCB && From == Scale::TDB) {
      iauTdbtcb(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    }

    // TCB -> TDB
    else if constexpr (To == Scale::TDB && From == Scale::TCB) {
      iauTcbtdb(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    } else {
      return convert<To, From>(Time, conv_resolve_op<To, From>());
    }
  }
};

#endif
