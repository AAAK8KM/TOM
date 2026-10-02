#include "EOP/Containers.hpp"
#include "EOP/EOP.hpp"
#include "IERSBulletinB.hpp"
#include "IERSFetch.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace {

// The fixture is bulletin 464: section 1 runs from MJD 61254 to 61314, with
// the final values ending at 61284 and the preliminary extension carrying the
// series on from 61285.
constexpr double firstMjd = 61254;
constexpr double lastMjd = 61314;
constexpr std::size_t days = 61;

std::string fixture(const std::string &name) {
  const std::string path = std::string(ORBMECH_IERS_TEST_DATA) + "/" + name;
  std::ifstream in(path);
  if (!in)
    throw std::runtime_error("cannot open test data " + path);
  std::ostringstream out;
  out << in.rdbuf();
  return out.str();
}

const std::string &bulletin464() {
  static const std::string text = fixture("bulletinb-464.txt");
  return text;
}

TEST(BulletinB, FillsTheWholeWindow) {
  EOPVector w(firstMjd, days);

  const iers::FillReport r = iers::fill_from_bulletin_b(w, bulletin464());

  EXPECT_EQ(r.written, days);
  EXPECT_EQ(r.skipped, 0u);
  EXPECT_DOUBLE_EQ(r.firstMjd, firstMjd);
  EXPECT_DOUBLE_EQ(r.lastMjd, lastMjd);
}

TEST(BulletinB, SlotIndexIsTheDayOffsetFromMjd0) {
  EOPVector w(firstMjd, days);
  iers::fill_from_bulletin_b(w, bulletin464());

  EXPECT_DOUBLE_EQ(w[0].mjd, firstMjd);
  EXPECT_DOUBLE_EQ(w[days - 1].mjd, lastMjd);
  for (std::size_t i = 0; i < days; ++i)
    EXPECT_DOUBLE_EQ(w[i].mjd, firstMjd + (double)i);
}

// 2026 8 2 61254 222.428 364.675 12.2951, in mas and ms.
TEST(BulletinB, UnitsBecomeSecondsAndRadians) {
  EOPVector w(firstMjd, days);
  iers::fill_from_bulletin_b(w, bulletin464());

  EXPECT_DOUBLE_EQ(w[0].dut, 12.2951e-3);
  EXPECT_DOUBLE_EQ(w[0].X, 222.428 * iers::mas2rad);
  EXPECT_DOUBLE_EQ(w[0].Y, 364.675 * iers::mas2rad);
}

// The last row of section 1 sits in the preliminary extension, past the
// "Final values" block, and the UT1-UTC there is negative.
TEST(BulletinB, KeepsThePreliminaryExtension) {
  EOPVector w(firstMjd, days);
  iers::fill_from_bulletin_b(w, bulletin464());

  EXPECT_DOUBLE_EQ(w[31].mjd, 61285); // first extension row
  EXPECT_DOUBLE_EQ(w[31].dut, 1.7603e-3);
  EXPECT_DOUBLE_EQ(w[days - 1].dut, -22.5085e-3);
}

// Section 2 repeats the same dates with dPsi/dEps, so a parser that runs past
// the section boundary would overwrite the window with pole offsets.
TEST(BulletinB, StopsAtSectionTwo) {
  EOPVector w(firstMjd, days);
  iers::fill_from_bulletin_b(w, bulletin464());

  // dPsi1980 at MJD 61254 is -121.732 mas; x is +222.428 mas.
  EXPECT_GT(w[0].X, 0);
  EXPECT_DOUBLE_EQ(w[0].X, 222.428 * iers::mas2rad);
}

TEST(BulletinB, WindowShorterThanTheBulletinSkipsTheRest) {
  EOPVector w(firstMjd, 10);

  const iers::FillReport r = iers::fill_from_bulletin_b(w, bulletin464());

  EXPECT_EQ(r.written, 10u);
  EXPECT_EQ(r.skipped, days - 10);
  EXPECT_DOUBLE_EQ(w[9].mjd, firstMjd + 9);
}

TEST(BulletinB, WindowWiderThanTheBulletinLeavesTheRestAlone) {
  EOPVector w(firstMjd - 5, days + 10);

  const iers::FillReport r = iers::fill_from_bulletin_b(w, bulletin464());

  EXPECT_EQ(r.written, days);
  EXPECT_EQ(r.skipped, 0u);
  EXPECT_DOUBLE_EQ(w[0].mjd, 0); // untouched slot, still default constructed
  EXPECT_DOUBLE_EQ(w[5].mjd, firstMjd);
}

TEST(BulletinB, WindowOutsideTheBulletinThrows) {
  EOPVector w(firstMjd - 1000, days);

  EXPECT_THROW(iers::fill_from_bulletin_b(w, bulletin464()), iers::ParseError);
}

