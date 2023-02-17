cmake_minimum_required(VERSION 3.14)

project(MongoCore LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")


file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set(bsoncxx_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../mongo-msvc2022x64-noBoost/lib/cmake/bsoncxx-3.7.0)
set(mongocxx_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../mongo-msvc2022x64-noBoost/lib/cmake/mongocxx-3.7.0)


include_directories("${CMAKE_SOURCE_DIR}")

add_library(MongoCore SHARED
  ${HEADERS}
  ${SOURCES}
)

target_include_directories(MongoCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../mongo-msvc2022x64-noBoost/include")

target_link_directories(MongoCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../mongo-msvc2022x64-noBoost/lib")

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})

target_link_libraries(MongoCore PRIVATE mongo::bsoncxx_shared)

target_link_libraries(MongoCore PRIVATE mongo::mongocxx_shared)


target_compile_definitions(MongoCore PRIVATE MONGOCORE_LIBRARY)
