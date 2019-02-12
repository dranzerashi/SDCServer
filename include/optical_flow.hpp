#pragma once
#include <iostream>
#include <fstream>

#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/cudaoptflow.hpp"
#include "opencv2/cudaarithm.hpp"
#include <opencv2/opencv.hpp>
#include "camconfig.hpp"

#include "detector.hpp"

#define BLOCK_SIZE 5
#define ITERATIONS 8
#define POLY_N 5
#define POLY_SIGMA 1.1
#define MOVEMENT_INTENSITY_THRESHOLD 14.5



void processFrameWithOpticalFlow(cv::Mat, cv::Mat);

class OpticalFlowProcess: public Detector{
    private:
        cv::cuda::GpuMat prev_frame;
        cv::cuda::GpuMat curr_frame;
        cv::Ptr<cv::cuda::FarnebackOpticalFlow> farn; 
        CamConfig cfg;
        // detection_start_time -> initial starting time when smoke is detected, it resets when no smoke is detected for certain amount of time
        int64 detection_start_time = 0;
        // buffer_time -> time for which system needs to wait before resetting the 'detection_start_time' in case no smoke is detected before threshold time
        int64 buffer_time = 0;
        int64 snooze_timeout = 600;
        int64 latest_post_timestamp = 0;
        void start_timer();
        void stop_timer();
        int64 timer_duration();
    
    public:
        OpticalFlowProcess(CamConfig camconfig): farn(cv::cuda::FarnebackOpticalFlow::create(1,0.5,false,BLOCK_SIZE,ITERATIONS,POLY_N,POLY_SIGMA,0)), cfg(camconfig){}
        void detect(cv::Mat);
};