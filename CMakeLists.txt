cmake_minimum_required(VERSION 3.7)
PROJECT(SDCServer)
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)

FIND_PACKAGE(Boost 1.65.1 REQUIRED COMPONENTS system thread regex)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)


FIND_PACKAGE(Pistache REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)

FIND_PACKAGE(restc-cpp REQUIRED)
find_path(B64_INCLUDE NAMES b64/encode.h)
find_library(B64_LIB NAMES b64)
#FIND_PACKAGE(b64 REQUIRED)
#FIND_PACKAGE(cpprestsdk REQUIRED)
# LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../restc-cpp/lib)

# add_library(restc-cpp STATIC IMPORTED)
# set_property(TARGET restc-cpp PROPERTY IMPORTED_LOCATION /home/allahbaksh/workspaces/smokeDetectionWorkspace/restc-cpp/lib/librestc-cpp.a)
# include_directories(/home/allahbaksh/workspaces/smokeDetectionWorkspace/restc-cpp/dbuild/external-projects/src/externalRapidJson/include)
# include_directories(/home/allahbaksh/workspaces/smokeDetectionWorkspace/restc-cpp/include)


SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} pistache b64 ${OpenCV_LIBS} ${RESTC_CPP_LIBRARIES})# cpprestsdk::cpprest)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SOURCES "src/*.cpp")

ADD_EXECUTABLE(SDCServer ${SOURCES})
TARGET_LINK_LIBRARIES(SDCServer ${USED_LIBS})
