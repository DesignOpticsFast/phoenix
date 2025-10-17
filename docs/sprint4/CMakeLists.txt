cmake_minimum_required(VERSION 3.16)
project(QtChartsScaleProbe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts)

# Set Qt6_DIR
set(Qt6_DIR "/opt/Qt/6.9.3/gcc_64/lib/cmake/Qt6")

# Create executable
add_executable(gate0_5_qtcharts_test gate0_5_qtcharts_test.cpp)
add_executable(gate0_5_qtcharts_headless gate0_5_qtcharts_headless.cpp)
add_executable(gate0_5_transport_test gate0_5_transport_test.cpp)

# Enable Qt6 MOC
set_target_properties(gate0_5_qtcharts_test PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

set_target_properties(gate0_5_qtcharts_headless PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

set_target_properties(gate0_5_transport_test PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Link Qt6 libraries
target_link_libraries(gate0_5_qtcharts_test 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Charts
)

target_link_libraries(gate0_5_qtcharts_headless 
    Qt6::Core
)

target_link_libraries(gate0_5_transport_test 
    Qt6::Core
)

# Set properties
set_target_properties(gate0_5_qtcharts_test PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)
