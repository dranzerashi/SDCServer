#include<iostream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <map>

struct ThreadKey {
    std::string cameraID;
    std::string configID;
};

class ThreadHandler{
    private:
        std::map<ThreadKey, boost::thread::id> running_threads;

    public:
        void startThreadForMonitoring();
        void stopThreadForMonitoring(std::string cameraID, std::string configID);

};
