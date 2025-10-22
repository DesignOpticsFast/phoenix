cmake_minimum_required(VERSION 3.20)

# Include policy enforcement first (if available)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/policy_no_deprecated.cmake")
    include(policy_no_deprecated)
else()
    message(STATUS "Policy file not found - skipping deprecation checks")
endif()

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

# Add icon audit tool for systematic icon checking
add_executable(icon_audit tools/icon_audit.cpp)
target_link_libraries(icon_audit PRIVATE Qt6::Core)


find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent Core Graphs)

# Phoenix is standalone for Sprint 4 Stage 1
# Bedrock integration will be added in future stages

add_executable(phoenix_app
  # sources
  src/main.cpp
  src/ui/StepViewer.cpp
  
  # New UI structure - MainWindow
  src/ui/main/MainWindow.cpp
  
  # New UI structure - Dialogs
  src/ui/dialogs/PreferencesDialog.cpp
  src/ui/dialogs/EnvironmentPage.cpp
  src/ui/dialogs/LanguagePage.cpp
  
  # New UI structure - Themes
  src/ui/themes/ThemeManager.cpp
  
  # New UI structure - Splash Screen
  src/ui/splash/PhoenixSplashScreen.cpp
  
  # Icon system sources
  src/ui/icons/IconProvider.cpp
  src/ui/icons/IconBootstrap.cpp
  src/ui/icons/PhxIconImageProvider.cpp
  src/tools/IconGallery.cpp

  # headers with Q_OBJECT for AUTOMOC
  src/ui/StepViewer.hpp
  src/tools/IconGallery.h
  
  # New UI structure - MainWindow
  src/ui/main/MainWindow.h
  
  # New UI structure - Dialogs
  src/ui/dialogs/PreferencesDialog.h
  src/ui/dialogs/EnvironmentPage.h
  src/ui/dialogs/LanguagePage.h
  
  # New UI structure - Themes
  src/ui/themes/ThemeManager.h
  
  # New UI structure - Splash Screen
  src/ui/splash/PhoenixSplashScreen.h

  # non-Q_OBJECT header (ok to list)
  src/ui/icons/IconProvider.h
  src/ui/icons/IconBootstrap.h
  src/ui/icons/PhxIconImageProvider.h
  src/ui/MainWindow.ui
  
  # Resource files (AUTORCC)
  src/ui/icons/phoenix_icons.qrc
  
  # macOS bundle icon
  $<$<PLATFORM_ID:Darwin>:${APP_ICNS}>
)

# Phoenix standalone - no Bedrock-dependent sources for Sprint 4 Stage 1
# Bedrock integration sources will be added in future stages

target_include_directories(phoenix_app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform-specific application icon setup
if(APPLE)
    # macOS bundle configuration for Dock icon
    set(APP_BUNDLE_ID "com.designopticsfast.phoenix")
    set(APP_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Phoenix.icns")
    
    # Add the .icns file to sources
    target_sources(phoenix_app PRIVATE ${APP_ICNS})
    
    # Set bundle properties
    set_target_properties(phoenix_app PROPERTIES
        MACOSX_BUNDLE TRUE
        OUTPUT_NAME "Phoenix"
        MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_ID}
        MACOSX_BUNDLE_ICON_FILE "Phoenix"   # basename only (no .icns)
    )
    
    # Put the .icns file in the bundle Resources/ dir
    set_source_files_properties(${APP_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    
elseif(WIN32)
    # Windows icon setup (when .ico file is available)
    set(WIN_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/Phoenix.ico")
    if(EXISTS ${WIN_ICON})
        target_sources(phoenix_app PRIVATE ${WIN_ICON})
        set_target_properties(phoenix_app PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()

# Phoenix standalone - no Bedrock includes for Sprint 4 Stage 1
# Resources are handled by AUTORCC with phoenix_icons.qrc

# Add translation files
# Note: Translation files are created but not compiled in this stage
# They will be processed in future phases when lupdate/lrelease are available

target_link_libraries(phoenix_app PRIVATE
  Qt6::Widgets
  Qt6::Concurrent
  Qt6::Core
  Qt6::Graphs
)

# Phoenix standalone - no Bedrock dependencies for Sprint 4 Stage 1

# Guard for missing macOS icon
if(APPLE AND NOT EXISTS ${CMAKE_SOURCE_DIR}/resources/macos/Phoenix.icns)
  message(FATAL_ERROR "Missing Phoenix.icns — run scripts/generate_macos_icon.sh on macOS.")
endif()

