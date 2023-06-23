#include <cstdint>
#include "yahttp/yahttp.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  try {
    YaHTTP::AsyncRequestLoader yarl;
    YaHTTP::Request req;

    yarl.initialize(&req);
    bool finished = yarl.feed(std::string(reinterpret_cast<const char*>(data), size));
    if (finished) {
      yarl.finalize();
    }
  }
  catch (const YaHTTP::ParseError& e) {
  }
  catch (const std::exception& e) {
  }
  catch (...) {
  }

  return 0;
}
