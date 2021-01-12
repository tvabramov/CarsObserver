#ifndef CAPTURER_H
#define CAPTURER_H

#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <string>
#include <thread>
#include <utility>

struct CapturerOutput {
  cv::Mat frame;
  std::chrono::time_point<std::chrono::system_clock> timestamp;

  CapturerOutput() {}
  CapturerOutput(
      const cv::Mat& _frame,
      const std::chrono::time_point<std::chrono::system_clock>& _timestamp)
      : frame(_frame), timestamp(_timestamp) {}
  CapturerOutput(
      cv::Mat&& _frame,
      std::chrono::time_point<std::chrono::system_clock>&& _timestamp)
      : frame(std::move(_frame)), timestamp(std::move(_timestamp)) {}

  CapturerOutput(const CapturerOutput& _other) = default;

  CapturerOutput(CapturerOutput&& _other) noexcept
      : frame(std::move(_other.frame)),
        timestamp(std::move(_other.timestamp)) {}

  CapturerOutput& operator=(const CapturerOutput& _other) = default;

  CapturerOutput& operator=(CapturerOutput&& _other) noexcept {
    frame = std::move(_other.frame);
    timestamp = std::move(_other.timestamp);

    return *this;
  }
};

class Capturer {
 public:
  explicit Capturer(std::string _source, int _settedFrameWidth,
                    int _settedFrameHeight, std::string _settedCodec,
                    int _settedFps, cv::Rect2d _roi, int _framesDelayMs,
                    std::string _origFrameName, int _timeoutMs);

  std::list<CapturerOutput> pop();

  void doWork();

 protected:
  std::string mSource;
  int mSettedFrameWidth, mSettedFrameHeight;
  std::string mSettedCodec;
  int mSettedFps;
  cv::Rect2d mRoi;
  int mFramesDelayMs;
  std::string mOrigFrameName;
  int mTimeoutMs;
  std::chrono::time_point<std::chrono::system_clock> mPrevFrameTs;
  bool mMustDoOrig;
  cv::VideoCapture mCvCapture;

  std::list<CapturerOutput> mOutputData;
  std::mutex mOutputMutex;
  std::condition_variable mHaveOutput;
};

#endif  // CAPTURER_H
