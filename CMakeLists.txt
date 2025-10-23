cmake_minimum_required(VERSION 3.20)

project(Phoenix
  VERSION 0.0.2
  DESCRIPTION "Qt UI for Bedrock"
  LANGUAGES CXX)

# ---- C++ & Qt automations ---------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---- Version header (generated at configure time)
# Generates ${CMAKE_BINARY_DIR}/generated/version.h containing PHOENIX_VERSION
configure_file(${CMAKE_SOURCE_DIR}/src/version.h.in
               ${CMAKE_BINARY_DIR}/generated/version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated)

# ---- Qt packages (find these BEFORE defining targets) -----------------------
# Qt Graphs replaces old Qt Charts; ensure the component list matches your code.
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent Core Graphs)

# ---- Optional policy include (compiler/Qt-version aware) --------------------
# We include this AFTER Qt is found so the policy can use Qt6 variables/targets.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/policy_no_deprecated.cmake")
    include(policy_no_deprecated)
else()
    message(STATUS "Policy file not found - skipping deprecation checks")
endif()

# ---- Tools / helper targets -------------------------------------------------
# Icon audit tool (uses QtCore)
add_executable(icon_audit tools/icon_audit.cpp)
target_link_libraries(icon_audit PRIVATE Qt6::Core)

# ---- Phoenix application ----------------------------------------------------
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
          src/ui/icons/PhxLogCategories.cpp
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

  # non-Q_OBJECT headers
  src/ui/icons/IconProvider.h
  src/ui/icons/IconBootstrap.h
  src/ui/icons/PhxIconImageProvider.h
  src/ui/MainWindow.ui

  # Qt resource file (AUTORCC)
  src/ui/icons/phoenix_icons.qrc

  # macOS bundle icon (defined below on APPLE)
  $<$<PLATFORM_ID:Darwin>:${APP_ICNS}>
)

# Include paths for the app
target_include_directories(phoenix_app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt libs
target_link_libraries(phoenix_app
  PRIVATE
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Core
    Qt6::Graphs
)

# ---- Platform-specific app icon/bundle settings -----------------------------
if (APPLE)
    # macOS bundle configuration for Dock icon
    set(APP_BUNDLE_ID "com.designopticsfast.phoenix")
    set(APP_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Phoenix.icns")

    # Add the .icns file to sources and mark location in the bundle
    if (EXISTS "${APP_ICNS}")
      target_sources(phoenix_app PRIVATE ${APP_ICNS})
      set_source_files_properties(${APP_ICNS}
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    endif()

    set_target_properties(phoenix_app PROPERTIES
        MACOSX_BUNDLE TRUE
        OUTPUT_NAME "Phoenix"
        MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_ID}
        MACOSX_BUNDLE_ICON_FILE "Phoenix"   # basename only (no .icns)
    )

    # Guard for missing macOS icon
    if (NOT EXISTS "${APP_ICNS}")
      message(FATAL_ERROR "Missing Phoenix.icns — run scripts/generate_macos_icon.sh on macOS.")
    endif()

elseif (WIN32)
    # Windows icon if available
    set(WIN_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/Phoenix.ico")
    if (EXISTS "${WIN_ICON}")
        target_sources(phoenix_app PRIVATE ${WIN_ICON})
        set_target_properties(phoenix_app PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()
