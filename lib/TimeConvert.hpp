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

template <TimeClass T> class convert {
protected:
  T TimeIn_;

public:
  convert(const T &TimeIn) : TimeIn_(TimeIn) {};
  template <Scale Sc> operator rebind<T, Sc>() noexcept {
    // Convert template logic
    return {};
  }
};

#endif
