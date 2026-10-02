#include "IERSFetch.hpp"

#include <curl/curl.h>
#include <mutex>

namespace iers {
namespace {

// curl_global_init is not thread safe and must run once before any easy
// handle exists.
void global_init() {
  static std::once_flag once;
  static CURLcode code = CURLE_OK;
  std::call_once(once, [] { code = curl_global_init(CURL_GLOBAL_DEFAULT); });
  if (code != CURLE_OK)
    throw FetchError(std::string("curl_global_init failed: ") +
                     curl_easy_strerror(code));
}

std::size_t append(char *data, std::size_t size, std::size_t nmemb,
                   void *userp) {
  auto *const out = static_cast<std::string *>(userp);
  const std::size_t bytes = size * nmemb;
  out->append(data, bytes);
  return bytes;
}

struct EasyHandle {
  CURL *h = nullptr;
  EasyHandle() : h(curl_easy_init()) {
    if (!h)
      throw FetchError("curl_easy_init returned no handle");
  }
  ~EasyHandle() {
    if (h)
      curl_easy_cleanup(h);
  }
  EasyHandle(const EasyHandle &) = delete;
  EasyHandle &operator=(const EasyHandle &) = delete;
};

} // namespace

std::string bulletin_b_url(int number) {
  if (number <= 0)
    throw FetchError("Bulletin B numbers start at 1, got " +
                     std::to_string(number));
  return std::string(bulletinBPrefix) + std::to_string(number) + ".txt";
}

std::string http_get(const std::string &url, long timeoutSeconds) {
  global_init();

  EasyHandle easy;
  std::string body;

  curl_easy_setopt(easy.h, CURLOPT_URL, url.c_str());
  curl_easy_setopt(easy.h, CURLOPT_WRITEFUNCTION, append);
  curl_easy_setopt(easy.h, CURLOPT_WRITEDATA, &body);
  curl_easy_setopt(easy.h, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(easy.h, CURLOPT_TIMEOUT, timeoutSeconds);
  curl_easy_setopt(easy.h, CURLOPT_CONNECTTIMEOUT, timeoutSeconds);
  curl_easy_setopt(easy.h, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(easy.h, CURLOPT_USERAGENT, "OrbMech/0.1");
  curl_easy_setopt(easy.h, CURLOPT_ACCEPT_ENCODING, "");

  const CURLcode code = curl_easy_perform(easy.h);
  if (code != CURLE_OK)
    throw FetchError("GET " + url + " failed: " + curl_easy_strerror(code));

  long status = 0;
  curl_easy_getinfo(easy.h, CURLINFO_RESPONSE_CODE, &status);
  if (status >= 400)
    throw FetchError("GET " + url + " returned HTTP " +
                     std::to_string(status));
  if (body.empty())
    throw FetchError("GET " + url + " returned an empty body");

  return body;
}

} // namespace iers
