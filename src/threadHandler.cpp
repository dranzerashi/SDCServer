#include "threadHandler.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

void ThreadHandler::startThreadForMonitoring(){
    std::cout<<"Starting thread";
}

void ThreadHandler::stopThreadForMonitoring(std::string cameraID, std::string configID){
    std::cout<<"Stopping thread";
}








