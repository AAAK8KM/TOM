#include "Time/Time.hpp"
#include "constants.hpp"

#include <gtest/gtest.h>

namespace {

constexpr double j2000 = 2451545.0;

using T = Time<Scale::TT>;
using D = TimeDelta<Scale::TT>;

double total(const auto &X) noexcept { return X.jd1() + X.jd2(); }

// jd1 carries the whole days and jd2 the fraction. Every operation runs
// normalize, so the split is checked alongside the value wherever it moves.
TEST(Normalize, WholeDaysGoToJd1) {
  const T t(j2000 + 0.5, 1.25);

  EXPECT_DOUBLE_EQ(t.jd1(), j2000 + 1.0);
  EXPECT_DOUBLE_EQ(t.jd2(), 0.75);
  EXPECT_DOUBLE_EQ(total(t), j2000 + 1.75);
}

TEST(TimeArith, PlusDelta) {
  const T t(j2000, 0.25);
  const D d(0.0, 0.5);

  const T sum = t + d;

  EXPECT_DOUBLE_EQ(total(sum), j2000 + 0.75);
  EXPECT_DOUBLE_EQ(sum.jd1(), j2000);
}

// The delta sits on either side of the +.
TEST(TimeArith, DeltaPlusTimeIsCommutative) {
  const T t(j2000, 0.25);
  const D d(0.0, 0.5);

  EXPECT_DOUBLE_EQ(total(t + d), total(d + t));
}

TEST(TimeArith, MinusDelta) {
  const T t(j2000, 0.75);
  const D d(0.0, 0.5);

  const T diff = t - d;

  EXPECT_DOUBLE_EQ(total(diff), j2000 + 0.25);
}

// Subtracting two absolute times is the one operation that changes category.
TEST(TimeArith, TimeMinusTimeIsADelta) {
  const T later(j2000 + 2.0, 0.5);
  const T earlier(j2000, 0.25);

  const auto d = later - earlier;

  static_assert(std::same_as<decltype(d), const D>);
  EXPECT_DOUBLE_EQ(total(d), 2.25);
}

TEST(TimeArith, TimeMinusItselfIsZero) {
  const T t(j2000, 0.25);

  EXPECT_DOUBLE_EQ(total(t - t), 0.0);
}

TEST(TimeArith, RoundTripThroughADelta) {
  const T t(j2000, 0.25);
  const D d(3.0, 0.5);

  EXPECT_DOUBLE_EQ(total((t + d) - d), total(t));
}

TEST(TimeArith, CompoundAssignment) {
  T t(j2000, 0.25);
  const D d(0.0, 0.25);

  t += d;
  EXPECT_DOUBLE_EQ(total(t), j2000 + 0.5);

  t -= d;
  EXPECT_DOUBLE_EQ(total(t), j2000 + 0.25);
}

TEST(TimeArith, Ordering) {
  const T earlier(j2000, 0.25);
  const T later(j2000, 0.75);

  EXPECT_LT(earlier, later);
  EXPECT_GT(later, earlier);
  EXPECT_EQ(earlier, T(j2000, 0.25));
}

TEST(DeltaArith, PlusAndMinus) {
  const D a(1.0, 0.25);
  const D b(0.0, 0.5);

  EXPECT_DOUBLE_EQ(total(a + b), 1.75);
  EXPECT_DOUBLE_EQ(total(a - b), 0.75);
}

TEST(DeltaArith, TimesScalar) {
  const D d(1.0, 0.25);

  EXPECT_DOUBLE_EQ(total(d * 2.0), 2.5);
  EXPECT_DOUBLE_EQ(total(d * 0.0), 0.0);
  EXPECT_DOUBLE_EQ(total(d * -1.0), -1.25);
}

TEST(DeltaArith, ScalarTimesDeltaIsCommutative) {
  const D d(1.0, 0.25);

  EXPECT_DOUBLE_EQ(total(2.0 * d), total(d * 2.0));
}

TEST(DeltaArith, DividedByScalar) {
  const D d(1.0, 0.5);

  EXPECT_DOUBLE_EQ(total(d / 2.0), 0.75);
  EXPECT_DOUBLE_EQ(total(d / 0.5), 3.0);
}

TEST(DeltaArith, MultiplyThenDivideRoundTrips) {
  const D d(2.0, 0.25);

  EXPECT_DOUBLE_EQ(total((d * 8.0) / 8.0), total(d));
}

TEST(DeltaArith, CompoundAssignment) {
  D d(1.0, 0.0);

  d += D(0.0, 0.5);
  EXPECT_DOUBLE_EQ(total(d), 1.5);

  d -= D(0.0, 0.25);
  EXPECT_DOUBLE_EQ(total(d), 1.25);

  d *= 4.0;
  EXPECT_DOUBLE_EQ(total(d), 5.0);

  d /= 2.0;
  EXPECT_DOUBLE_EQ(total(d), 2.5);
}

TEST(DeltaArith, SelfSubtractionIsZero) {
  D d(3.0, 0.5);

  d -= d;

  EXPECT_DOUBLE_EQ(total(d), 0.0);
}

// A delta is a duration, so it is worth pinning that it reads back in
// seconds the way the scaling operators imply.
TEST(DeltaArith, OneDayIsSecondsInDay) {
  const D oneDay(1.0, 0.0);

  EXPECT_DOUBLE_EQ(total(oneDay) * secondsInDay, secondsInDay);
  EXPECT_DOUBLE_EQ(total(oneDay / secondsInDay) * secondsInDay, 1.0);
}

TEST(DeltaArith, Ordering) {
  const D shorter(0.0, 0.25);
  const D longer(1.0, 0.0);

  EXPECT_LT(shorter, longer);
  EXPECT_EQ(shorter, D(0.0, 0.25));
}

// MJD is what the EOP tables are indexed by, so it is taken off jd1, which
// carries the whole days, with the fraction added back afterwards.
TEST(Mjd, J2000IsFiftyOneFiveFourFourPointFive) {
  const T t(j2000, 0.0);

  EXPECT_DOUBLE_EQ(t.mjd(), 51544.5);
  EXPECT_DOUBLE_EQ(t.mjd(), t.jd() - mjdZero);
}

TEST(Mjd, FollowsTheFraction) {
  const T t(j2000, 0.25);

  EXPECT_DOUBLE_EQ(t.mjd(), 51544.75);
}

TEST(Mjd, KeepsTheSmallPartOfTheSplit) {
  const T t(j2000, 1e-9);

  EXPECT_DOUBLE_EQ(t.mjd(), 51544.5 + 1e-9);
  EXPECT_NEAR(t.mjd() - 51544.5, 1e-9, 1e-11);
}
} // namespace
