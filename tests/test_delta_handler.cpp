#include "DeltaHandler.hpp"
#include "Time.hpp"
#include "TimeConvert.hpp"
#include "constants.hpp"

#include <gtest/gtest.h>

// The dummy has to keep satisfying the concept the Converter is written
// against, otherwise every conversion falls back to a template error.
static_assert(TimeDeltaHandler<DummyDelta>);

namespace {

constexpr double j2000 = 2451545.0;

TEST(DummyDelta, DefaultsAreReported) {
  const DummyDelta d;

  EXPECT_DOUBLE_EQ(d.dut(Time<Scale::UTC>(j2000, 0.0)),
                   DummyDelta::defaultDut);
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

} // namespace
