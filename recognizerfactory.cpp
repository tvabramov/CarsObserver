#include "recognizerfactory.h"

#include <boost/log/trivial.hpp>
#include <memory>

#include "recognizers/facerecognizer.h"
#include "recognizers/mobilenetssdrecognizer.h"

using namespace cv;
using namespace std;
using namespace nlohmann;

shared_ptr<Recognizer> RecognizerFactory::createRecognizer(
    const json &_config) {
  bool on = _config.contains("on") && _config["on"].is_boolean()
                ? _config["on"].get<bool>()
                : false;

  if (!on) return nullptr;

  return shared_ptr<Recognizer>(new Recognizer(
      createInternalRecognizer(_config.contains("InternalRecognizer")
                                   ? _config["InternalRecognizer"]
                                   : json()),
      _config.contains("recognitionDelayMs") &&
              _config["recognitionDelayMs"].is_number()
          ? _config["recognitionDelayMs"].get<int>()
          : 300,
      _config.contains("maxPendingFrames") &&
              _config["maxPendingFrames"].is_number()
          ? _config["maxPendingFrames"].get<int>()
          : 10,
      _config.contains("waitTimeoutMs") && _config["waitTimeoutMs"].is_number()
          ? _config["waitTimeoutMs"].get<int>()
          : 200));
}

unique_ptr<AbstractRecognizer> RecognizerFactory::createInternalRecognizer(
    const json &_config) {
  if (_config.contains("typeName") && _config["typeName"].is_string()) {
    auto name = _config["typeName"].get<string>();
    if (name == "MobileNetSSDRecognizer")
      return unique_ptr<AbstractRecognizer>(new MobileNetSSDRecognizer());
  }

  return unique_ptr<AbstractRecognizer>(new FaceRecognizer());
}
