#include "verifiers/hunverifier.h"

#include <algorithm>
#include <limits>
#include <set>
#include <vector>

#include "verifiers/hungarian.h"

using namespace cv;
using namespace std;

namespace {

// Computes IOU between two bounding boxes
double getIOU2(const cv::Rect2d &bb_test, const cv::Rect2d &bb_gt) {
  double in = (bb_test & bb_gt).area();
  double un = bb_test.area() + bb_gt.area() - in;

  return un < std::numeric_limits<double>::epsilon() ? 0.0 : in / un;
}

}  // namespace

HunVerifier::HunVerifier(double _threshold, int _maxRecFails)
    : AbstractVerifier(), mThreshold(_threshold), mMaxRecFails(_maxRecFails) {}

void HunVerifier::verify(list<TrackedItem> &_t, const list<RecognizedItem> &_r,
                         ItemFilterFunction _weakFitFunc,
                         ItemFilterFunction _strongFitFunc) {
  vector<vector<double>> iouMatrix(_t.size(), vector<double>(_r.size(), 0.0));

  for (auto it_t = _t.begin(); it_t != _t.end(); ++it_t)
    for (auto it_r = _r.begin(); it_r != _r.end(); ++it_r)
      iouMatrix[distance(_t.begin(), it_t)][distance(_r.begin(), it_r)] =
          _weakFitFunc(*it_r) ? 1.0 - getIOU2(it_t->rect, it_r->rect) : 1.0;

  // solve the assignment problem using hungarian algorithm.
  // the resulting assignment is [track(prediction) : detection], with
  // len=preNum
  vector<int> assignment;
  Hungarian::Solve(iouMatrix, assignment);

  set<int> unmatchedTrajectories;
  set<int> unmatchedDetections;
  set<int> allItems;
  set<int> matchedItems;

  if (_r.size() > _t.size())  // there are unmatched detections
  {
    for (size_t i = 0; i < _r.size(); ++i) allItems.insert(i);

    for (size_t i = 0; i < _t.size(); ++i) matchedItems.insert(assignment[i]);

    set_difference(allItems.begin(), allItems.end(), matchedItems.begin(),
                   matchedItems.end(),
                   insert_iterator<set<int>>(unmatchedDetections,
                                             unmatchedDetections.begin()));
  } else if (_r.size() <
             _t.size())  // there are unmatched trajectory/predictions
  {
    for (size_t i = 0; i < _t.size(); ++i)
      if (assignment[i] ==
          -1)  // unassigned label will be set as -1 in the assignment algorithm
        unmatchedTrajectories.insert(i);
  }

  // filter out matched with low IOU
  vector<cv::Point> matchedPairs;
  for (size_t i = 0; i < _t.size(); ++i) {
    if (assignment[i] == -1)  // pass over invalid values
      continue;

    if (1.0 - iouMatrix[i][assignment[i]] < mThreshold) {
      unmatchedTrajectories.insert(i);
      unmatchedDetections.insert(assignment[i]);
    } else
      matchedPairs.push_back(cv::Point(i, assignment[i]));
  }

  vector<RecognizedItem> bufr(_r.begin(), _r.end());

  auto it_t = _t.begin();
  int t_index = 0;
  while (it_t != _t.end()) {
    auto p = find_if(matchedPairs.begin(), matchedPairs.end(),
                     [t_index](const auto &el) { return el.x == t_index; });

    if (p != matchedPairs.end() && p->y >= 0 &&
        p->y < static_cast<int>(bufr.size()) && _weakFitFunc(bufr[p->y])) {
      it_t->recType = bufr[p->y].type;
      it_t->recConfidence = bufr[p->y].confidence;
      it_t->rect = bufr[p->y].rect;
      it_t->recFailsCount = 0;

      ++it_t;
    } else if (it_t->recFailsCount < mMaxRecFails) {
      // std::cout << "stay for tid = " << it_t->trackId << std::endl;
      ++(it_t->recFailsCount);
      ++it_t;
    } else {
      _t.erase(it_t++);
    }

    ++t_index;
  }

  // New items
  for (const int &rind : unmatchedDetections)
    if (rind >= 0 && rind < static_cast<int>(bufr.size()) &&
        _strongFitFunc(bufr[rind]))
      _t.push_back(bufr[rind]);
}
