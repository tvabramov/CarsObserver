#include "crosscounterfactory.h"

#include <boost/log/trivial.hpp>

using namespace cv;
using namespace std;
using namespace nlohmann;

shared_ptr<CrossCounter> CrossCounterFactory::createCrossCounter(
    const nlohmann::json &_config) {
  bool on = _config.contains("on") && _config["on"].is_boolean()
                ? _config["on"].get<bool>()
                : false;

  if (!on) return nullptr;

  vector<pair<Point2d, Point2d>> lines;

  if (_config.contains("lines") && _config["lines"].is_array())
    for (json::const_iterator it = _config["lines"].begin();
         it != _config["lines"].end(); ++it)
      if (it->contains("begX") && (*it)["begX"].is_number() &&
          it->contains("begY") && (*it)["begY"].is_number() &&
          it->contains("endX") && (*it)["endX"].is_number() &&
          it->contains("endY") && (*it)["endY"].is_number())
        lines.push_back(make_pair(
            Point2d((*it)["begX"].get<double>(), (*it)["begY"].get<double>()),
            Point2d((*it)["endX"].get<double>(), (*it)["endY"].get<double>())));

  Size debugVideoSize;

  if (_config.contains("debugVideoWidth") &&
      _config["debugVideoWidth"].is_number() &&
      _config.contains("debugVideoHeight") &&
      _config["debugVideoHeight"].is_number())
    debugVideoSize = Size(_config["debugVideoWidth"].get<int>(),
                          _config["debugVideoHeight"].get<int>());

  return shared_ptr<CrossCounter>(new CrossCounter(
      _config.contains("debugScreenOutput") &&
              _config["debugScreenOutput"].is_boolean()
          ? _config["debugScreenOutput"].get<bool>()
          : false,
      debugVideoSize, lines,
      _config.contains("waitTimeoutMs") && _config["waitTimeoutMs"].is_number()
          ? _config["waitTimeoutMs"].get<int>()
          : 200));
}
