#ifndef CROSSCOUNTER_H
#define CROSSCOUNTER_H

#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "tracker.h"

struct VerifiedPoint {
  cv::Point2d point;
  bool verified;

  VerifiedPoint() {}
  VerifiedPoint(const cv::Point2d &_point, bool _verified)
      : point(_point), verified(_verified) {}
};

struct TailedItem {
  int trackId;
  std::list<VerifiedPoint> tail;  // Without last position.
  cv::Rect2d rect;                // Last position
  bool verified;                  // Last position verified
  std::set<int> crosses;  // lines numbers, which this TailedItem crossed

  TailedItem() : trackId(-1), verified(false) {}

  // TailedItem -> TailedItem

  TailedItem(const TailedItem &_other) = default;

  TailedItem &operator=(const TailedItem &_other) = default;

  TailedItem(TailedItem &&_other) noexcept
      : trackId(std::exchange(_other.trackId, -1)),
        tail(std::move(_other.tail)),
        rect(std::move(_other.rect)),
        verified(std::exchange(_other.verified, false)),
        crosses(std::move(_other.crosses)) {}

  TailedItem &operator=(TailedItem &&_other) noexcept {
    trackId = std::exchange(_other.trackId, -1);
    tail = std::move(_other.tail);
    rect = std::move(_other.rect);
    verified = std::exchange(_other.verified, false);
    crosses = std::move(_other.crosses);

    return *this;
  }

  // TrackedItem -> TailedItem

  TailedItem(const TrackedItem &_other)
      : trackId(_other.trackId),
        rect(_other.rect),
        verified(_other.recType && _other.recConfidence) {}

  TailedItem &operator=(const TrackedItem &_other) {
    trackId = _other.trackId;
    rect = _other.rect;
    verified = _other.recType && _other.recConfidence;
    tail.clear();
    crosses.clear();

    return *this;
  }

  TailedItem(TrackedItem &&_trackedItem)
      : trackId(std::exchange(_trackedItem.trackId, -1)),
        rect(std::move(_trackedItem.rect)),
        verified(_trackedItem.recType && _trackedItem.recConfidence) {}

  TailedItem &operator=(TrackedItem &&_trackedItem) {
    trackId = std::exchange(_trackedItem.trackId, -1);
    rect = std::move(_trackedItem.rect);
    verified = _trackedItem.recType && _trackedItem.recConfidence;
    tail.clear();
    crosses.clear();

    return *this;
  }

  // Pushing TrackedItem

  void pushTrackedItem(const TrackedItem &_trackedItem) {
    assert(trackId == _trackedItem.trackId);
    assert(!rect.empty() && !_trackedItem.rect.empty());

    tail.push_back(VerifiedPoint(
        cv::Point2d(rect.x + rect.width / 2, rect.y + rect.height / 2),
        verified));
    rect = _trackedItem.rect;
    verified = _trackedItem.recType && _trackedItem.recConfidence;
  }

  void pushTrackedItem(TrackedItem &&_trackedItem) {
    assert(trackId == _trackedItem.trackId);
    assert(!rect.empty() && !_trackedItem.rect.empty());

    tail.push_back(VerifiedPoint(
        cv::Point2d(rect.x + rect.width / 2, rect.y + rect.height / 2),
        verified));
    rect = std::move(_trackedItem.rect);
    verified = _trackedItem.recType && _trackedItem.recConfidence;
  }
};

struct CrossEvent {
  int line_id, track_id;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
  int crosses;
  int xdir, ydir;  // -1, 0 or 1

  CrossEvent()
      : line_id(-1),
        track_id(-1),
        timestamp(std::chrono::system_clock::now()),
        crosses(0),
        xdir(0),
        ydir(0) {}
  CrossEvent(int _line_id, int _track_id,
             std::chrono::time_point<std::chrono::system_clock> _timestamp,
             int _crosses, int _xdir, int _ydir)
      : line_id(_line_id),
        track_id(_track_id),
        timestamp(_timestamp),
        crosses(_crosses),
        xdir(_xdir),
        ydir(_ydir) {}
};

class CrossCounter {
 public:
  explicit CrossCounter(
      bool _debugScreenOutput, const cv::Size &_debugVideoSize,
      const std::vector<std::pair<cv::Point2d, cv::Point2d>> &_lines,
      int _timeoutMs);
  virtual ~CrossCounter();

  void push(const std::list<TrackerOutput> &_input);
  void push(std::list<TrackerOutput> &&_input);

  void doWork();

  std::list<CrossEvent> pop();

 protected:
  bool mDebugScreenOutput;
  cv::Size mDebugVideoSize;
  std::vector<std::pair<cv::Point2d, cv::Point2d>> mLines;
  int mTimeoutMs;

  std::list<TrackerOutput> mInputData;
  std::mutex mInputMutex;
  std::condition_variable mHaveInput;

  std::list<TailedItem> mCurrentTracks;
  std::vector<int> mCrossCounts;

  std::list<CrossEvent> mOutputData;
  std::mutex mOutputMutex;
  std::condition_variable mHaveOutput;
};

#endif  // CROSSCOUNTER_H
