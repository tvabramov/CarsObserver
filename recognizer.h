#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <utility>

#include "capturer.h"
#include "recognizers/abstractrecognizer.h"

struct RecognizerOutput {
  cv::Mat frame;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
  std::list<RecognizedItem> items;
  bool recognitionDone;

  RecognizerOutput() : recognitionDone(false) {}
  RecognizerOutput(
      const cv::Mat &_frame,
      const std::chrono::time_point<std::chrono::system_clock> &_timestamp,
      const std::list<RecognizedItem> &_items, bool _recognitionDone)
      : frame(_frame),
        timestamp(_timestamp),
        items(_items),
        recognitionDone(_recognitionDone) {}
  RecognizerOutput(
      cv::Mat &&_frame,
      std::chrono::time_point<std::chrono::system_clock> &&_timestamp,
      std::list<RecognizedItem> &&_items, bool _recognitionDone)
      : frame(std::move(_frame)),
        timestamp(std::move(_timestamp)),
        items(std::move(_items)),
        recognitionDone(_recognitionDone) {}

  // RecognizerOutput -> RecognizerOutput

  RecognizerOutput(const RecognizerOutput &_other) = default;

  RecognizerOutput &operator=(const RecognizerOutput &_other) = default;

  RecognizerOutput(RecognizerOutput &&_other) noexcept
      : frame(std::move(_other.frame)),
        timestamp(std::move(_other.timestamp)),
        items(std::move(_other.items)),
        recognitionDone(std::exchange(_other.recognitionDone, false)) {}

  RecognizerOutput &operator=(RecognizerOutput &&_other) noexcept {
    frame = std::move(_other.frame);
    timestamp = std::move(_other.timestamp);
    items = std::move(_other.items);
    recognitionDone = std::exchange(_other.recognitionDone, false);

    return *this;
  }
};

class Recognizer {
 public:
  explicit Recognizer(std::unique_ptr<AbstractRecognizer> _recognizer,
                      int _recognitionDelayMs, int _maxPending, int _timeoutMs);

  void push(const std::list<CapturerOutput> &_input);
  void push(std::list<CapturerOutput> &&_input);

  std::list<RecognizerOutput> pop();

  void doWork();

 protected:
  std::list<CapturerOutput> mInputData;
  std::list<RecognizerOutput> mOutputData;
  std::mutex mInputMutex, mOutputMutex;
  std::condition_variable mHaveInput, mHaveOutput;

  std::unique_ptr<AbstractRecognizer> mRecognizer;
  int mRecognitionDelayMs;
  int mMaxPending;
  int mTimeoutMs;

  std::chrono::time_point<std::chrono::system_clock> mLastRec;

  int timeDiffMs(
      const std::chrono::time_point<std::chrono::system_clock> &_begin,
      const std::chrono::time_point<std::chrono::system_clock> &_end);
};

#endif  // RECOGNIZER_H