TEST(BulletinB, TextWithoutSectionOneThrows) {
  EOPVector w(firstMjd, days);

  EXPECT_THROW(iers::fill_from_bulletin_b(w, "<html>not a bulletin</html>"),
               iers::ParseError);
}

TEST(BulletinB, SectionOneWithoutRowsThrows) {
  EOPVector w(firstMjd, days);
  const std::string text = " 1 - DAILY FINAL VALUES OF x, y, UT1-UTC\n"
                           " Final values \n"
                           " Mean formal error      0.040    0.043\n"
                           " 2 - DAILY FINAL VALUES OF CELESTIAL POLE\n";

  EXPECT_THROW(iers::fill_from_bulletin_b(w, text), iers::ParseError);
}

TEST(BulletinB, GapInTheDailySeriesThrows) {
  EOPVector w(firstMjd, days);
  const std::string text =
      " 1 - DAILY FINAL VALUES OF x, y, UT1-UTC\n"
      "2026   8   2   61254  222.428  364.675   12.2951    0.391 -0.321\n"
      "2026   8   4   61256  222.615  362.934   11.2588    0.406 -0.270\n";

  EXPECT_THROW(iers::fill_from_bulletin_b(w, text), iers::ParseError);
}

TEST(BulletinB, UnreadableValueInADatedRowThrows) {
  EOPVector w(firstMjd, days);
  const std::string text =
      " 1 - DAILY FINAL VALUES OF x, y, UT1-UTC\n"
      "2026   8   2   61254  222.428  364.675   ****      0.391 -0.321\n";

  EXPECT_THROW(iers::fill_from_bulletin_b(w, text), iers::ParseError);
}

// What the container is for: EOP reads the filled window.
TEST(BulletinB, FilledWindowDrivesEop) {
  EOPVector w(firstMjd, days);
  iers::fill_from_bulletin_b(w, bulletin464());

  const EOP<EOPVector> eop(w);

  EXPECT_DOUBLE_EQ(eop.dut(firstMjd), 12.2951e-3);
  // Halfway between 61254 and 61255, 12.2951 ms and 11.7718 ms.
  EXPECT_DOUBLE_EQ(eop.dut(firstMjd + 0.5), (12.2951e-3 + 11.7718e-3) / 2);

  const CIPP p = eop.pole(firstMjd);
  EXPECT_DOUBLE_EQ(p.X, 222.428 * iers::mas2rad);
  EXPECT_DOUBLE_EQ(p.Y, 364.675 * iers::mas2rad);
}

// The fixed size container takes the same fill, with the window set by N.
TEST(BulletinB, FillsAnEopArray) {
  EOPArray<days> w(firstMjd);

  const iers::FillReport r = iers::fill_from_bulletin_b(w, bulletin464());

  EXPECT_EQ(r.written, days);
  EXPECT_EQ(r.skipped, 0u);
  EXPECT_DOUBLE_EQ(w[0].dut, 12.2951e-3);
  EXPECT_DOUBLE_EQ(w[days - 1].mjd, lastMjd);

  const EOP<EOPArray<days>> eop(w);
  EXPECT_DOUBLE_EQ(eop.dut(firstMjd), 12.2951e-3);
}

TEST(BulletinBUrl, NumberedBulletin) {
  EXPECT_EQ(iers::bulletin_b_url(464),
            "https://datacenter.iers.org/products/eop/bulletinb/format_2009/"
            "bulletinb-464.txt");
  EXPECT_THROW(iers::bulletin_b_url(0), iers::FetchError);
}

// Off by default: a build with the adapter enabled must pass without network
// access. Run with ORBMECH_IERS_NETWORK_TESTS=1 to hit the IERS data centre.
class NetworkTest : public ::testing::Test {
protected:
  void SetUp() override {
    const char *const flag = std::getenv("ORBMECH_IERS_NETWORK_TESTS");
    if (!flag || std::string(flag) != "1")
      GTEST_SKIP() << "set ORBMECH_IERS_NETWORK_TESTS=1 to run this";
  }
};

TEST_F(NetworkTest, FetchesTheLatestBulletin) {
  // Wide window, so any current bulletin lands inside it.
  EOPVector w(firstMjd, 4000);

  const iers::FillReport r = iers::fill_from_latest(w);

  EXPECT_GT(r.written, 30u);
  EXPECT_GE(r.lastMjd, r.firstMjd);
}

TEST_F(NetworkTest, FetchesANumberedBulletin) {
  EOPVector w(firstMjd, days);

  const iers::FillReport r = iers::fill_from_bulletin(w, 464);

  EXPECT_EQ(r.written, days);
  EXPECT_DOUBLE_EQ(r.firstMjd, firstMjd);
}

TEST_F(NetworkTest, BadUrlThrows) {
  EOPVector w(firstMjd, days);

  EXPECT_THROW(iers::fill_from_url(
                   w, "https://datacenter.iers.org/data/no/such/file.txt"),
               iers::FetchError);
}

} // namespace
