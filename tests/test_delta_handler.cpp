#include "EOP/Containers.hpp"
#include "EOP/EOP.hpp"
#include "Time/Converter.hpp"
#include "Time/Time.hpp"
#include "Time/Transform.hpp"
#include "constants.hpp"

#include <gtest/gtest.h>

// The dummy has to keep satisfying the concept the Converter is written
// against, otherwise every conversion falls back to a template error.
static_assert(TimeDeltaHandler<DummyDelta>);

namespace {

constexpr double j2000 = 2451545.0;

TEST(DummyDelta, DefaultsAreReported) {
  const DummyDelta d;

  EXPECT_DOUBLE_EQ(d.dut(Time<Scale::UTC>(j2000, 0.0)), DummyDelta::defaultDut);
  EXPECT_DOUBLE_EQ(d.dttut(Time<Scale::TT>(j2000, 0.0)),
                   DummyDelta::defaultDttut);
  EXPECT_DOUBLE_EQ(d.dtdb(Time<Scale::TT>(j2000, 0.0)),
                   DummyDelta::defaultDtdb);
}

TEST(DummyDelta, ConstructorSetsEveryOffset) {
  const DummyDelta d(0.25, 69.184, 0.001);

  EXPECT_DOUBLE_EQ(d.dut(Time<Scale::UTC>(j2000, 0.0)), 0.25);
  EXPECT_DOUBLE_EQ(d.dttut(Time<Scale::TT>(j2000, 0.0)), 69.184);
  EXPECT_DOUBLE_EQ(d.dtdb(Time<Scale::TT>(j2000, 0.0)), 0.001);
}

TEST(DummyDelta, SettersReplaceOffsets) {
  DummyDelta d;

  d.set_dut(-0.5);
  d.set_dttut(70.0);
  d.set_dtdb(0.002);

  EXPECT_DOUBLE_EQ(d.dut(Time<Scale::UTC>(j2000, 0.0)), -0.5);
  EXPECT_DOUBLE_EQ(d.dttut(Time<Scale::TT>(j2000, 0.0)), 70.0);
  EXPECT_DOUBLE_EQ(d.dtdb(Time<Scale::TT>(j2000, 0.0)), 0.002);
}

// Each getter is reachable from either end of the pair it corrects, which is
// what the concept demands.
TEST(DummyDelta, BothScalesOfAPairAreAccepted) {
  const DummyDelta d(0.25, 69.184, 0.001);

  EXPECT_DOUBLE_EQ(d.dut(Time<Scale::UT1>(j2000, 0.0)),
                   d.dut(Time<Scale::UTC>(j2000, 0.0)));
  EXPECT_DOUBLE_EQ(d.dttut(Time<Scale::UT1>(j2000, 0.0)),
                   d.dttut(Time<Scale::TT>(j2000, 0.0)));
  EXPECT_DOUBLE_EQ(d.dtdb(Time<Scale::TDB>(j2000, 0.0)),
                   d.dtdb(Time<Scale::TT>(j2000, 0.0)));
}

// Being a dummy, it ignores the epoch entirely. Pinning that down keeps the
// conversion tests honest about why their expectations are constants.
TEST(DummyDelta, OffsetsDoNotDependOnEpoch) {
  const DummyDelta d(0.25, 69.184, 0.001);

  EXPECT_DOUBLE_EQ(d.dut(Time<Scale::UTC>(j2000, 0.0)),
                   d.dut(Time<Scale::UTC>(j2000 + 3652.5, 0.0)));
  EXPECT_DOUBLE_EQ(d.dttut(Time<Scale::TT>(j2000, 0.0)),
                   d.dttut(Time<Scale::TT>(j2000 + 3652.5, 0.0)));
  EXPECT_DOUBLE_EQ(d.dtdb(Time<Scale::TT>(j2000, 0.0)),
                   d.dtdb(Time<Scale::TT>(j2000 + 3652.5, 0.0)));
}

TEST(DummyDelta, DeltasAreAcceptedAsWellAsTimes) {
  const DummyDelta d(0.25, 69.184, 0.001);

  EXPECT_DOUBLE_EQ(d.dut(TimeDelta<Scale::UTC>(1.0, 0.0)), 0.25);
  EXPECT_DOUBLE_EQ(d.dttut(TimeDelta<Scale::TT>(1.0, 0.0)), 69.184);
  EXPECT_DOUBLE_EQ(d.dtdb(TimeDelta<Scale::TDB>(1.0, 0.0)), 0.001);
}


// EOPTimeDelta reads a table indexed by MJD in UTC, and reports every offset
// in seconds, the units SOFA takes.
class EOPTimeDeltaTest : public ::testing::Test {
protected:
  // Two days around J2000, UT1 - UTC stepping 0.1 s a day.
  EOPTimeDeltaTest() : table_(51544.0, 3) {
    table_[0] = DailyEOP{51544.0, 0.1, 0, 0};
    table_[1] = DailyEOP{51545.0, 0.2, 0, 0};
    table_[2] = DailyEOP{51546.0, 0.3, 0, 0};
  }

