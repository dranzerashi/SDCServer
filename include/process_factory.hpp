#pragma once
#include <iostream>
#include <string>

#include "camconfig.hpp"
#include "detector.hpp"
#include "optical_flow.hpp"
#include "black_water_detector.hpp"

using namespace std;

class ProcessFactory{
    
    public:
        // ProcessFactory(){};
        static shared_ptr<Detector> getDetector(CamConfig cfg){
            if(cfg.getModelID() == "smoke_detection"){
                cout<<"Model: smoke_detection"<<endl;
                return make_shared<OpticalFlowProcess>(OpticalFlowProcess(cfg));
            } else if (cfg.getModelID() == "black_water_detection"){
                cout<<"Model: black_water_detection"<<endl;
                return make_shared<BlackWaterDetector>(BlackWaterDetector(cfg));
            }
            throw runtime_error("No model ID present!!");
        }
};