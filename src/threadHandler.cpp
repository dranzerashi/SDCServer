#include "threadHandler.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
using namespace std;

namespace ocv
{
void wait(int seconds)
{
    boost::this_thread::sleep_for(boost::chrono::seconds{seconds});
}
void startOpticalFlowMonitoring()
{
    cout << "Thread started successfully at: " << boost::this_thread::get_id() << endl;
    try
    {
        while (true)
        {
            cout << "Thread running at: " << boost::this_thread::get_id() << endl;
            wait(1);
            boost::this_thread::interruption_point();
        }
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
    cout<<"threadKey"<<threadKey<<endl;
    boost::thread t{ocv::startOpticalFlowMonitoring};
    running_threads.insert(std::pair<string, boost::thread *>(threadKey, &t));
}
void ThreadHandler::stopThreadForMonitoring(std::string cameraID, std::string configID)
{
    std::cout << "Stopping thread" << endl;
    shared_ptr<boost::thread> threadPtr;
    string threadKey = cameraID + ":" + configID;
    if (running_threads.count(threadKey)==1)
    {
        threadPtr = running_threads.at(threadKey);
        cout<<"thread: "<< threadPtr <<endl;
        std::cout << "Thread found" << (*threadPtr).get_id();
        (*threadPtr).interrupt();
    }
    (*threadPtr).join();
}