  static constexpr double ttUtc = 32.184 + 37.0;

  EOPVector table_;
};

TEST_F(EOPTimeDeltaTest, DutIsLookedUpByMjdNotJd) {
  const EOP<EOPVector> eop(table_);
  const EOPTimeDelta<EOP<EOPVector>> d(eop);

  // J2000 is MJD 51544.5, halfway between the first two rows.
  EXPECT_DOUBLE_EQ(d.dut(Time<Scale::UTC>(j2000, 0.0)), 0.15);
  EXPECT_DOUBLE_EQ(d.dut(Time<Scale::UTC>(j2000, 0.25)), 0.175);
}

// TT - UT1 = (TT - UTC) - (UT1 - UTC), so it runs a touch under 69.184 s here.
TEST_F(EOPTimeDeltaTest, DttutIsTtMinusUt1InSeconds) {
  const EOP<EOPVector> eop(table_);
  const EOPTimeDelta<EOP<EOPVector>> d(eop);

  const Time<Scale::TT> tt(j2000, 0.0);
  const double utcMjd = tt.mjd() - ttUtc / secondsInDay;
  const double dut = 0.1 + 0.1 * (utcMjd - 51544.0);

  EXPECT_NEAR(d.dttut(tt), ttUtc - dut, 1e-12);
  EXPECT_LT(d.dttut(tt), ttUtc);
}

// The UT1 branch has to find the UTC of the instant before it can read the
// table, so it agrees with the UTC branch to well inside a microsecond.
TEST_F(EOPTimeDeltaTest, Ut1BranchAgreesWithTheUtcBranch) {
  const EOP<EOPVector> eop(table_);
  const EOPTimeDelta<EOP<EOPVector>> d(eop);

  const Time<Scale::UTC> utc(j2000, 0.0);
  const double dut = d.dut(utc);
  const Time<Scale::UT1> ut1(j2000, dut / secondsInDay);

  EXPECT_NEAR(d.dut(ut1), dut, 1e-9);
  EXPECT_NEAR(d.dttut(ut1), ttUtc - dut, 1e-9);
}

TEST_F(EOPTimeDeltaTest, SatisfiesTheConverterConcept) {
  static_assert(TimeDeltaHandler<EOPTimeDelta<EOP<EOPVector>>>);

  const EOP<EOPVector> eop(table_);
  const EOPTimeDelta<EOP<EOPVector>> delta(eop);
  const Converter<EOPTimeDelta<EOP<EOPVector>>> cv(delta);

  const Time<Scale::UTC> utc(j2000, 0.0);
  const auto ut1 = cv.convert<Scale::UT1>(utc);

  EXPECT_NEAR((ut1.jd1() - utc.jd1() + ut1.jd2() - utc.jd2()) * secondsInDay,
              0.15, 1e-6);
}
} // namespace
