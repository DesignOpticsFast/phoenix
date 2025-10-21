cmake_minimum_required(VERSION 3.16)
project(qt_screenshot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

add_executable(qt_screenshot qt_screenshot.cpp)

target_link_libraries(qt_screenshot Qt6::Core Qt6::Widgets Qt6::Gui)
