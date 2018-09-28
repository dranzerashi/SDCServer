#pragma once
#include <iostream>
#include <cstddef>
#include <b64/encode.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "nlohmann/json.hpp"
using json = nlohmann::json;
using namespace std;
using namespace cv;

namespace eventns{

    class Event{
        private:
        public:
            long id;
            string camID;
            string timestamp;
            string snapshot;
            string snapshotContentType;
            string eventType;

            Event(string camID,
            string timestamp,
            cv::Mat snapshot,
            string snapshotContentType,
            string eventType ):
            camID(camID),timestamp(timestamp), snapshot(base64encoding(snapshot)),snapshotContentType(snapshotContentType),eventType(eventType){}
            json to_json() {
                return json{{"camID", camID},{"timestamp", timestamp},{"snapshot", snapshot},{"snapshotContentType", snapshotContentType},{"eventType", eventType}};
            }

            void from_json(const json& j, Event& e) {
                e.camID = j.at("camID").get<std::string>();
                e.timestamp = j.at("timestamp").get<std::string>();
                e.snapshot = j.at("snapshot").get<std::string>();
                e.snapshotContentType = j.at("snapshotContentType").get<std::string>();
                e.eventType = j.at("eventType").get<std::string>();
        }
        string base64encoding(cv::Mat input){
            // int width = input.cols;
            // int height = input.rows;
            // int type = input.type();
            // size_t size = input.total() * input.elemSize();

            // // Initialize a stringstream and write the data
            
            // ss.write((char*)(&width), sizeof(int));
            // ss.write((char*)(&height), sizeof(int));
            // ss.write((char*)(&type), sizeof(int));
            // ss.write((char*)(&size), sizeof(size_t));

            // // Write the whole image data
            // ss.write((char*)input.data, size);
            vector<uchar> out;
            imencode(".jpg", input, out);
            string s(out.begin(), out.end());
            stringstream ss(s);
            base64::encoder e;
            stringstream os;
            e.encode(ss,os);
            return os.str();
        }
    };
}