#include "recognizers/mobilenetssdrecognizer.h"

// Params, neural net from
// https://www.pyimagesearch.com/2017/09/11/object-detection-with-deep-learning-and-opencv/
// and
// https://web-answers.ru/c/opencv-c-hwnd2mat-skrinshot-gt-blobfromimage.html
MobileNetSSDRecognizer::MobileNetSSDRecognizer()
    : CaffeRecognizer("./models/MobileNetSSD_deploy.prototxt.txt",
                      "./models/MobileNetSSD_deploy.caffemodel", 0.007843,
                      cv::Size(300, 300), cv::Scalar(127.5, 127.5, 127.5),
                      false, false, CV_32F) {}
