#pragma once
#include <iostream>
#include <regex>
#include "opencv2/core.hpp"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

enum class CameraStatus{
    ON, OFF
};

class CamConfig {
    private:
        long id;
        std::string source;
        std::string camID;
        std::string location;
        std::string region;
        CameraStatus status;
        std::string cameraMake;
        std::string ipAddress;
        std::string assetOwner;
        
        long configId;
        std::string roiCoords;
        std::string threshold;
        std::string color;
        std::string modelID;
        bool enable;

    public:
        CamConfig(json j){
            id = j["id"].get<long>();
            camID = j["camID"].get<std::string>();

            source = j["source"].get<std::string>();

            if(!j["location"].empty()){
                location = j["location"].get<std::string>();
            }


            if(!j["region"].empty()){
                region = j["region"].get<std::string>();
            }

            status = j["status"].get<CameraStatus>();

            if(!j["cameraMake"].empty()){
                cameraMake = j["cameraMake"].get<std::string>();
            }

            ipAddress = j["ipAddress"].get<std::string>();
            
            if(!j["assetOwner"].empty()){
                assetOwner = j["assetOwner"].get<std::string>();
            }

            configId = j["configId"].get<long>();
            
            if(!j["roiCoords"].empty()){
                roiCoords = j["roiCoords"].get<std::string>();
            }
            
            threshold = j["threshold"].get<std::string>();
            
            color = j["color"].get<std::string>();
            
            if(!j["modelID"].empty()){
                modelID = j["modelID"].get<std::string>();
            }
            enable = j["enable"].get<bool>();

            getROICoords();
            getMinColorRange();
        }

        std::string getKey(){
            if(camID.empty()){
                return NULL;
            }
            return camID + ":" + std::to_string(configId);
        }

        int getThreshold(){
            return std::stoi(threshold);
        }

        std::string getCamID(){
            return camID;
        }

        std::string getSource(){
            return source;
        }

        cv::Rect getROICoords(){
            std::regex r("\\((\\d+),(\\d+)\\);\\((\\d+),(\\d+)\\);"); 
            std::smatch matches;
            std::regex_match (roiCoords, matches, r);
            cv::Rect roi(std::stoi(matches[1]),
                        std::stoi(matches[2]),
                        std::stoi(matches[3])-std::stoi(matches[1]),
                        std::stoi(matches[4])-std::stoi(matches[2]));
            // for(int i=0; i<matches.size();i++){
            //     std::cout<<"ROI match: "<<matches[i]<<std::endl;
            // }
            return roi;
        }

        cv::Vec3b getMinColorRange(){
            std::regex r("([a-zA-Z]{3}):\\((\\d{1,3}),(\\d{1,3})\\);\\((\\d{1,3}),(\\d{1,3})\\);\\((\\d{1,3}),(\\d{1,3})\\);"); 
            std::smatch matches;
            std::regex_match (color, matches, r);
            cv::Vec3b minRange;
            minRange[0] = std::stoi(matches[2]);
            minRange[1] = std::stoi(matches[4]);
            minRange[2] = std::stoi(matches[6]);
            // for(int i=0; i<matches.size();i++){
            //     std::cout<<"Match: "<<matches[i]<<std::endl;
            // }
            return minRange;
        }

        cv::Vec3b getMaxColorRange(){
            std::regex r("([a-zA-Z]{3}):\\((\\d{1,3}),(\\d{1,3})\\);\\((\\d{1,3}),(\\d{1,3})\\);\\((\\d{1,3}),(\\d{1,3})\\);"); 
            std::smatch matches;
            std::regex_match (color, matches, r);
            cv::Vec3b maxRange;
            maxRange[0] = std::stoi(matches[3]);
            maxRange[1] = std::stoi(matches[5]);
            maxRange[2] = std::stoi(matches[7]);
            // for(int i=0; i<matches.size();i++){
            //     std::cout<<"Match: "<<matches[i]<<std::endl;
            // }
            return maxRange;
        }


};