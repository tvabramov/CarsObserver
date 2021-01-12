#include "capturer.h"

#include <boost/log/trivial.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace cv;
using namespace std;

Capturer::Capturer(string _source, int _settedFrameWidth,
                   int _settedFrameHeight, string _settedCodec, int _settedFps,
                   Rect2d _roi, int _framesDelayMs, string _origFrameName,
                   int _timeoutMs)
    : mSource(move(_source)),
      mSettedFrameWidth(_settedFrameWidth),
      mSettedFrameHeight(_settedFrameHeight),
      mSettedCodec(move(_settedCodec)),
      mSettedFps(_settedFps),
      mRoi(move(_roi)),
      mFramesDelayMs(_framesDelayMs),
      mOrigFrameName(move(_origFrameName)),
      mTimeoutMs(_timeoutMs),
      mPrevFrameTs(chrono::system_clock::now()),
      mMustDoOrig(mOrigFrameName.empty() ? false : true) {
  mCvCapture = mSource.empty() ? VideoCapture(0) : VideoCapture(mSource);

  // For getting cam info use "sudo v4l2-ctl -d /dev/video0 --list-formats-ext"
  if (mSettedFrameWidth > 0)
    mCvCapture.set(CAP_PROP_FRAME_WIDTH, mSettedFrameWidth);

  if (mSettedFrameHeight > 0)
    mCvCapture.set(CAP_PROP_FRAME_HEIGHT, mSettedFrameHeight);

  if (mSettedCodec.length() == 4)
    mCvCapture.set(CAP_PROP_FOURCC,
                   VideoWriter::fourcc(mSettedCodec[0], mSettedCodec[1],
                                       mSettedCodec[2], mSettedCodec[3]));

  if (mSettedFps > 0) mCvCapture.set(CAP_PROP_FPS, mSettedFps);
}

list<CapturerOutput> Capturer::pop() {
  unique_lock<mutex> lck(mOutputMutex);

  if (mOutputData.empty())
    mHaveOutput.wait_for(lck, chrono::milliseconds(mTimeoutMs));

  return move(mOutputData);
}

void Capturer::doWork() {
  Mat frame;
  mCvCapture >> frame;

  if (!frame.empty() && mMustDoOrig) {
    imwrite(mOrigFrameName, frame);
    mMustDoOrig = false;
  }

  // assert(!frame.empty());
  if (frame.empty()) {
    this_thread::sleep_for(std::chrono::milliseconds(500));

    mCvCapture.release();
    mCvCapture = VideoCapture(mSource.empty() ? 0 : mSource);

    BOOST_LOG_TRIVIAL(warning)
        << "Frame is empty, video capturer has been reopened";
    return;
  }

  // Delay for video replaing
  if (mFramesDelayMs > 0)
    this_thread::sleep_for(std::chrono::milliseconds(mFramesDelayMs));

  auto ts = chrono::system_clock::now();

  if (!mRoi.empty()) {
    Rect2d roi(mRoi);

    if (roi.x < 0) roi.x = 0;
    if (roi.y < 0) roi.y = 0;
    if (roi.x + roi.width > frame.cols) roi.width = frame.cols - roi.x;
    if (roi.y + roi.height > frame.rows) roi.height = frame.rows - roi.y;

    frame = frame(roi);
  }

  unique_lock<mutex> lck(mOutputMutex);

  mOutputData.push_back(CapturerOutput(frame, ts));

  BOOST_LOG_TRIVIAL(trace) << "Pushed new frame";

  mHaveOutput.notify_all();
}
