#ifndef VERIFIERS_ABSTRACTVERIFIER_H
#define VERIFIERS_ABSTRACTVERIFIER_H

#include <functional>
#include <list>

#include "trackers/abstracttracker.h"

class AbstractVerifier {
 public:
  using ItemFilterFunction = std::function<bool(const RecognizedItem&)>;

  explicit AbstractVerifier() {}
  virtual ~AbstractVerifier() {}

  virtual void verify(std::list<TrackedItem>& _t,
                      const std::list<RecognizedItem>& _r,
                      ItemFilterFunction _weakFitFunc,
                      ItemFilterFunction _strongFitFunc) = 0;
};

#endif  // VERIFIERS_ABSTRACTVERIFIER_H
