#ifndef adapter_iersbulletinb_hpp__
#define adapter_iersbulletinb_hpp__

#include "EOP/EOP.hpp"

#include <charconv>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Parser for IERS Bulletin B, plain text, format 2009 ("Upgraded C04"
// solution). Only section 1 is read: the daily final values of x, y and
// UT1-UTC, followed by the preliminary extension, which continues the same
// daily series and is kept so the table reaches the publication date.
//
// The bulletin states angles in milliarcseconds and UT1-UTC in milliseconds.
// DailyEOP is filled in the units the rest of the library works in: dut in
// seconds, X and Y in radians.
//
// The container is not owned here. The caller brings an EOPcontainer that
// already knows its window through mjd0() and size(), and the rows are
// written into the slots that window covers.
namespace iers {

inline constexpr double mas2rad = 3.14159265358979323846 / (180.0 * 3600e3);
inline constexpr double ms2s = 1e-3;

class ParseError : public std::runtime_error {
public:
  explicit ParseError(const std::string &what) : std::runtime_error(what) {}
};

// What a fill covered, so a caller can tell a short bulletin from a wrong one.
struct FillReport {
  std::size_t written = 0; // slots of the container that got a row
  std::size_t skipped = 0; // rows of the bulletin outside the window
  double firstMjd = 0;     // first row in the bulletin
  double lastMjd = 0;      // last row in the bulletin
};

namespace detail {

inline std::string_view trim(std::string_view s) noexcept {
  const auto first = s.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos)
    return {};
  const auto last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, last - first + 1);
}

// Splits on runs of blank space. The bulletin mixes spaces and tabs, and the
// column widths move between editions, so the rows are read by token and not
// by offset.
inline std::vector<std::string_view> tokenize(std::string_view line) {
  std::vector<std::string_view> out;
  std::size_t i = 0;
  while (i < line.size()) {
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
      ++i;
    const std::size_t start = i;
    while (i < line.size() && line[i] != ' ' && line[i] != '\t')
      ++i;
    if (i > start)
      out.push_back(line.substr(start, i - start));
  }
  return out;
}

inline bool to_double(std::string_view tok, double &value) noexcept {
  const auto *const end = tok.data() + tok.size();
  const auto res = std::from_chars(tok.data(), end, value);
  return res.ec == std::errc{} && res.ptr == end;
}

inline bool to_int(std::string_view tok, int &value) noexcept {
  const auto *const end = tok.data() + tok.size();
  const auto res = std::from_chars(tok.data(), end, value);
  return res.ec == std::errc{} && res.ptr == end;
}

// A data row starts with the date and the MJD as plain integers:
// year month day mjd x y UT1-UTC ...
// Everything else in the section (titles, units, the mean formal error line,
// the "Preliminary extension" marker) fails one of these checks.
inline bool parse_row(const std::vector<std::string_view> &tok, DailyEOP &out) {
  if (tok.size() < 7)
    return false;

  int year = 0, month = 0, day = 0, mjd = 0;
  if (!to_int(tok[0], year) || !to_int(tok[1], month) || !to_int(tok[2], day) ||
      !to_int(tok[3], mjd))
    return false;
  if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31 ||
      mjd < 15020)
    return false;

  double x = 0, y = 0, dut = 0;
  if (!to_double(tok[4], x) || !to_double(tok[5], y) || !to_double(tok[6], dut))
    throw ParseError("Bulletin B: row for MJD " + std::to_string(mjd) +
                     " has a date but no readable x, y, UT1-UTC");

  out = DailyEOP{(double)mjd, dut * ms2s, x * mas2rad, y * mas2rad};
  return true;
}

inline bool is_section_header(std::string_view line, int number) {
  return trim(line).starts_with(std::to_string(number) + " - ");
}

// Section 1 of the bulletin, row by row, in file order.
inline std::vector<DailyEOP> section1_rows(std::string_view text) {
  std::vector<DailyEOP> rows;
  bool inSection = false;

  std::size_t pos = 0;
  while (pos <= text.size()) {
    const std::size_t nl = text.find('\n', pos);
    const std::string_view line =
        text.substr(pos, nl == std::string_view::npos ? nl : nl - pos);

    if (!inSection) {
      if (is_section_header(line, 1))
        inSection = true;
    } else if (is_section_header(line, 2)) {
      break;
    } else {
      DailyEOP row{};
      if (parse_row(tokenize(line), row))
        rows.push_back(row);
    }

    if (nl == std::string_view::npos)
      break;
    pos = nl + 1;
  }

  if (!inSection)
    throw ParseError("Bulletin B: section 1 header not found; the input is "
                     "not a format 2009 bulletin");
  if (rows.empty())
    throw ParseError("Bulletin B: section 1 holds no daily rows");

  // EOP interpolates on a fixed one-day step, so a hole in the series would
  // read as valid data for the wrong date.
  for (std::size_t i = 1; i < rows.size(); ++i)
    if (rows[i].mjd != rows[i - 1].mjd + 1.0)
      throw ParseError("Bulletin B: daily series is not continuous, MJD " +
                       std::to_string((long)rows[i - 1].mjd) +
                       " is followed by " + std::to_string((long)rows[i].mjd));

  return rows;
}

} // namespace detail

// Fills the slots of ctr that the bulletin covers. Slot i stands for
// MJD ctr.mjd0() + i, which is the mapping EOP::get_index uses, so a row is
// written at index mjd - mjd0(). Rows outside [mjd0(), mjd0() + size()) are
// counted as skipped. Throws ParseError if the bulletin is unreadable or
// covers none of the window; slots the bulletin does not reach keep whatever
// the caller put there.
template <EOPcontainer C>
FillReport fill_from_bulletin_b(C &ctr, std::string_view text) {
  const std::vector<DailyEOP> rows = detail::section1_rows(text);

  const double mjd0 = ctr.mjd0();
  const std::size_t size = ctr.size();

  FillReport report;
  report.firstMjd = rows.front().mjd;
  report.lastMjd = rows.back().mjd;

  for (const DailyEOP &row : rows) {
    const double offset = row.mjd - mjd0;
    if (offset < 0 || offset >= (double)size) {
      ++report.skipped;
      continue;
    }
    ctr[(std::size_t)offset] = row;
    ++report.written;
  }

  if (report.written == 0)
    throw ParseError("Bulletin B: covers MJD " +
                     std::to_string((long)report.firstMjd) + " to " +
                     std::to_string((long)report.lastMjd) +
                     ", the container window starts at MJD " +
                     std::to_string((long)mjd0) + " and holds " +
                     std::to_string(size) + " days");

  return report;
}

} // namespace iers

#endif
