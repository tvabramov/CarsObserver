#include "utilities.h"

#include <iomanip>
#include <sstream>

using namespace std::chrono;

std::string timeToStrWithMs(const time_point<system_clock> &_timestamp) {
  milliseconds ms = duration_cast<milliseconds>(_timestamp.time_since_epoch());

  seconds s = duration_cast<seconds>(ms);
  time_t t = s.count();
  size_t fractional_seconds = ms.count() % 1000;

  auto tm = *localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0')
      << std::setw(3) << fractional_seconds;

  return oss.str();
}
