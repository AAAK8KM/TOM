#ifndef timeconvert_hpp__
#define timeconvert_hpp__

#include "Time.hpp"

template <class T, Scale NewS> struct rebind_s;

template <template <Scale> class T, Scale NewS, Scale OldS>
struct rebind_s<T<OldS>, NewS> {
  using type = T<NewS>;
};

template <TimeClass T, Scale NewS>
using rebind = typename rebind_s<T, NewS>::type;

template <Scale Sc, TimeClass T> rebind<T, Sc> convert(const T &Time) noexcept;

template <TimeClass T> class autoconvert {
protected:
  T TimeIn_;

public:
  autoconvert(const T &Time) : TimeIn_(Time) {};
  template <Scale Sc> operator rebind<T, Sc>() noexcept {
    return convert<Sc>(TimeIn_);
  }
  autoconvert(const autoconvert &) = delete;
  autoconvert(autoconvert &&) = delete;
  autoconvert &operator=(const autoconvert &) = delete;
  autoconvert &operator=(autoconvert &&) = delete;
};

#endif
