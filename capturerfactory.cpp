#include "capturerfactory.h"

#include <boost/log/trivial.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;
using namespace std;
using namespace nlohmann;

shared_ptr<Capturer> CapturerFactory::createCapturer(
    const nlohmann::json &_config) {
  bool on = _config.contains("on") && _config["on"].is_boolean()
                ? _config["on"].get<bool>()
                : false;

  if (!on) return nullptr;

  Rect2d rect;

  if (_config.contains("roiX") && _config["roiX"].is_number() &&
      _config.contains("roiY") && _config["roiY"].is_number() &&
      _config.contains("roiW") && _config["roiW"].is_number() &&
      _config.contains("roiH") && _config["roiH"].is_number())
    rect = Rect2d(_config["roiX"].get<double>(), _config["roiY"].get<double>(),
                  _config["roiW"].get<double>(), _config["roiH"].get<double>());

  return shared_ptr<Capturer>(new Capturer(
      _config.contains("source") && _config["source"].is_string()
          ? _config["source"].get<string>()
          : "",
      _config.contains("settedFrameWidth") &&
              _config["settedFrameWidth"].is_number()
          ? _config["settedFrameWidth"].get<int>()
          : -1,
      _config.contains("settedFrameHeight") &&
              _config["settedFrameHeight"].is_number()
          ? _config["settedFrameHeight"].get<int>()
          : -1,
      _config.contains("settedCodec") && _config["settedCodec"].is_string()
          ? _config["settedCodec"].get<string>()
          : "",
      _config.contains("settedFps") && _config["settedFps"].is_number()
          ? _config["settedFps"].get<int>()
          : -1,
      rect,
      _config.contains("framesDelayMs") && _config["framesDelayMs"].is_number()
          ? _config["framesDelayMs"].get<int>()
          : 0,
      _config.contains("origFrameName") && _config["origFrameName"].is_string()
          ? _config["origFrameName"].get<string>()
          : "",
      _config.contains("waitTimeoutMs") && _config["waitTimeoutMs"].is_number()
          ? _config["waitTimeoutMs"].get<int>()
          : 200));
}
