#include "crosscounter.h"

#include <boost/log/trivial.hpp>
#include <cassert>
#include <chrono>

#include "utilities.h"

using namespace cv;
using namespace std;
using namespace std::chrono;

namespace {

// Alg from http://algolist.ru/maths/geom/intersect/lineline2d.php
bool isLinesCrossInt(const Point2d &l1beg, const Point2d &l1end,
                     const Point2d &l2beg, const Point2d &l2end) {
  auto maxx1 = std::max(l1beg.x, l1end.x);
  auto maxy1 = std::max(l1beg.y, l1end.y);
  auto minx1 = std::min(l1beg.x, l1end.x);
  auto miny1 = std::min(l1beg.y, l1end.y);
  auto maxx2 = std::max(l2beg.x, l2end.x);
  auto maxy2 = std::max(l2beg.y, l2end.y);
  auto minx2 = std::min(l2beg.x, l2end.x);
  auto miny2 = std::min(l2beg.y, l2end.y);

  if (minx1 > maxx2 || maxx1 < minx2 || miny1 > maxy2 || maxy1 < miny2)
    return false;  // Момент, када линии имеют одну общую вершину...

  auto dx1 = l1end.x - l1beg.x;
  auto dy1 = l1end.y - l1beg.y;  // Длина проекций первой линии на ось x и y
  auto dx2 = l2end.x - l2beg.x;
  auto dy2 = l2end.y - l2beg.y;  // Длина проекций второй линии на ось x и y
  auto dxx = l1beg.x - l2beg.x;
  auto dyy = l1beg.y - l2beg.y;

  int div, mul;

  if ((div = (int)((double)dy2 * dx1 - (double)dx2 * dy1)) == 0)
    return false;  // Линии параллельны...
  if (div > 0) {
    if ((mul = (int)((double)dx1 * dyy - (double)dy1 * dxx)) < 0 || mul > div)
      return false;  // Первый отрезок пересекается за своими границами...
    if ((mul = (int)((double)dx2 * dyy - (double)dy2 * dxx)) < 0 || mul > div)
      return false;  // Второй отрезок пересекается за своими границами...
  }

  if ((mul = -(int)((double)dx1 * dyy - (double)dy1 * dxx)) < 0 || mul > -div)
    return false;  // Первый отрезок пересекается за своими границами...
  if ((mul = -(int)((double)dx2 * dyy - (double)dy2 * dxx)) < 0 || mul > -div)
    return false;  // Второй отрезок пересекается за своими границами...

  return true;
}

bool isLinesCross(const Point2d &l1beg, const Point2d &l1end,
                  const Point2d &l2beg, const Point2d &l2end) {
  // TODO: fix it
  return isLinesCrossInt(l1beg, l1end, l2beg, l2end) ||
         isLinesCrossInt(l2beg, l2end, l1beg, l1end);
}

void putTextBg(Mat &_frame, const string &_text, const Point &_point,
               int _fontFace, double _fontScale, const Scalar &_textColor,
               const Scalar &_bgColor, int _thickness = 1, int _lineType = 8,
               bool _bottomLeftOrigin = false) {
  int baseLine;
  auto textSize =
      getTextSize(_text, _fontFace, _fontScale, _thickness, &baseLine);

  int padding = 3;

  rectangle(
      _frame,
      cv::Rect(_point.x - padding, _point.y - textSize.height - padding,
               textSize.width + 2 * padding, textSize.height + 3 + 2 * padding),
      _bgColor, cv::FILLED);

  putText(_frame, _text, _point, _fontFace, _fontScale, _textColor, _thickness,
          _lineType, _bottomLeftOrigin);
}

}  // namespace

CrossCounter::CrossCounter(bool _debugScreenOutput, const Size &_debugVideoSize,
                           const vector<pair<Point2d, Point2d>> &_lines,
                           int _timeoutMs)
    : mDebugScreenOutput(_debugScreenOutput),
      mDebugVideoSize(_debugVideoSize),
      mLines(_lines),
      mTimeoutMs(_timeoutMs),
      mCrossCounts(vector<int>(mLines.size(), 0)) {
  // if (mDebugScreenOutput)
  //   namedWindow("CrossCounter", WINDOW_AUTOSIZE);
}

CrossCounter::~CrossCounter() {
  if (mDebugScreenOutput) destroyWindow("CrossCounter");
}

void CrossCounter::push(const list<TrackerOutput> &_input) {
  unique_lock<mutex> lck(mInputMutex);

  mInputData.insert(mInputData.end(), _input.begin(), _input.end());

  mHaveInput.notify_all();
}

void CrossCounter::push(list<TrackerOutput> &&_input) {
  unique_lock<mutex> lck(mInputMutex);

  mInputData.splice(mInputData.end(), move(_input));

  mHaveInput.notify_all();
}

