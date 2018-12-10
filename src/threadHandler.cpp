#include "threadHandler.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include "process_factory.hpp"


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
    string inputName = cfg.getIPAdress(); // "/home/allahbaksh/workspaces/smokeDetectionWorkspace/SDCServer/data/yellow_smoke.mp4";
    cout << "Thread started successfully at: " << boost::this_thread::get_id() << endl;
    try
    {
        VideoCapture capture;
        capture.open(inputName);
        Mat frame;
        // OpticalFlowProcess ofp(cfg);
        std::shared_ptr<Detector> ofp = ProcessFactory::getDetector(cfg);
        int ctr=0;
        const int64 start = getTickCount();
        bool is_fps_calculated = false;
        while (true)
        {
            // cout << "Thread running at: " << boost::this_thread::get_id() << endl;
            capture >> frame;
            //cout<<"RunningThread:"<<running_threads->at(threadKey)<<endl;
            if (frame.empty() || !running_threads->at(cfg.getKey()))
            {
                cout<<"Frames Ended"<<endl;
                break;
            }
            ofp.get()->detect(frame);

            ctr+=1;
            int64 duration = float(getTickCount() - start) / getTickFrequency();
            if(  duration > 5 && !is_fps_calculated){
                is_fps_calculated = true;
                cout<<"FPS for "<<inputName<<" is "<<ctr/duration<< endl;
            }
            
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
    catch (exception e){
        cout << e.what() << endl;
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

std::map<std::string, bool> ThreadHandler::runningThreads(){
    std::cout<<"ThreadHandler::runningThreads"<<endl;
    std::map<std::string, bool> copy_running_threads;
    copy_running_threads.insert(running_threads->begin(), running_threads->end());
    return copy_running_threads;
    
}
