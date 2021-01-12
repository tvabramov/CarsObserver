#ifndef RECOGNIZERS_FACERECOGNIZER_H
#define RECOGNIZERS_FACERECOGNIZER_H

#include "recognizers/cafferecognizer.h"

class FaceRecognizer : public CaffeRecognizer {
 public:
  explicit FaceRecognizer();
};

#endif  // RECOGNIZERS_FACERECOGNIZER_H
