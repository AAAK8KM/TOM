#ifndef eop_hpp__
#define eop_hpp__

#include <cmath>

struct DailyEOP {
  double mjd;
  double dut;
  double X;
  double Y;
};

struct CIPP {
  double X;
  double Y;
};

template <template <typename> class V>
concept EOPcontainer = requires(std::size_t d, V<DailyEOP> ctr) {
  { ctr[d] } -> std::same_as<DailyEOP>;
};

template <template <typename> class EOPC>
  requires EOPcontainer<EOPC>
class EOP {
  EOPC<DailyEOP> data_;
  double mjd0;

  std::pair<std::size_t, double> get_index(const double mjd) const {
    double d = std::floor(mjd - mjd0);
    const double frac = mjd - mjd0 - (double)d;
    return {(std::size_t)d, frac};
  };

public:
  // in UTC
  double dut(const double mjd) const {
    const auto [d, frac] = get_index(mjd);
    if (d + 1 >= data_.size() || mjd < mjd0)
      return 0;
    const double shift =
        (data_[d + 1].dut > data_[d].dut + 0.7
             ? -1
             : (data_[d + 1].dut < data_[d].dut - 0.7 ? 1 : 0));
    return std::lerp(data_[d].dut, data_[d + 1].dut + shift, frac);
  }

  // in UTC
  CIPP pole(double mjd) const {
    const auto [d, frac] = get_index(mjd);
    if (d + 1 >= data_.size() || mjd < mjd0)
      return {0, 0};
    return {std::lerp(data_[d].X, data_[d + 1].X, frac),
            std::lerp(data_[d].Y, data_[d + 1].Y, frac)};
  };
};

template <typename T>
concept EOPC = requires {
  []<template <typename> class CNTR>(const EOP<CNTR> &) {}(std::declval<T>());
};

#endif
