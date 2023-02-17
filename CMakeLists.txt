cmake_minimum_required(VERSION 3.14)

project(MongoCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(MongoCore SHARED
  MongoCore_global.h
  mongocore.cpp
  mongocore.h
)

target_compile_definitions(MongoCore PRIVATE MONGOCORE_LIBRARY)
