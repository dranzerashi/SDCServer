#include<iostream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <map>

class ThreadKey {
    private:
        std::string cameraID;
        std::string configID;
    
    public:
        ThreadKey(std::string cameraID, std::string configID):cameraID(cameraID),configID(configID){}
};

class ThreadHandler{
    private:
        //std::map<ThreadKey, boost::thread::id> running_threads;
        std::map<std::string, std::shared_ptr<boost::thread>> running_threads;

    public:
        void startThreadForMonitoring(std::string cameraID, std::string configID);
        void stopThreadForMonitoring(std::string cameraID, std::string configID);

};
