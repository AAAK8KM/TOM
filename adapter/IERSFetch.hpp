#ifndef adapter_iersfetch_hpp__
#define adapter_iersfetch_hpp__

#include "IERSBulletinB.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

namespace iers {

// The page at iers.org/iers/en/publications/bulletins/bulletins is an HTML
// index. The machine readable form of the same bulletins lives in the IERS
// data centre, and these are the two URLs it serves them at.
inline constexpr const char *latestBulletinB =
    "https://datacenter.iers.org/data/latestVersion/bulletinB.txt";
inline constexpr const char *bulletinBPrefix =
    "https://datacenter.iers.org/products/eop/bulletinb/format_2009/bulletinb-";

class FetchError : public std::runtime_error {
public:
  explicit FetchError(const std::string &what) : std::runtime_error(what) {}
};

// URL of one numbered bulletin, e.g. 464.
std::string bulletin_b_url(int number);

// Plain HTTP GET over libcurl. Throws FetchError on a transport error or on
// any HTTP status of 400 and above.
std::string http_get(const std::string &url, long timeoutSeconds = 30);

// Downloads a bulletin and fills the slots of ctr that it covers. Transport
// and parsing stay separate, so a caller that already holds the text calls
// fill_from_bulletin_b directly.
template <EOPcontainer C>
FillReport fill_from_url(C &ctr, const std::string &url,
                         long timeoutSeconds = 30) {
  const std::string text = http_get(url, timeoutSeconds);
  return fill_from_bulletin_b(ctr, text);
}

// Latest bulletin.
template <EOPcontainer C>
FillReport fill_from_latest(C &ctr, long timeoutSeconds = 30) {
  return fill_from_url(ctr, latestBulletinB, timeoutSeconds);
}

// One numbered bulletin.
template <EOPcontainer C>
FillReport fill_from_bulletin(C &ctr, int number, long timeoutSeconds = 30) {
  return fill_from_url(ctr, bulletin_b_url(number), timeoutSeconds);
}

} // namespace iers

#endif
