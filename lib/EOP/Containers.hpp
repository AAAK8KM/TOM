#ifndef eopstandart_hpp__
#define eopstandart_hpp__

#include "EOP.hpp"

#include <array>
#include <cstddef>
#include <utility>
#include <vector>

// Ready made EOPcontainer implementations, so a caller does not have to write
// one to use EOP. Both model the same thing: a run of daily samples whose
// slot i stands for MJD mjd0() + i, which is the mapping EOP indexes with.
//
// EOPVector sizes at run time, for a table whose span is known only once the
// data arrives. EOPArray sizes at compile time, for a fixed window with no
// allocation.

class EOPVector {
  std::vector<DailyEOP> data_;
  double mjd0_ = 0;

public:
  EOPVector() = default;

  // Window of days days, starting at MJD mjd0, every slot zeroed.
  EOPVector(double mjd0, std::size_t days) : data_(days), mjd0_(mjd0) {}

  // Takes the samples as they are; mjd0 is the MJD of data[0].
  EOPVector(std::vector<DailyEOP> data, double mjd0)
      : data_(std::move(data)), mjd0_(mjd0) {}

  DailyEOP &operator[](std::size_t i) { return data_[i]; }
  const DailyEOP &operator[](std::size_t i) const { return data_[i]; }

  std::size_t size() const noexcept { return data_.size(); }
  double mjd0() const noexcept { return mjd0_; }

  bool empty() const noexcept { return data_.empty(); }
  auto begin() const noexcept { return data_.begin(); }
  auto end() const noexcept { return data_.end(); }

  // Moves the window. The samples already held are not touched, so this is
  // for setting the start before a fill, not for shifting data.
  void set_mjd0(double mjd0) noexcept { mjd0_ = mjd0; }

  void resize(std::size_t days) { data_.resize(days); }

  // Appends one day. The caller keeps the one day step: a sample whose mjd is
  // not mjd0() + size() breaks the slot mapping EOP relies on.
  void push_back(const DailyEOP &day) { data_.push_back(day); }
};

static_assert(EOPcontainer<EOPVector>);

template <std::size_t N> class EOPArray {
  std::array<DailyEOP, N> data_{};
  double mjd0_ = 0;

public:
  EOPArray() = default;

  // Window of N days, starting at MJD mjd0.
  explicit EOPArray(double mjd0) noexcept : mjd0_(mjd0) {}

  EOPArray(const std::array<DailyEOP, N> &data, double mjd0) noexcept
      : data_(data), mjd0_(mjd0) {}

  DailyEOP &operator[](std::size_t i) { return data_[i]; }
  const DailyEOP &operator[](std::size_t i) const { return data_[i]; }

  std::size_t size() const noexcept { return N; }
  double mjd0() const noexcept { return mjd0_; }

  auto begin() const noexcept { return data_.begin(); }
  auto end() const noexcept { return data_.end(); }

  void set_mjd0(double mjd0) noexcept { mjd0_ = mjd0; }
};

static_assert(EOPcontainer<EOPArray<1>>);

#endif
