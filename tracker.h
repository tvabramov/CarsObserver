#ifndef TRACKER_H
#define TRACKER_H

#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <utility>

#include "recognizer.h"
#include "recognizers/abstractrecognizer.h"
#include "trackers/abstracttracker.h"
#include "verifiers/abstractverifier.h"

struct TrackerOutput {
  cv::Mat frame;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
  std::list<TrackedItem> items;

  TrackerOutput() {}
  TrackerOutput(
      const cv::Mat &_frame,
      const std::chrono::time_point<std::chrono::system_clock> &_timestamp,
      const std::list<TrackedItem> &_items)
      : frame(_frame), timestamp(_timestamp), items(_items) {}
  TrackerOutput(cv::Mat &&_frame,
                std::chrono::time_point<std::chrono::system_clock> &&_timestamp,
                std::list<TrackedItem> &&_items)
      : frame(std::move(_frame)),
        timestamp(std::move(_timestamp)),
        items(std::move(_items)) {}

  // TrackerOutput -> TrackerOutput

  TrackerOutput(const TrackerOutput &_other) = default;

  TrackerOutput &operator=(const TrackerOutput &_other) = default;

  TrackerOutput(TrackerOutput &&_other) noexcept
      : frame(std::move(_other.frame)),
        timestamp(std::move(_other.timestamp)),
        items(std::move(_other.items)) {}

  TrackerOutput &operator=(TrackerOutput &&_other) noexcept {
    frame = std::move(_other.frame);
    timestamp = std::move(_other.timestamp);
    items = std::move(_other.items);

    return *this;
  }
};

class Tracker {
 public:
  explicit Tracker(std::unique_ptr<AbstractTracker> _tracker,
                   std::unique_ptr<AbstractVerifier> _verifier,
                   AbstractVerifier::ItemFilterFunction _weakFitFunc,
                   AbstractVerifier::ItemFilterFunction _strongFitFunc,
                   int _maxPending, int _timeoutMs);

  void push(const std::list<RecognizerOutput> &_input);
  void push(std::list<RecognizerOutput> &&_input);

  std::list<TrackerOutput> pop();

  void doWork();

 protected:
  std::list<RecognizerOutput> mInputData;
  std::list<TrackerOutput> mOutputData;
  std::mutex mInputMutex, mOutputMutex;
  std::condition_variable mHaveInput, mHaveOutput;

  std::unique_ptr<AbstractTracker> mTracker;
  std::unique_ptr<AbstractVerifier> mVerifier;
  AbstractVerifier::ItemFilterFunction mWeakFitFunc;
  AbstractVerifier::ItemFilterFunction mStrongFitFunc;

  int mMaxPending;
  int mTimeoutMs;
  int mCounter;
};

#endif  // TRACKER_H
