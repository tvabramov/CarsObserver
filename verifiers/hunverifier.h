#ifndef VERIFIERS_HUNVERIFIER_H
#define VERIFIERS_HUNVERIFIER_H

#include <boost/log/core.hpp>

#include "verifiers/abstractverifier.h"

class HunVerifier : public AbstractVerifier {
 public:
  explicit HunVerifier(double _threshold, int _maxRecFails);

  virtual void verify(std::list<TrackedItem>& _t,
                      const std::list<RecognizedItem>& _r,
                      ItemFilterFunction _weakFitFunc,
                      ItemFilterFunction _strongFitFunc) override;

 private:
  double mThreshold;
  int mMaxRecFails;
};

#endif  // VERIFIERS_HUNVERIFIER_H
