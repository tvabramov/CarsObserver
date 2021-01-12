#ifndef TRACKERFACTORY_H
#define TRACKERFACTORY_H

#include <nlohmann/json.hpp>

#include "recognizers/abstractrecognizer.h"
#include "tracker.h"
#include "trackers/abstracttracker.h"
#include "verifiers/abstractverifier.h"

class TrackerFactory {
 public:
  virtual ~TrackerFactory() {}

  static std::shared_ptr<Tracker> createTracker(const nlohmann::json &_config);

 private:
  explicit TrackerFactory() {}

  static std::unique_ptr<AbstractRecognizer> createRecognizer(
      const nlohmann::json &_config);
  static std::unique_ptr<AbstractTracker> createInternalTracker(
      const nlohmann::json &_config);
  static std::unique_ptr<AbstractVerifier> createVerifier(
      const nlohmann::json &_config);
  static AbstractVerifier::ItemFilterFunction createWeakFitFunc(
      const nlohmann::json &_config);
  static AbstractVerifier::ItemFilterFunction createStrongFitFunc(
      const nlohmann::json &_config);
};

#endif  // TRACKERFACTORY_H
