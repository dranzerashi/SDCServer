#pragma once
#include <iostream>
#include "opencv2/core.hpp"

class Detector{
    public:
        virtual void detect(cv::Mat) =0;
};