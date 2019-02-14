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

class FlameDetector : public Detector
{
  private:
    CamConfig cfg;

    const int X_CORDINATE = 575;
    const int Y_CORDINATE = 45;
    const int WIDTH =  60;
    const int HEIGHT = 50;

    const int CONST_AREA_THRESHOLD = 30;
    const int CONST_FLAME_THRESHOLD = 10;
    const int THICKNESS = 2;

    static GpuMat prev_frame;

    Mat frame_HSI;

    cv::Rect regionOfInterest;


    GpuMat gpu_frame;
    Mat frame;
    cv::Mat greyMat;
    GpuMat next_frame;
    Mat croppedImg;
    bool r1,r2,r3;
    
    

    // detection_start_time -> initial starting time when smoke is detected, it resets when no smoke is detected for certain amount of time
    int64 detection_start_time = 0;
    // buffer_time -> time for which system needs to wait before resetting the 'detection_start_time' in case no smoke is detected before threshold time
    int64 buffer_time = 0;
    int64 snooze_timeout = 600;
    int64 latest_post_timestamp = 0; 



    bool Flame_Area_Detection(const Mat &src);
    bool get_HSI(const Mat &pic);
    bool Motion_Detection(const Mat &prev,const  Mat &next);


    void check_and_notify(cv::Mat);
    void start_timer();
    int64 timer_duration();
    void stop_timer();


  public:
    FlameDetector(CamConfig camConfig) : cfg(camConfig)
    {
      regionOfInterest = cfg.getROICoords();
    };
    void detect(cv::Mat);
};