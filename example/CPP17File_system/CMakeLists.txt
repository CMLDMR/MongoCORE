cmake_minimum_required(VERSION 3.5)

project(CPP17File_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")

add_executable(CPP17File_system main.cpp)

target_include_directories(CPP17File_system PUBLIC "C:/Mongo/mongoCPPCore/install/include")
target_link_directories(CPP17File_system PUBLIC "C:/Mongo/mongoCPPCore/install/lib")
target_link_libraries(CPP17File_system PRIVATE MongoCore)


set(bsoncxx_DIR C:/Mongo/mongo-msvc2022x64-noBoost/lib/cmake/bsoncxx-3.7.0)
set(mongocxx_DIR C:/Mongo/mongo-msvc2022x64-noBoost/lib/cmake/mongocxx-3.7.0)
target_include_directories(CPP17File_system PUBLIC "C:/Mongo/mongo-msvc2022x64-noBoost/include")
target_link_directories(CPP17File_system PUBLIC "C:/Mongo/mongo-msvc2022x64-noBoost/lib")

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})

target_link_libraries(CPP17File_system PRIVATE mongo::bsoncxx_shared)
target_link_libraries(CPP17File_system PRIVATE mongo::mongocxx_shared)


install(TARGETS CPP17File_system
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
