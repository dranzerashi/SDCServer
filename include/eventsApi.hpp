#pragma once
#include <iostream>
#include <string>
#include "restc-cpp/restc-cpp.h"
#include "restc-cpp/SerializeJson.h"
#include "restc-cpp/RequestBuilder.h"
#include "event.hpp"


using namespace std;
using namespace restc_cpp;

void postEvent(eventns::Event event)
{
   auto rest_client = RestClient::Create();
   
   //Event event("AXIS0001", "2018-04-20T01:17:56.787Z","", "image/jpg","SMOKE_DETECTION");
//    cout<<"camid "<<event.camID<<endl;
   auto done = rest_client->ProcessWithPromise([&](Context& ctx){
       json j = event.to_json();
       auto res = ctx.Post("http://localhost:8080/api/events",j.dump());
       cout<<"Response: "<<res.get()<<endl;
   });
   try{
       done.get();
   }catch(const exception& ex){
       clog<<"Main thread: Caught exception"<<ex.what()<<endl;
   }
}