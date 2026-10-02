#ifndef time_converter_hpp__
#define time_converter_hpp__

#include "Time/Time.hpp"
#include "Time/TimeScaleDFS.hpp"
#include <sofa.h>

template <class T, Scale NewS> struct rebind_s;

template <template <Scale> class T, Scale NewS, Scale OldS>
struct rebind_s<T<OldS>, NewS> {
  using type = T<NewS>;
};

template <TimeClass T, Scale NewS>
using rebind = typename rebind_s<T, NewS>::type;

template <class T>
concept TimeDeltaHandler =
    requires(T delta, Time<Scale::UTC> TUTC, Time<Scale::UT1> TUT1,
             Time<Scale::TT> TTT, Time<Scale::TDB> TTDB) {
      { delta.dut(TUTC) } -> std::same_as<double>;
      { delta.dut(TUT1) } -> std::same_as<double>;
      { delta.dttut(TTT) } -> std::same_as<double>;
      { delta.dttut(TUT1) } -> std::same_as<double>;
      { delta.dtdb(TTT) } -> std::same_as<double>;
      { delta.dtdb(TTDB) } -> std::same_as<double>;
    };

template <TimeDeltaHandler D> class Converter {
private:
  template <Scale To, Scale Next, Scale... Rest, template <Scale> class Tc,
            Scale From>
    requires(TimeClass<Tc<From>>)
  Tc<To> convert_chain(const Tc<From> &T) const {
    if constexpr (sizeof...(Rest) == 0)
      return convert<To>(convert<Next>(T));
    else
      return convert<To>(convert_chain<Next, Rest...>(T));
  };

  template <Scale To, Scale From, Scale... Scs, template <Scale> class Tc>
    requires(TimeClass<Tc<From>>)
  Tc<To> convert_seq(const Tc<From> &T, op_seq<Scs...>) const {
    return convert_chain<Scs...>(T);
  };

  template <template <Scale> class Tc, Scale From>
    requires(TimeClass<Tc<From>>)
  class autoconvert_t {
  protected:
    const Tc<From> &TimeIn_;
    const Converter &cv_;

  public:
    autoconvert_t(const Tc<From> &TimeIn, const Converter &cv) noexcept
        : TimeIn_(TimeIn), cv_(cv) {};
    template <Scale To> operator Tc<To>() const noexcept {
      return cv_.template convert<To>(TimeIn_);
    }
    autoconvert_t(const autoconvert_t &) = delete;
    autoconvert_t(autoconvert_t &&) = delete;
    autoconvert_t &operator=(const autoconvert_t &) = delete;
    autoconvert_t &operator=(autoconvert_t &&) = delete;
  };

public:
  D delts_;
  Converter(const D &delts) : delts_(delts) {}
  Converter(D &&delts) : delts_(delts) {}
  template <Scale To, Scale From, template <Scale> class Tc>
    requires(TimeClass<Tc<From>>)
  Tc<To> convert(const Tc<From> &Time) const {
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
      iauUtcut1(Time.jd1(), Time.jd2(), delts_.dut(Time), &jd1, &jd2);

      return {jd1, jd2};
    }

    // UT1 -> UTC
    else if constexpr (To == Scale::UTC && From == Scale::UT1) {
      iauUt1utc(Time.jd1(), Time.jd2(), delts_.dut(Time), &jd1, &jd2);

      return {jd1, jd2};
    }

    // TT -> UT1
    else if constexpr (To == Scale::UT1 && From == Scale::TT) {
      iauTtut1(Time.jd1(), Time.jd2(), delts_.dttut(Time), &jd1, &jd2);

      return {jd1, jd2};
    }

    // UT1 -> TT
    else if constexpr (To == Scale::TT && From == Scale::UT1) {
      iauUt1tt(Time.jd1(), Time.jd2(), delts_.dttut(Time), &jd1, &jd2);

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

    // auto convert
    else if constexpr (To == Scale::TDB && From == Scale::TCB) {
      iauTcbtdb(Time.jd1(), Time.jd2(), &jd1, &jd2);
      return {jd1, jd2};
    } else {
      return convert_seq<To, From>(Time, conv_resolve_op<To, From>());
    }
  }

  template <Scale From, template <Scale> class Tc>
    requires(TimeClass<Tc<From>>)
  autoconvert_t<Tc, From> autoconvert(const Tc<From> &T) const noexcept {
    return autoconvert_t<Tc, From>(T, *this);
  }
};

#endif
