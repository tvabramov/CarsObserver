#ifndef TRACKERS_ABSTRACTTRACKER_H
#define TRACKERS_ABSTRACTTRACKER_H

#include <boost/optional.hpp>
#include <list>
#include <utility>

#include "recognizers/abstractrecognizer.h"

struct TrackedItem {
  int trackId;
  boost::optional<int>
      recType;  // Defined only if confirmed by recognizing algorithm
  boost::optional<double>
      recConfidence;  // Defined only if confirmed by recognizing algorithm
  cv::Rect2d rect;
  int recFailsCount;  // How many times recognition algorithm does not confirm
                      // presence

  TrackedItem() : trackId(-1), recFailsCount(0) {}
  TrackedItem(int _trackId, int _recType, double _recConfidence,
              const cv::Rect2d &_rect, int _recFailsCount)
      : trackId(_trackId),
        recType(_recType),
        recConfidence(_recConfidence),
        rect(_rect),
        recFailsCount(_recFailsCount) {}
  TrackedItem(int _trackId, int _recType, double _recConfidence,
              cv::Rect2d &&_rect, int _recFailsCount)
      : trackId(_trackId),
        recType(_recType),
        recConfidence(_recConfidence),
        rect(std::move(_rect)),
        recFailsCount(_recFailsCount) {}

  // TrackedItem -> TrackedItem

  TrackedItem(const TrackedItem &_other) = default;

  TrackedItem &operator=(const TrackedItem &_other) = default;

  TrackedItem(TrackedItem &&_other) noexcept
      : trackId(std::exchange(_other.trackId, -1)),
        recType(std::move(_other.recType)),
        recConfidence(std::move(_other.recConfidence)),
        rect(std::move(_other.rect)),
        recFailsCount(std::exchange(_other.recFailsCount, 0)) {}

  TrackedItem &operator=(TrackedItem &&_other) noexcept {
    trackId = std::exchange(_other.trackId, -1);
    recType = std::move(_other.recType);
    recConfidence = std::move(_other.recConfidence);
    rect = std::move(_other.rect);
    recFailsCount = std::exchange(_other.recFailsCount, 0);

    return *this;
  }

  // RecognizedItem -> TrackedItem

  TrackedItem(const RecognizedItem &_recItem)
      : trackId(-1),
        recType(_recItem.type),
        recConfidence(_recItem.confidence),
        rect(_recItem.rect),
        recFailsCount(0) {}

  TrackedItem &operator=(const RecognizedItem &_recItem) {
    trackId = -1;
    recType = _recItem.type;
    recConfidence = std::move(_recItem.confidence);
    rect = std::move(_recItem.rect);
    recFailsCount = 0;

    return *this;
  }

  TrackedItem(RecognizedItem &&_recItem)
      : trackId(-1),
        recType(std::move(_recItem.type)),
        recConfidence(std::move(_recItem.confidence)),
        rect(std::move(_recItem.rect)),
        recFailsCount(0) {}

  TrackedItem &operator=(RecognizedItem &&_recItem) {
    trackId = -1;
    recType = std::move(_recItem.type);
    recConfidence = std::move(_recItem.confidence);
    rect = std::move(_recItem.rect);
    recFailsCount = 0;

    return *this;
  }
};

class AbstractTracker {
 public:
  explicit AbstractTracker() {}
  virtual ~AbstractTracker() {}

  virtual std::list<TrackedItem> track(const cv::Mat &_frame) = 0;

  virtual void reset(const cv::Mat &_frame,
                     const std::list<TrackedItem> &_items) = 0;
};

#endif  // TRACKERS_ABSTRACTTRACKER_H
