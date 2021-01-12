#include "trackers/cvtracker.h"

#include <boost/log/trivial.hpp>
#include <cassert>

using namespace cv;
using namespace std;

CvTracker::CvTracker(CvTrackerCreateFunction _createFunc, int _frameSizeX,
                     int _frameSizeY)
    : AbstractTracker(),
      mCreateFunction(_createFunc),
      mFrameSizeX(_frameSizeX),
      mFrameSizeY(_frameSizeY) {}

list<TrackedItem> CvTracker::track(const Mat &_frame) {
  Mat bufFrame;
  cvtColor(_frame, bufFrame, cv::COLOR_BGR2GRAY);
  resize(bufFrame, bufFrame, cv::Size(mFrameSizeX, mFrameSizeY));

  double scaleX =
      static_cast<double>(_frame.cols) / static_cast<double>(mFrameSizeX);
  double scaleY =
      static_cast<double>(_frame.rows) / static_cast<double>(mFrameSizeY);

  // For mTrackedItems and mCvTrackers have same objects
  // (This is antipattern), we must ensure that are exactly same
  assert(mTrackedItems.size() == mCvTrackers.size());

  auto it_t = mCvTrackers.begin();
  auto it_i = mTrackedItems.begin();

  while (it_t != mCvTrackers.end() && it_i != mTrackedItems.end()) {
    Rect2d bbox;
    // If rect is small, it failed
    if (it_i->rect.width > 10 && it_i->rect.height > 10 &&
        (*it_t)->update(bufFrame, bbox)) {
      it_i->rect = Rect2d(bbox.x * scaleX, bbox.y * scaleY, bbox.width * scaleX,
                          bbox.height * scaleY);
      it_i->recType = boost::none;
      it_i->recConfidence = boost::none;

      // boundary checking
      it_i->rect = it_i->rect & Rect2d(0, 0, _frame.cols, _frame.rows);

      if (!it_i->rect.empty()) {
        // All is ok
        ++it_t;
        ++it_i;
      } else {
        // Invalid rect. Delete it
        mCvTrackers.erase(it_t++);
        mTrackedItems.erase(it_i++);

        BOOST_LOG_TRIVIAL(trace) << "Tracker item has been lost and removed";
      }
    } else {
      mCvTrackers.erase(it_t++);
      mTrackedItems.erase(it_i++);

      BOOST_LOG_TRIVIAL(trace) << "Tracker item has been lost and removed";
    }
  }

  return mTrackedItems;
}

void CvTracker::reset(const Mat &_frame, const list<TrackedItem> &_items) {
  mCvTrackers.clear();
  mTrackedItems.clear();

  Mat bufFrame;
  cvtColor(_frame, bufFrame, cv::COLOR_BGR2GRAY);
  resize(bufFrame, bufFrame, cv::Size(mFrameSizeX, mFrameSizeY));

  double scaleX =
      static_cast<double>(_frame.cols) / static_cast<double>(mFrameSizeX);
  double scaleY =
      static_cast<double>(_frame.rows) / static_cast<double>(mFrameSizeY);

  for (const auto &item : _items) {
    Rect2d bufRect(item.rect.x / scaleX, item.rect.y / scaleY,
                   item.rect.width / scaleX, item.rect.height / scaleY);

    auto t = mCreateFunction();

    // If rect is small, it failed
    if (bufRect.width > 2.0 && bufRect.height > 2.0 && t &&
        t->init(bufFrame, bufRect)) {
      mCvTrackers.push_back(t);
      mTrackedItems.push_back(item);
    } else {
      BOOST_LOG_TRIVIAL(fatal) << "Can not add item";
    }
  }
}
