#ifndef CROSSCOUNTERFACTORY_H
#define CROSSCOUNTERFACTORY_H

#include <nlohmann/json.hpp>

#include "crosscounter.h"

class CrossCounterFactory {
 public:
  virtual ~CrossCounterFactory() {}

  static std::shared_ptr<CrossCounter> createCrossCounter(
      const nlohmann::json &_config);

 private:
  explicit CrossCounterFactory() {}
};

#endif  // CROSSCOUNTERFACTORY_H
