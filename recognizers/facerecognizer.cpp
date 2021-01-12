#include "recognizers/facerecognizer.h"

// Params, neural net from
// https://www.pyimagesearch.com/2018/09/24/opencv-face-recognition/
FaceRecognizer::FaceRecognizer()
    : CaffeRecognizer(
          "./models/res10_300x300_ssd_iter_140000_deploy.prototxt.txt",
          "./models/res10_300x300_ssd_iter_140000.caffemodel", 1.0,
          cv::Size(300, 300), cv::Scalar(104.0, 177.0, 123.0), false, false,
          CV_32F) {}
