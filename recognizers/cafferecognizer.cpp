#include "recognizers/cafferecognizer.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace dnn;
using namespace std;

CaffeRecognizer::CaffeRecognizer(const string &_prototxtPath,
                                 const string &_caffemodelPath,
                                 double _scaleFactor, const Size &_size,
                                 const Scalar &_mean, bool _swapRB, bool _crop,
                                 int _ddepth)
    : AbstractRecognizer(),
      mScaleFactor(_scaleFactor),
      mSize(_size),
      mMean(_mean),
      mSwapRB(_swapRB),
      mCrop(_crop),
      mDdepth(_ddepth) {
  // TODO: if it causes an exception?
  mNet = readNetFromCaffe(_prototxtPath, _caffemodelPath);
}

list<RecognizedItem> CaffeRecognizer::recognize(const Mat &_frame) {
  // Code from
  // https://web-answers.ru/c/opencv-c-hwnd2mat-skrinshot-gt-blobfromimage.html
  Mat blob = blobFromImage(_frame, mScaleFactor, mSize, mMean, mSwapRB, mCrop,
                           mDdepth);

  mNet.setInput(blob);
  Mat detections = mNet.forward();
  Mat detectionMat(detections.size[2], detections.size[3], mDdepth,
                   detections.ptr<float>());
  list<RecognizedItem> items;
  for (int i = 0; i < detectionMat.rows; i++) {
    float confidence = detectionMat.at<float>(i, 2);

    int idx = static_cast<int>(detectionMat.at<float>(i, 1));
    int xLeftBottom =
        static_cast<int>(detectionMat.at<float>(i, 3) * _frame.cols);
    int yLeftBottom =
        static_cast<int>(detectionMat.at<float>(i, 4) * _frame.rows);
    int xRightTop =
        static_cast<int>(detectionMat.at<float>(i, 5) * _frame.cols);
    int yRightTop =
        static_cast<int>(detectionMat.at<float>(i, 6) * _frame.rows);

    Rect2d rect(xLeftBottom, yLeftBottom, xRightTop - xLeftBottom,
                yRightTop - yLeftBottom);

    // boundary checking
    rect = rect & Rect2d(0, 0, _frame.cols, _frame.rows);

    if (!rect.empty()) items.push_back(RecognizedItem(idx, confidence, rect));
  }

  return items;
}