void CrossCounter::doWork() {
  list<TrackerOutput> inputData;

  {
    unique_lock<mutex> lck(mInputMutex);

    while (mInputData.empty())
      mHaveInput.wait_for(lck, chrono::milliseconds(mTimeoutMs));

    inputData = move(mInputData);
  }

  while (!inputData.empty()) {
    auto d = inputData.front();
    inputData.pop_front();

    assert(!d.frame.empty());

    auto it_track = mCurrentTracks.begin();
    while (it_track != mCurrentTracks.end()) {
      auto it_d =
          find_if(d.items.begin(), d.items.end(), [it_track](const auto &el) {
            return el.trackId == it_track->trackId;
          });

      if (it_d == d.items.end()) {
        mCurrentTracks.erase(it_track++);  // Kill track
      } else {
        it_track->pushTrackedItem(*it_d);  // Append to tail
        while (it_track->tail.size() > (mDebugScreenOutput ? 30 : 1))
          it_track->tail.pop_front();

        d.items.erase(it_d);
        ++it_track;
      }
    }

    // New tracks
    for (const auto &el : d.items) mCurrentTracks.push_back(el);

    std::list<CrossEvent> ces;

    // Cross checking
    assert(mCrossCounts.size() == mLines.size());
    for (auto it_line = mLines.begin(); it_line != mLines.end(); ++it_line) {
      for (auto &t : mCurrentTracks)
        if (t.crosses.find(distance(mLines.begin(), it_line)) ==
                t.crosses.end() &&
            !t.tail.empty() &&
            isLinesCross(t.tail.back().point,
                         Point2d(t.rect.x + t.rect.width / 2,
                                 t.rect.y + t.rect.height / 2),
                         it_line->first, it_line->second)) {
          t.crosses.insert(distance(mLines.begin(), it_line));
          ++(mCrossCounts[distance(mLines.begin(), it_line)]);

          {
            auto p1 = t.tail.back().point;
            auto p2 = Point2d(t.rect.x + t.rect.width / 2,
                              t.rect.y + t.rect.height / 2);

            int xdir = 0;
            if (p2.x > p1.x)
              xdir = 1;
            else if (p2.x < p1.x)
              xdir = -1;

            int ydir = 0;
            if (p2.y > p1.y)
              ydir = 1;
            else if (p2.y < p1.y)
              ydir = -1;

            ces.push_back(CrossEvent(
                distance(mLines.begin(), it_line), t.trackId, d.timestamp,
                mCrossCounts[distance(mLines.begin(), it_line)], xdir, ydir));
          }

          BOOST_LOG_TRIVIAL(trace)
              << "Crosses count[" << distance(mLines.begin(), it_line)
              << "] = " << mCrossCounts[distance(mLines.begin(), it_line)];
        }
    }

    // Do smth with ces:
    ces.clear();

    // Optional debug output
    if (mDebugScreenOutput) {
      Mat frame = d.frame.clone();

      for (auto it_line = mLines.begin(); it_line != mLines.end(); ++it_line) {
        line(frame, it_line->first, it_line->second, Scalar(0, 0, 255), 2);
        putTextBg(
            frame,
            "L" + to_string(distance(mLines.begin(), it_line) + 1) + " [" +
                to_string(mCrossCounts[distance(mLines.begin(), it_line)]) +
                "]",
            Point(it_line->first.x + 10, it_line->first.y + 20),
            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), Scalar(40, 40, 40));
      }

      for (const auto &t : mCurrentTracks) {
        if (t.tail.size() > 1)
          for (auto it = t.tail.begin(); it != std::prev(t.tail.end()); ++it)
            line(frame, Point2d(it->point.x, it->point.y),
                 Point2d(std::next(it)->point.x, std::next(it)->point.y),
                 /*t.crossed ? Scalar(0, 0, 255) : */ Scalar(255, 0, 0), 1);

        for (auto it = t.tail.begin(); it != t.tail.end(); ++it)
          rectangle(frame, Rect2d(it->point.x - 2, it->point.y - 2, 5, 5),
                    /*t.crossed ? Scalar(0, 0, 255) : */
                    (it->verified ? Scalar(0, 255, 0) : Scalar(255, 0, 0)), 1);

        rectangle(frame, t.rect, /*t.crossed ? Scalar(0, 0, 255) :*/
                  (t.verified ? Scalar(0, 255, 0) : Scalar(255, 0, 0)), 2);

        putText(frame, "TID = " + to_string(t.trackId),
                Point(t.rect.x + 10, t.rect.y + 20), FONT_HERSHEY_SIMPLEX, 0.5,
                /*t.crossed ? Scalar(0, 0, 255) : */
                (t.verified ? Scalar(0, 255, 0) : Scalar(255, 0, 0)));
      }

      resize(frame, frame,
             mDebugVideoSize.empty() ? Size(frame.cols, frame.rows)
                                     : mDebugVideoSize);
      putText(frame, timeToStrWithMs(d.timestamp), Point(10, 20),
              FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0));

      {
        imshow("CrossCounter", frame);
        waitKey(1);
      }
    }
  }
}

list<CrossEvent> CrossCounter::pop() {
  unique_lock<mutex> lck(mOutputMutex);

  if (mOutputData.empty())
    mHaveOutput.wait_for(lck, chrono::milliseconds(mTimeoutMs));

  return move(mOutputData);
}
