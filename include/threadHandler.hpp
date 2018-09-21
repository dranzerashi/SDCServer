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
        std::shared_ptr<std::map<std::string, bool>> running_threads;

    public:
        ThreadHandler():running_threads(new std::map<std::string, bool>()){}
        void startThreadForMonitoring(std::string cameraID, std::string configID);
        void stopThreadForMonitoring(std::string cameraID, std::string configID);

};
