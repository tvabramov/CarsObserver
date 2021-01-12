#include "trackerfactory.h"

#include <boost/log/trivial.hpp>
#include <memory>

#include "recognizers/abstractrecognizer.h"
#include "trackers/cvtracker.h"
#include "verifiers/hunverifier.h"

using namespace std;
using namespace nlohmann;

shared_ptr<Tracker> TrackerFactory::createTracker(const json &_config) {
  bool on = _config.contains("on") && _config["on"].is_boolean()
                ? _config["on"].get<bool>()
                : false;

  if (!on) return nullptr;

  return shared_ptr<Tracker>(new Tracker(
      createInternalTracker(_config.contains("InternalTracker")
                                ? _config["InternalTracker"]
                                : json()),
      createVerifier(_config.contains("Verifier") ? _config["Verifier"]
                                                  : json()),
      createWeakFitFunc(_config.contains("WeakFitFunc") ? _config["WeakFitFunc"]
                                                        : json()),
      createStrongFitFunc(_config.contains("StrongFitFunc")
                              ? _config["StrongFitFunc"]
                              : json()),
      _config.contains("maxPendingFrames") &&
              _config["maxPendingFrames"].is_number()
          ? _config["maxPendingFrames"].get<int>()
          : 10,
      _config.contains("waitTimeoutMs") && _config["waitTimeoutMs"].is_number()
          ? _config["waitTimeoutMs"].get<int>()
          : 200));
}

unique_ptr<AbstractTracker> TrackerFactory::createInternalTracker(
    const json &_config) {
  int frameWidth =
      _config.contains("frameWidth") && _config["frameWidth"].is_number()
          ? _config["frameWidth"].get<int>()
          : 150;
  int frameHeight =
      _config.contains("frameHeight") && _config["frameHeight"].is_number()
          ? _config["frameHeight"].get<int>()
          : 150;

  if (_config.contains("typeName") && _config["typeName"].is_string()) {
    auto name = _config["typeName"].get<string>();

    if (name == "CvTracker") {
      if (_config.contains("cvTrackerTypeName") &&
          _config["cvTrackerTypeName"].is_string()) {
        auto cvname = _config["cvTrackerTypeName"].get<string>();

        if (cvname == "KCF")
          return unique_ptr<AbstractTracker>(
              new CvTracker([]() { return cv::TrackerKCF::create(); },
                            frameWidth, frameHeight));
      }
    }
  }

  return unique_ptr<AbstractTracker>(new CvTracker(
      []() { return cv::TrackerCSRT::create(); }, frameWidth, frameHeight));
}

unique_ptr<AbstractVerifier> TrackerFactory::createVerifier(
    const json &_config) {
  return unique_ptr<AbstractVerifier>(new HunVerifier(
      _config.contains("threshold") && _config["threshold"].is_number()
          ? _config["threshold"].get<double>()
          : 0.2,
      _config.contains("maxRecFails") && _config["maxRecFails"].is_number()
          ? _config["maxRecFails"].get<int>()
          : 0));
}

AbstractVerifier::ItemFilterFunction TrackerFactory::createWeakFitFunc(
    const json &_config) {
  double threshold =
      _config.contains("threshold") && _config["threshold"].is_number()
          ? _config["threshold"].get<double>()
          : 0.1;

  list<int> ids;
  if (_config.contains("ids") && _config["ids"].is_array())
    for (json::const_iterator it = _config["ids"].begin();
         it != _config["ids"].end(); ++it)
      if (it->is_number()) ids.push_back(it->get<int>());

  return [threshold, ids](const RecognizedItem &_item) {
    return (ids.empty() ||
            find(ids.begin(), ids.end(), _item.type) != ids.end()) &&
           _item.confidence >= threshold;
  };
}

AbstractVerifier::ItemFilterFunction TrackerFactory::createStrongFitFunc(
    const json &_config) {
  double threshold =
      _config.contains("threshold") && _config["threshold"].is_number()
          ? _config["threshold"].get<double>()
          : 0.9;

  list<int> ids;
  if (_config.contains("ids") && _config["ids"].is_array())
    for (json::const_iterator it = _config["ids"].begin();
         it != _config["ids"].end(); ++it)
      if (it->is_number()) ids.push_back(it->get<int>());

  return [threshold, ids](const RecognizedItem &_item) {
    return (ids.empty() ||
            find(ids.begin(), ids.end(), _item.type) != ids.end()) &&
           _item.confidence >= threshold;
  };
}
