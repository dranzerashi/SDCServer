#pragma once
#include <iostream>
#include <string>
#include "restc-cpp/restc-cpp.h"
#include "restc-cpp/SerializeJson.h"
#include "restc-cpp/RequestBuilder.h"
#include "event.hpp"


using namespace std;
using namespace restc_cpp;

void postEvent(eventns::Event);