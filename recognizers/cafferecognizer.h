#ifndef RECOGNIZERS_CAFFERECOGNIZER_H
#define RECOGNIZERS_CAFFERECOGNIZER_H

#include <opencv2/dnn.hpp>
#include <string>

#include "recognizers/abstractrecognizer.h"

class CaffeRecognizer : public AbstractRecognizer {
 public:
  explicit CaffeRecognizer(const std::string &_prototxtPath,
                           const std::string &_caffemodelPath,
                           double _scaleFactor, const cv::Size &_size,
                           const cv::Scalar &_mean, bool _swapRB, bool _crop,
                           int _ddepth);

  virtual std::list<RecognizedItem> recognize(const cv::Mat &_frame) override;

 protected:
  double mScaleFactor;
  cv::Size mSize;
  cv::Scalar mMean;
  bool mSwapRB, mCrop;
  int mDdepth;
  cv::dnn::Net mNet;
};

#endif  // RECOGNIZERS_CAFFERECOGNIZER_H
