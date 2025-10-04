cmake_minimum_required(VERSION 3.20)

project(Phoenix
  VERSION 0.0.1
  DESCRIPTION "Qt UI for Bedrock"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Pull Bedrock in as sibling
add_subdirectory(../bedrock ${CMAKE_BINARY_DIR}/_bedrock_build)

add_executable(phoenix_app
  src/main.cpp
  src/adapters/bedrock_client.cpp
  src/ui/MainWindow.cpp
  src/ui/StepViewer.cpp
)

target_include_directories(phoenix_app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../bedrock/include
)

target_link_libraries(phoenix_app PRIVATE
  Qt6::Widgets
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
