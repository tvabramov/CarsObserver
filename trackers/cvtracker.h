#ifndef TRACKERS_CVTRACKER_H
#define TRACKERS_CVTRACKER_H

#include <functional>
#include <opencv2/tracking.hpp>

#include "trackers/abstracttracker.h"

class CvTracker : public AbstractTracker {
 public:
  using CvTrackerCreateFunction = std::function<cv::Ptr<cv::Tracker>()>;

  explicit CvTracker(CvTrackerCreateFunction _createFunc, int _frameSizeX,
                     int _frameSizeY);

  virtual std::list<TrackedItem> track(const cv::Mat &_frame) override;

  virtual void reset(const cv::Mat &_frame,
                     const std::list<TrackedItem> &_items) override;

 protected:
  CvTrackerCreateFunction mCreateFunction;
  int mFrameSizeX, mFrameSizeY;
  std::list<cv::Ptr<cv::Tracker>> mCvTrackers;
  std::list<TrackedItem> mTrackedItems;
};

#endif  // TRACKERS_CVTRACKER_H
