#ifndef RECOGNIZERFACTORY_H
#define RECOGNIZERFACTORY_H

#include <nlohmann/json.hpp>
#include <memory>

#include "recognizer.h"
#include "recognizers/abstractrecognizer.h"

class RecognizerFactory {
 public:
  virtual ~RecognizerFactory() {}

  static std::shared_ptr<Recognizer> createRecognizer(
      const nlohmann::json &_config);

 private:
  RecognizerFactory() {}

  static std::unique_ptr<AbstractRecognizer> createInternalRecognizer(
      const nlohmann::json &_config);
};

#endif  // RECOGNIZERFACTORY_H
