#include "tracker.h"

#include <boost/log/trivial.hpp>

using namespace cv;
using namespace std;

Tracker::Tracker(unique_ptr<AbstractTracker> _tracker,
                 unique_ptr<AbstractVerifier> _verifier,
                 AbstractVerifier::ItemFilterFunction _weakFitFunc,
                 AbstractVerifier::ItemFilterFunction _strongFitFunc,
                 int _maxPending, int _timeoutMs)
    : mTracker(move(_tracker)),
      mVerifier(move(_verifier)),
      mWeakFitFunc(move(_weakFitFunc)),
      mStrongFitFunc(move(_strongFitFunc)),
      mMaxPending(_maxPending),
      mTimeoutMs(_timeoutMs),
      mCounter(0) {}

void Tracker::push(const list<RecognizerOutput> &_input) {
  unique_lock<mutex> lck(mInputMutex);

  mInputData.insert(mInputData.end(), _input.begin(), _input.end());

  mHaveInput.notify_all();
}

void Tracker::push(list<RecognizerOutput> &&_input) {
  unique_lock<mutex> lck(mInputMutex);

  mInputData.splice(mInputData.end(), move(_input));

  mHaveInput.notify_all();
}

list<TrackerOutput> Tracker::pop() {
  unique_lock<mutex> lck(mOutputMutex);

  if (mOutputData.empty())
    mHaveOutput.wait_for(lck, chrono::milliseconds(mTimeoutMs));

  return move(mOutputData);
}

void Tracker::doWork() {
  list<RecognizerOutput> inputData;

  {
    unique_lock<mutex> lck(mInputMutex);

    while (mInputData.empty())
      mHaveInput.wait_for(lck, chrono::milliseconds(mTimeoutMs));

    inputData = move(mInputData);
  }

  // Pre-analyze which frames process
  int peek = (mMaxPending >= 1 && inputData.size() >= 5)
                 ? inputData.size() / mMaxPending - 1
                 : 0;

  for (auto it_d = inputData.begin(); it_d != inputData.end(); ++it_d) {
    assert(!it_d->frame.empty());

    if (it_d->recognitionDone) {
      auto t0 = chrono::system_clock::now();
      auto t_items = mTracker->track(it_d->frame);
      auto dt = chrono::duration_cast<chrono::milliseconds>(
                    chrono::system_clock::now() - t0)
                    .count();
      BOOST_LOG_TRIVIAL(trace) << "Tracker: Track time = " << dt << " ms";

      t0 = chrono::system_clock::now();
      mVerifier->verify(t_items, it_d->items, mWeakFitFunc, mStrongFitFunc);
      dt = chrono::duration_cast<chrono::milliseconds>(
               chrono::system_clock::now() - t0)
               .count();
      BOOST_LOG_TRIVIAL(trace) << "Tracker: Verify time = " << dt << " ms";

      for_each(t_items.begin(), t_items.end(), [this](auto &_item) {
        if (_item.trackId == -1) _item.trackId = mCounter++;
      });

      mTracker->reset(it_d->frame, t_items);

      unique_lock<mutex> lck(mOutputMutex);

      mOutputData.push_back(TrackerOutput(
          move(it_d->frame), move(it_d->timestamp), move(t_items)));

      BOOST_LOG_TRIVIAL(trace)
          << "Tracker: Frame has been tracked and verified";
    } else {
      auto t0 = chrono::system_clock::now();
      auto t_items = mTracker->track(it_d->frame);
      auto dt = chrono::duration_cast<chrono::milliseconds>(
                    chrono::system_clock::now() - t0)
                    .count();
      BOOST_LOG_TRIVIAL(trace) << "Tracker: Track time = " << dt << " ms";

      mOutputData.push_back(TrackerOutput(
          move(it_d->frame), move(it_d->timestamp), move(t_items)));

      BOOST_LOG_TRIVIAL(trace) << "Tracker: Frame has been only tracked";
    }

    for (int p = 0; p < peek; ++p)
      if (next(it_d) != inputData.end()) {
        // TODO: maybe not peek "recognitionDone" frames.
        ++it_d;
        BOOST_LOG_TRIVIAL(trace) << "Tracker: Frame has been peeked";
      } else {
        break;
      }
  }

  mHaveOutput.notify_all();
}
