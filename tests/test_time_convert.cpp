#include "Time/Transform.hpp"
#include "Time/Time.hpp"
#include "Time/Converter.hpp"
#include "constants.hpp"

#include <gtest/gtest.h>

namespace {

constexpr double j2000 = 2451545.0;

// Tolerances are in days. A round trip through SOFA loses a little in the
// split of the two-part Julian date, so it is checked at microsecond level.
constexpr double usec = 1e-6 / secondsInDay;
constexpr double msec = 1e-3 / secondsInDay;

double total(const auto &T) noexcept { return T.jd1() + T.jd2(); }

// Difference in seconds, taken on the parts so the leading jd1 does not eat
// the small quantity we are after.
double diffSeconds(const auto &lhs, const auto &rhs) noexcept {
  return ((lhs.jd1() - rhs.jd1()) + (lhs.jd2() - rhs.jd2())) * secondsInDay;
}

class ConverterTest : public ::testing::Test {
protected:
  // Offsets close to the real ones around J2000, so the numbers below stay
  // recognisable, but exact and epoch independent.
  ConverterTest() : cv_(DummyDelta(0.25, 69.184, 0.001)) {}

  Converter<DummyDelta> cv_;
};

TEST_F(ConverterTest, SameScaleIsIdentity) {
  const Time<Scale::TT> in(j2000, 0.25);
  const auto out = cv_.convert<Scale::TT>(in);

  EXPECT_DOUBLE_EQ(total(out), total(in));
}

TEST_F(ConverterTest, TtToTaiIsThirtyTwoPointOneEightFourSeconds) {
  const Time<Scale::TT> tt(j2000, 0.0);
  const auto tai = cv_.convert<Scale::TAI>(tt);

  EXPECT_NEAR(diffSeconds(tt, tai), 32.184, 1e-6);
}

TEST_F(ConverterTest, TtToTdbUsesTheHandlerOffset) {
  const Time<Scale::TT> tt(j2000, 0.0);
  const auto tdb = cv_.convert<Scale::TDB>(tt);

  EXPECT_NEAR(diffSeconds(tdb, tt), 0.001, 1e-9);
}

TEST_F(ConverterTest, UtcToUt1UsesTheHandlerOffset) {
  const Time<Scale::UTC> utc(j2000, 0.0);
  const auto ut1 = cv_.convert<Scale::UT1>(utc);

  EXPECT_NEAR(diffSeconds(ut1, utc), 0.25, 1e-6);
}

// Exercises the branch that was passing the member function instead of the
// time object.
TEST_F(ConverterTest, TtToUt1UsesTheHandlerOffset) {
  const Time<Scale::TT> tt(j2000, 0.0);
  const auto ut1 = cv_.convert<Scale::UT1>(tt);

  EXPECT_NEAR(diffSeconds(tt, ut1), 69.184, 1e-6);
}

// Every directly connected pair, out and back.
class RoundTripTest : public ConverterTest {};

TEST_F(RoundTripTest, TtTai) {
  const Time<Scale::TT> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::TT>(cv_.convert<Scale::TAI>(in));

  EXPECT_NEAR(total(out), total(in), usec);
}

TEST_F(RoundTripTest, TaiUtc) {
  const Time<Scale::TAI> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::TAI>(cv_.convert<Scale::UTC>(in));

  EXPECT_NEAR(total(out), total(in), usec);
}

TEST_F(RoundTripTest, UtcUt1) {
  const Time<Scale::UTC> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::UTC>(cv_.convert<Scale::UT1>(in));

  EXPECT_NEAR(total(out), total(in), msec);
}

TEST_F(RoundTripTest, TtUt1) {
  const Time<Scale::TT> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::TT>(cv_.convert<Scale::UT1>(in));

  EXPECT_NEAR(total(out), total(in), usec);
}

TEST_F(RoundTripTest, TtTcg) {
  const Time<Scale::TT> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::TT>(cv_.convert<Scale::TCG>(in));

  EXPECT_NEAR(total(out), total(in), usec);
}

TEST_F(RoundTripTest, TtTdb) {
  const Time<Scale::TT> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::TT>(cv_.convert<Scale::TDB>(in));

  EXPECT_NEAR(total(out), total(in), usec);
}

TEST_F(RoundTripTest, TdbTcb) {
  const Time<Scale::TDB> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::TDB>(cv_.convert<Scale::TCB>(in));

  EXPECT_NEAR(total(out), total(in), usec);
}

// Paths with no direct SOFA routine, resolved by the DFS and walked one hop
// at a time.
class MultiHopTest : public ConverterTest {};

TEST_F(MultiHopTest, UtcToTtMatchesTheManualChain) {
  const Time<Scale::UTC> utc(j2000, 0.3);

  const auto direct = cv_.convert<Scale::TT>(utc);
  const auto manual = cv_.convert<Scale::TT>(cv_.convert<Scale::TAI>(utc));

  EXPECT_NEAR(total(direct), total(manual), usec);
}

TEST_F(MultiHopTest, UtcToTcbRoundTrips) {
  const Time<Scale::UTC> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::UTC>(cv_.convert<Scale::TCB>(in));

  EXPECT_NEAR(total(out), total(in), msec);
}

TEST_F(MultiHopTest, Ut1ToTcgRoundTrips) {
  const Time<Scale::UT1> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::UT1>(cv_.convert<Scale::TCG>(in));

  EXPECT_NEAR(total(out), total(in), usec);
}

TEST_F(MultiHopTest, TcbToUtcRoundTrips) {
  const Time<Scale::TCB> in(j2000, 0.3);
  const auto out = cv_.convert<Scale::TCB>(cv_.convert<Scale::UTC>(in));

  EXPECT_NEAR(total(out), total(in), msec);
}

// Deltas travel the same paths as absolute times.
TEST_F(ConverterTest, DeltasConvertToo) {
  const TimeDelta<Scale::TT> in(0.0, 0.5);
  const auto out = cv_.convert<Scale::TAI>(in);

  static_assert(std::same_as<decltype(out), const TimeDelta<Scale::TAI>>);
  EXPECT_NEAR(diffSeconds(in, out), 32.184, 1e-6);
}

class AutoConvertTest : public ConverterTest {};

TEST_F(AutoConvertTest, TargetScaleComesFromTheDestination) {
  const Time<Scale::TT> tt(j2000, 0.0);

  const Time<Scale::TAI> tai = cv_.autoconvert(tt);

  EXPECT_NEAR(diffSeconds(tt, tai), 32.184, 1e-6);
}

TEST_F(AutoConvertTest, AgreesWithAnExplicitConversion) {
  const Time<Scale::UTC> utc(j2000, 0.3);

  const Time<Scale::TT> automatic = cv_.autoconvert(utc);
  const auto explicitly = cv_.convert<Scale::TT>(utc);

  EXPECT_DOUBLE_EQ(total(automatic), total(explicitly));
}

TEST_F(AutoConvertTest, WorksForDeltas) {
  const TimeDelta<Scale::TT> in(0.0, 0.5);

  const TimeDelta<Scale::TAI> out = cv_.autoconvert(in);

  EXPECT_NEAR(diffSeconds(in, out), 32.184, 1e-6);
}

} // namespace
