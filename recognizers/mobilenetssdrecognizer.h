#ifndef RECOGNIZERS_MOBILENETSSDRECOGNIZER_H
#define RECOGNIZERS_MOBILENETSSDRECOGNIZER_H

#include "recognizers/cafferecognizer.h"

class MobileNetSSDRecognizer : public CaffeRecognizer {
 public:
  enum class ITEMCLASSES {
    BACKGROUND,
    AEROPLANE,
    BICYCLE,
    BIRD,
    BOAT,
    BOTTLE,
    BUS,
    CAR,
    CAT,
    CHAIR,
    COW,
    DININGTABLE,
    DOG,
    HORSE,
    MOTORBIKE,
    PERSON,
    POTTEDPLANT,
    SHEEP,
    SOFA,
    TRAIN,
    TVMONITOR,
    INVALID,
    COUNT
  };

  explicit MobileNetSSDRecognizer();
};

#endif  // RECOGNIZERS_MOBILENETSSDRECOGNIZER_H
