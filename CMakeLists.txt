cmake_minimum_required(VERSION 3.20)

project(Phoenix
  VERSION 0.0.1
  DESCRIPTION "Qt UI for Bedrock"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automoc/uic/rcc so Q_OBJECT headers generate moc files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

# Pull Bedrock in as sibling
add_subdirectory(../bedrock ${CMAKE_BINARY_DIR}/_bedrock_build)

add_executable(phoenix_app
  # sources
  src/main.cpp
  src/adapters/bedrock_client.cpp
  src/ui/MainWindow.cpp
  src/ui/StepViewer.cpp

  # headers with Q_OBJECT for AUTOMOC
  src/ui/MainWindow.hpp
  src/ui/StepViewer.hpp

  # non-Q_OBJECT header (ok to list)
  src/adapters/bedrock_client.hpp
  src/ui/MainWindow.ui
)

target_include_directories(phoenix_app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ../bedrock/include
)

target_link_libraries(phoenix_app PRIVATE
  Qt6::Widgets
  Qt6::Concurrent
  bedrock_engine
)

if(APPLE)
  set_target_properties(phoenix_app PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "dev.phoenix.ui"
    MACOSX_BUNDLE_BUNDLE_NAME "Phoenix"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
endif()
