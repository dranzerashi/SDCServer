#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>

#include "opencv2/opencv_modules.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/opengl.hpp>
#include <opencv2/cudacodec.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/opencv.hpp"

#include "detector.hpp"
#include "camconfig.hpp"
#include "events_api.hpp"

using namespace std;
using namespace cv;
using namespace cv::cuda;

class BlackWaterDetector : public Detector
{
  private:
    CamConfig cfg;

    const float THRESHOLD = 120;
    const int MAX_PIXEL_VAL = 255;
    const int RGB_CHANNELS = 3;
    const int THICKNESS = 4;
    cv::Rect regionOfInterest;

    cv::cuda::GpuMat g;
    cv::Mat frame;
    cv::Mat original_frame;
    float dominant_pixel;

    // detection_start_time -> initial starting time when smoke is detected, it resets when no smoke is detected for certain amount of time
    int64 detection_start_time = 0;
    // buffer_time -> time for which system needs to wait before resetting the 'detection_start_time' in case no smoke is detected before threshold time
    int64 buffer_time = 0;
    int64 snooze_timeout = 600;
    int64 latest_post_timestamp = 0; 

    void check_and_notify(cv::Mat);
    void start_timer();
    int64 timer_duration();
    void stop_timer();

  public:
    BlackWaterDetector(CamConfig camConfig) : cfg(camConfig)
    {
      regionOfInterest = cfg.getROICoords();
    };
    void detect(cv::Mat);
};