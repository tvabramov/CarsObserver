#ifndef CAPTURERFACTORY_H
#define CAPTURERFACTORY_H

#include <memory>
#include <nlohmann/json.hpp>

#include "capturer.h"

class CapturerFactory {
 public:
  virtual ~CapturerFactory() {}

  static std::shared_ptr<Capturer> createCapturer(
      const nlohmann::json &_config);

 private:
  explicit CapturerFactory() {}
};

#endif  // CAPTURERFACTORY_H
