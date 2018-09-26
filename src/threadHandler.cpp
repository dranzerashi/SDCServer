#include "threadHandler.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include "optical_flow.hpp"


using namespace std;
using namespace cv;
using namespace cv::cuda;

namespace ocv
{
void wait(int seconds)
{
    boost::this_thread::sleep_for(boost::chrono::seconds{seconds});
}
void startOpticalFlowMonitoring(shared_ptr<map<string, bool>> running_threads, CamConfig cfg)
{
    string inputName = "/home/allahbaksh/workspaces/smokeDetectionWorkspace/SDCServer/data/yellow_smoke.mp4";
    cout << "Thread started successfully at: " << boost::this_thread::get_id() << endl;
    try
    {
        VideoCapture capture;
        capture.open(inputName);
        Mat frame;
        OpticalFlowProcess ofp(cfg);
        while (true)
        {
            cout << "Thread running at: " << boost::this_thread::get_id() << endl;
            capture >> frame;
            //cout<<"RunningThread:"<<running_threads->at(threadKey)<<endl;
            if (frame.empty() || !running_threads->at(cfg.getKey()))
            {
                cout<<"Frames Ended"<<endl;
                break;
            }
            ofp.process(frame);
            
            
            //wait(1);
            //boost::this_thread::interruption_point();
        }
        capture.release();
        destroyAllWindows();
    }
    catch (boost::thread_interrupted &)
    {
        cout << "Thread interrupted" << endl;
    }
    running_threads->at(cfg.getKey())=false;
    cout << "Thread stopped" << endl;
}
}; // namespace ocv

void ThreadHandler::startThreadForMonitoring(CamConfig cfg)
{
    cout << "Starting thread" << endl;
    //ThreadKey threadKey(cameraID, configID);
    string threadKey = cfg.getKey();
    cout << "threadKey" << threadKey << endl;
    if (running_threads->count(threadKey) == 0)
    {
        running_threads->insert(std::pair<string, bool>(threadKey, true));
    }else{
        running_threads->at(threadKey)=true;
    }
    boost::thread t(ocv::startOpticalFlowMonitoring, running_threads, cfg);
    //shared_ptr<boost::thread> threadPtr(&t);
    //running_threads.insert(std::pair<string, bool>(threadKey, true));
}
void ThreadHandler::stopThreadForMonitoring(std::string cameraID, std::string configID)
{
    std::cout << "Stopping thread" << endl;
    shared_ptr<boost::thread> threadPtr;
    string threadKey = cameraID + ":" + configID;
    if (running_threads->count(threadKey) == 1)
    {
        // threadPtr = running_threads.at(threadKey);
        // cout << "thread: " << threadPtr << endl;
        // cout << "Thread found" << threadPtr->get_id();
        // threadPtr->interrupt();
        //TODO mutex
        running_threads->at(threadKey)=false;
    }
    // (*threadPtr).join();
}
