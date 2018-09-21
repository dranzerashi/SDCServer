#include <iostream>
#include <fstream>

#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/cudaoptflow.hpp"
#include "opencv2/cudaarithm.hpp"
#include <opencv2/opencv.hpp>

void processFrameWithOpticalFlow(cv::Mat, cv::Mat);

class OpticalFlowProcess{
    private:
        cv::cuda::GpuMat prev_frame;
        cv::cuda::GpuMat curr_frame;
        cv::Ptr<cv::cuda::FarnebackOpticalFlow> farn; 
    
    public:
        OpticalFlowProcess(): farn(cv::cuda::FarnebackOpticalFlow::create(1,0.5,false,5,8,5,1.1,0)){}
        void process(cv::Mat);
};