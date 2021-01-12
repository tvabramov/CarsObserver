#ifndef UTILITIES_H
#define UTILITIES_H

#include <chrono>
#include <string>

std::string timeToStrWithMs(
    const std::chrono::time_point<std::chrono::system_clock> &_timestamp);

#endif  // UTILITIES_H
