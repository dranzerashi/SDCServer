#include <algorithm>

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include "threadHandler.hpp"


#include "nlohmann/json.hpp"
#include "camconfig.hpp"

using namespace std;
using namespace Pistache;
using njson = nlohmann::json;

void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic {

void handleReady(const Rest::Request&, Http::ResponseWriter response) {
    response.send(Http::Code::Ok, "1");
}

}

class OpticalFlowEndpoint {
public:
    OpticalFlowEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
            .threads(thr)
            .flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() {
        httpEndpoint->shutdown();
    }

private:
    void setupRoutes() {
        using namespace Rest;
        Routes::Post(router, "/start/:cameraID/:configID", Routes::bind(&OpticalFlowEndpoint::startMonitoringOnThread, this));
        Routes::Get(router, "/stop/:cameraID/:configID", Routes::bind(&OpticalFlowEndpoint::stopMonitoringOnThread, this));

        // Routes::Post(router, "/record/:name/:value?", Routes::bind(&OpticalFlowEndpoint::doRecordMetric, this));
        // Routes::Get(router, "/value/:name", Routes::bind(&OpticalFlowEndpoint::doGetMetric, this));
        // Routes::Get(router, "/ready", Routes::bind(&Generic::handleReady));
        // Routes::Get(router, "/auth", Routes::bind(&OpticalFlowEndpoint::doAuth, this));

    }


    void startMonitoringOnThread(const Rest::Request& request, Http::ResponseWriter response){
        string cameraID, configID;
        if(request.hasParam(":cameraID")){
            cameraID = request.param(":cameraID").as<std::string>();
        }
        if(request.hasParam(":configID")){
            configID = request.param(":configID").as<std::string>();
        }
        string content=request.body();
        cout<<"Content "<<content<<endl;
        njson j = njson::parse(content);
        CamConfig cfg(j);
    
        //cout<<j["name"].get<string>()<<" is a "<<j["race"].get<string>()<<" who is always "<<j["status"].get<string>()<<endl;
      
        threadHandler.startThreadForMonitoring(cfg);
        response.send(Http::Code::Ok, "Started");
    }

    void stopMonitoringOnThread(const Rest::Request& request, Http::ResponseWriter response){
      string cameraID, configID;
      if(request.hasParam(":cameraID")){
        cameraID = request.param(":cameraID").as<std::string>();
      }
      if(request.hasParam(":configID")){
        configID = request.param(":configID").as<std::string>();
      }
      threadHandler.stopThreadForMonitoring(cameraID, configID);
      cout<<"Stopping Monitoring on Thread with CameraID: "<<cameraID<<" and configID: "<<configID<<endl;
      response.send(Http::Code::Ok, "Stopped");
    }

    void doRecordMetric(const Rest::Request& request, Http::ResponseWriter response) {
        auto name = request.param(":name").as<std::string>();

        Guard guard(metricsLock);
        auto it = std::find_if(metrics.begin(), metrics.end(), [&](const Metric& metric) {
            return metric.name() == name;
        });

        int val = 1;
        if (request.hasParam(":value")) {
            auto value = request.param(":value");
            val = value.as<int>();
        }

        if (it == std::end(metrics)) {
            metrics.push_back(Metric(std::move(name), val));
            response.send(Http::Code::Created, std::to_string(val));
        }
        else {
            auto &metric = *it;
            metric.incr(val);
            response.send(Http::Code::Ok, std::to_string(metric.value()));
        }

    }

    void doGetMetric(const Rest::Request& request, Http::ResponseWriter response) {
        auto name = request.param(":name").as<std::string>();

        Guard guard(metricsLock);
        auto it = std::find_if(metrics.begin(), metrics.end(), [&](const Metric& metric) {
            return metric.name() == name;
        });

        if (it == std::end(metrics)) {
            response.send(Http::Code::Not_Found, "Metric does not exist");
        } else {
            const auto& metric = *it;
            response.send(Http::Code::Ok, std::to_string(metric.value()));
        }

    }

    void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        printCookies(request);
        response.cookies()
            .add(Http::Cookie("lang", "en-US"));
        response.send(Http::Code::Ok);
    }

    class Metric {
    public:
        Metric(std::string name, int initialValue = 1)
            : name_(std::move(name))
            , value_(initialValue)
        { }

        int incr(int n = 1) {
            int old = value_;
            value_ += n;
            return old;
        }

        int value() const {
            return value_;
        }

        std::string name() const {
            return name_;
        }
    private:
        std::string name_;
        int value_;
    };

    typedef std::mutex Lock;
    typedef std::lock_guard<Lock> Guard;
    Lock metricsLock;
    std::vector<Metric> metrics;

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;

    ThreadHandler threadHandler;
};

int main(int argc, char *argv[]) {
    Port port(9080);

    int thr = 17;

    if (argc >= 2) {
        port = std::stol(argv[1]);

        if (argc == 3)
            thr = std::stol(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;
    OpticalFlowEndpoint stats(addr);
    stats.init(thr);
    stats.start();
    stats.shutdown();

    //How to us post call
    // cv::Mat image = imread( "../src/Images/download.jpeg", CV_LOAD_IMAGE_COLOR );
    // Event event("AXIS0001", "2018-04-20T01:17:56.787Z", image, "image/jpg","SMOKE_DETECTION");
    // postEvent(event);
}