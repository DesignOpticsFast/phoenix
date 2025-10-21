cmake_minimum_required(VERSION 3.20)

# Include policy enforcement first
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(policy_no_deprecated)

project(Phoenix
  VERSION 0.0.1
  DESCRIPTION "Qt UI for Bedrock"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automoc/uic/rcc so Q_OBJECT headers generate moc files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent Core Graphs)

# Pull Bedrock in as sibling
add_subdirectory(../bedrock ${CMAKE_BINARY_DIR}/_bedrock_build)

add_executable(phoenix_app
  # sources
  src/main.cpp
  src/adapters/bedrock_client.cpp
  src/ui/StepViewer.cpp
  src/palantir/PalantirClient.cpp
  src/analysis/XYWindow.cpp
  src/plot/QtGraphsPlotView.cpp
  src/palantir/palantir.pb.cc
  
  # New UI structure - MainWindow
  src/ui/main/MainWindow.cpp
  
  # New UI structure - Dialogs
  src/ui/dialogs/PreferencesDialog.cpp
  src/ui/dialogs/EnvironmentPage.cpp
  
  # New UI structure - Themes
  src/ui/themes/ThemeManager.cpp
  
  # Icon system sources
  src/ui/icons/IconProvider.cpp
  src/ui/icons/IconBootstrap.cpp
  src/ui/icons/PhxIconImageProvider.cpp
  src/tools/IconGallery.cpp

  # headers with Q_OBJECT for AUTOMOC
  src/ui/StepViewer.hpp
  src/palantir/PalantirClient.hpp
  src/analysis/XYWindow.hpp
  src/plot/QtGraphsPlotView.hpp
  src/tools/IconGallery.h
  
  # New UI structure - MainWindow
  src/ui/main/MainWindow.h
  
  # New UI structure - Dialogs
  src/ui/dialogs/PreferencesDialog.h
  src/ui/dialogs/EnvironmentPage.h
  
  # New UI structure - Themes
  src/ui/themes/ThemeManager.h

  # non-Q_OBJECT header (ok to list)
  src/adapters/bedrock_client.hpp
  src/ui/icons/IconProvider.h
  src/ui/icons/IconBootstrap.h
  src/ui/icons/PhxIconImageProvider.h
  src/ui/MainWindow.ui
)

target_include_directories(phoenix_app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ../bedrock/include
)

# Add icon resources
qt_add_resources(phoenix_app "phoenix_icons"
  PREFIX "/"
  FILES
    src/ui/icons/phoenix_icons.qrc
)

# Add translation files
# Note: Translation files are created but not compiled in this stage
# They will be processed in future phases when lupdate/lrelease are available

target_link_libraries(phoenix_app PRIVATE
  Qt6::Widgets
  Qt6::Concurrent
  Qt6::Core
  Qt6::Graphs
  bedrock_plugin  # Clean plugin interface without OCCT dependencies
  protobuf  # Protocol Buffers for Palantir communication
)

if(APPLE)
  set_target_properties(phoenix_app PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "dev.phoenix.ui"
    MACOSX_BUNDLE_BUNDLE_NAME "Phoenix"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
endif()
