cmake_minimum_required(VERSION 3.13.0)

# set the project name
project(DHT_Common)

#Header files
include_directories(include)

find_library(wiringPi_LIB wiringPi)

file(GLOB SOURCES "src/*.cpp")


# add the executable
add_executable(dht ${SOURCES})

#add link libraries
target_link_libraries(dht wiringPi mysqlcppconn)
