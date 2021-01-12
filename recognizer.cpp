#include "recognizer.h"

#include <boost/log/trivial.hpp>

using namespace cv;
using namespace std;
using namespace std::chrono;

Recognizer::Recognizer(unique_ptr<AbstractRecognizer> _recognizer,
                       int _recognitionDelayMs, int _maxPending, int _timeoutMs)
    : mRecognizer(move(_recognizer)),
      mRecognitionDelayMs(_recognitionDelayMs),
      mMaxPending(_maxPending),
      mTimeoutMs(_timeoutMs),
      mLastRec(chrono::system_clock::now()) {}

void Recognizer::push(const list<CapturerOutput> &_input) {
  unique_lock<mutex> lck(mInputMutex);

  mInputData.insert(mInputData.end(), _input.begin(), _input.end());

  mHaveInput.notify_all();
}

void Recognizer::push(list<CapturerOutput> &&_input) {
  unique_lock<mutex> lck(mInputMutex);

  mInputData.splice(mInputData.end(), move(_input));

  mHaveInput.notify_all();
}

list<RecognizerOutput> Recognizer::pop() {
  unique_lock<mutex> lck(mOutputMutex);

  if (mOutputData.empty())
    mHaveOutput.wait_for(lck, chrono::milliseconds(mTimeoutMs));

  return move(mOutputData);
}

void Recognizer::doWork() {
  list<CapturerOutput> inputData;

  {
    unique_lock<mutex> lck(mInputMutex);

    while (mInputData.empty())
      mHaveInput.wait_for(lck, chrono::milliseconds(mTimeoutMs));

    inputData = move(mInputData);
  }

  bool recognitionDone = false;  // Do recognition only for one frame

  for (auto it_d = inputData.begin(); it_d != inputData.end(); ++it_d) {
    assert(!it_d->frame.empty());

    if (!recognitionDone &&
        timeDiffMs(mLastRec, it_d->timestamp) >= mRecognitionDelayMs) {
      mLastRec = it_d->timestamp;

      auto t0 = chrono::system_clock::now();
      auto r_items = mRecognizer->recognize(it_d->frame);
      auto dt = chrono::duration_cast<chrono::milliseconds>(
                    chrono::system_clock::now() - t0)
                    .count();
      BOOST_LOG_TRIVIAL(trace)
          << "Recognizer: Recognize time = " << dt << " ms";

      unique_lock<mutex> lck(mOutputMutex);

      mOutputData.push_back(RecognizerOutput(
          move(it_d->frame), move(it_d->timestamp), move(r_items), true));

      recognitionDone = true;

      BOOST_LOG_TRIVIAL(trace) << "Recognizer: Frame has been recognized";
    } else {
      unique_lock<mutex> lck(mOutputMutex);

      mOutputData.push_back(RecognizerOutput(move(it_d->frame),
                                             move(it_d->timestamp),
                                             list<RecognizedItem>(), false));

      BOOST_LOG_TRIVIAL(trace) << "Recognizer: Frame has been peeked";
    }
  }

  mHaveOutput.notify_all();
}

int Recognizer::timeDiffMs(const time_point<system_clock> &_begin,
                           const time_point<system_clock> &_end) {
  milliseconds b = duration_cast<milliseconds>(_begin.time_since_epoch());
  milliseconds e = duration_cast<milliseconds>(_end.time_since_epoch());

  return e.count() - b.count();
}
