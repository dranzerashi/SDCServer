cmake_minimum_required(VERSION 2.8)
PROJECT(SDCServer)
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
#SET(lib_PATH ${CMAKE_BINARY_DIR}/lib)
#SET(lib_PATH /home/allahbaksh/workspaces/smokeDetectionWorkspace/served.build)

FIND_PACKAGE(Boost 1.65.1 REQUIRED COMPONENTS system thread regex)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)


FIND_PACKAGE(Pistache REQUIRED)
# IF(Pistache_FOUND)
#   INCLUDE_DIRECTORIES(${Pistache_INCLUDE_DIRS})
#   LINK_DIRECTORIES(${Pistache_LIBRARY_DIRS})
#   message("Pistache: ${Pistache} ${Pistache_INCLUDE_DIRS}")
# ENDIF(Pistache_FOUND)


#INCLUDE_DIRECTORIES(${lib_PATH})
#LINK_DIRECTORIES(${lib_PATH})

#find_library(served NAMES libserved.so PATHS ${lib_PATH})
#message("${served}")


SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} pistache)
message("Used libs ${USED_LIBS}")
ADD_EXECUTABLE(SDCServer SDCServer.cpp)
TARGET_LINK_LIBRARIES(SDCServer ${USED_LIBS})
