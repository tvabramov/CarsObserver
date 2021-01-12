#ifndef RECOGNIZERS_ABSTRACTRECOGNIZER_H
#define RECOGNIZERS_ABSTRACTRECOGNIZER_H

#include <list>
#include <opencv2/opencv.hpp>
#include <utility>

struct RecognizedItem {
  int type;
  double confidence;
  cv::Rect2d rect;

  RecognizedItem() : type(-1), confidence(0.0) {}
  RecognizedItem(int _type, double _confidence, const cv::Rect2d& _rect)
      : type(_type), confidence(_confidence), rect(_rect) {}
  RecognizedItem(int _type, double _confidence, cv::Rect2d&& _rect)
      : type(_type), confidence(_confidence), rect(std::move(_rect)) {}

  RecognizedItem(const RecognizedItem& _other) = default;

  RecognizedItem(RecognizedItem&& _other) noexcept
      : type(std::exchange(_other.type, -1)),
        confidence(std::exchange(_other.confidence, 0.0)),
        rect(std::move(_other.rect)) {}

  RecognizedItem& operator=(const RecognizedItem& _other) = default;

  RecognizedItem& operator=(RecognizedItem&& _other) noexcept {
    type = std::exchange(_other.type, -1);
    confidence = std::exchange(_other.confidence, 0.0);
    rect = std::move(_other.rect);

    return *this;
  }
};

class AbstractRecognizer {
 public:
  explicit AbstractRecognizer() {}
  virtual ~AbstractRecognizer() {}

  virtual std::list<RecognizedItem> recognize(const cv::Mat& _frame) = 0;
};

#endif  // RECOGNIZERS_ABSTRACTRECOGNIZER_H
