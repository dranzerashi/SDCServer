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
void startOpticalFlowMonitoring()
{
    string inputName = "/home/allahbaksh/workspaces/smokeDetectionWorkspace/SDCServer/data/yellow_smoke.mp4";
    cout << "Thread started successfully at: " << boost::this_thread::get_id() << endl;
    try
    {
        VideoCapture capture;
        capture.open(inputName);
        Mat frame, prev_frame;
        OpticalFlowProcess ofp;
        while (true)
        {
            cout << "Thread running at: " << boost::this_thread::get_id() << endl;
            capture >> frame;
            if (frame.empty())
            {
                cout<<"Frames Ended"<<endl;
                break;
            }
            // if (prev_frame.empty())
            // {
            //     prev_frame = frame;
            //     continue;
            // }
            //imshow("frame1", prev_frame);
            //if (waitKey(30) >= 0)break;

            //processFrameWithOpticalFlow(prev_frame, frame);
            ofp.process(frame);
            //prev_frame = frame;
            
            //wait(1);
            //boost::this_thread::interruption_point();
        }
        //destroyAllWindows();
        capture.release();
    }
    catch (boost::thread_interrupted &)
    {
        cout << "Thread interrupted" << endl;
    }

    cout << "Thread stopped" << endl;
}
}; // namespace ocv

void ThreadHandler::startThreadForMonitoring(std::string cameraID, std::string configID)
{
    cout << "Starting thread" << endl;
    //ThreadKey threadKey(cameraID, configID);
    string threadKey = cameraID + ":" + configID;
    cout << "threadKey" << threadKey << endl;
    boost::thread t{ocv::startOpticalFlowMonitoring};
    shared_ptr<boost::thread> threadPtr(&t);
    running_threads.insert(std::pair<string, shared_ptr<boost::thread>>(threadKey, threadPtr));
}
void ThreadHandler::stopThreadForMonitoring(std::string cameraID, std::string configID)
{
    std::cout << "Stopping thread" << endl;
    shared_ptr<boost::thread> threadPtr;
    string threadKey = cameraID + ":" + configID;
    if (running_threads.count(threadKey) == 1)
    {
        threadPtr = running_threads.at(threadKey);
        cout << "thread: " << threadPtr << endl;
        cout << "Thread found" << threadPtr->get_id();
        threadPtr->interrupt();
    }
    (*threadPtr).join();
}
