cmake_minimum_required(VERSION 3.20)

project(Phoenix
  VERSION 0.0.3
  DESCRIPTION "Qt UI for Bedrock"
  LANGUAGES CXX)

# ---- C++ & Qt automations ---------------------------------------------------
# C++/Qt config (confirmed present)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# === Palantir integration toggle (default OFF for Sprint 4) ===
option(PHX_WITH_PALANTIR "Enable Palantir integration" OFF)

# === Developer tools toggle (default OFF for release builds) ===
option(PHX_DEV_TOOLS "Build developer tools" OFF)

# Make the flag visible to all targets
add_compile_definitions(
    $<$<BOOL:${PHX_WITH_PALANTIR}>:PHX_WITH_PALANTIR=1>
    $<$<NOT:$<BOOL:${PHX_WITH_PALANTIR}>>:PHX_WITH_PALANTIR=0>
)

# (Note: real/stub source selection will be added in a later chunk)

# ---- Version header (generated at configure time)
# Generates ${CMAKE_BINARY_DIR}/generated/version.h containing PHOENIX_VERSION
configure_file(${CMAKE_SOURCE_DIR}/src/version.h.in
               ${CMAKE_BINARY_DIR}/generated/version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated)

# ---- Qt packages (find these BEFORE defining targets) -----------------------
# Qt Graphs replaces old Qt Charts; ensure the component list matches your code.
find_package(Qt6 6.6 REQUIRED COMPONENTS Widgets Concurrent Core Graphs LinguistTools)

# ---- QML Debugging: explicitly disable -------------------------------------
# Ensure QML debugging macro is NOT defined even if the IDE injects it.
# Note: Defining QT_QML_DEBUG=0 still enables hooks; we must undefine it entirely.
add_compile_options(-UQT_QML_DEBUG)

# ---- Optional policy include (compiler/Qt-version aware) --------------------
# We include this AFTER Qt is found so the policy can use Qt6 variables/targets.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/policy_no_deprecated.cmake")
    include(policy_no_deprecated)
else()
    message(STATUS "Policy file not found - skipping deprecation checks")
endif()

# ---- Tools / helper targets (dev-only) --------------------------------------
if(PHX_DEV_TOOLS)
  # Icon audit tool (uses QtCore)
  add_executable(icon_audit tools/icon_audit.cpp)
  target_link_libraries(icon_audit PRIVATE Qt6::Core)
endif()

# ---- Phoenix sources --------------------------------------------------------
set(PHOENIX_SOURCES
  # sources
  src/main.cpp

  # Graphs / Vega-Lite infrastructure
  src/graphs/VegaLiteLocalizer.cpp
  src/graphs/VegaLiteLocalizer.hpp
  src/graphs/FormatUtils.hpp

  # Plot views
  src/plot/QtGraphsPlotView.cpp
  src/plot/QtGraphsPlotView.hpp

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
  src/ui/icons/PhxLogging.cpp
  src/ui/icons/IconBootstrap.cpp
  src/ui/icons/PhxIconImageProvider.cpp
  src/ui/UILogging.cpp

  # I/O utilities
  src/app/io/FileIO.cpp

  # I18n utilities
  src/app/LocaleInit.cpp
  src/app/LocaleInit.hpp
  src/app/I18nSelfTest.cpp
  src/app/I18nSelfTest.hpp
  src/app/MemoryMonitor.cpp
  src/app/MemoryMonitor.hpp

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

  # Qt resource file (AUTORCC)
  src/ui/icons/phoenix_icons.qrc
)

if(PHX_WITH_PALANTIR)
  list(APPEND PHOENIX_SOURCES
    src/palantir/PalantirClient.cpp
    src/palantir/PalantirLogging.cpp
    src/palantir/PalantirClient.hpp
  )
endif()

if(PHX_DEV_TOOLS)
  list(APPEND PHOENIX_SOURCES
    src/tools/IconGallery.cpp
    src/tools/IconGallery.h
  )
endif()

# ---- Phoenix application ----------------------------------------------------
add_executable(phoenix_app
  ${PHOENIX_SOURCES}
)

# ---- Translations ------------------------------------------------------------
set(PHOENIX_TS
  ${CMAKE_SOURCE_DIR}/translations/phoenix_en.ts
  ${CMAKE_SOURCE_DIR}/translations/phoenix_de.ts
)

qt_add_lupdate(
  LUPDATE_TARGET phoenix_update_ts
  TS_FILES ${PHOENIX_TS}
  SOURCES ${PHOENIX_SOURCES}
  OPTIONS -no-obsolete
)

qt_add_lrelease(
  LRELEASE_TARGET phoenix_qm
  TS_FILES ${PHOENIX_TS}
  QM_FILES_OUTPUT_VARIABLE PHOENIX_QM_FILES
)

add_custom_target(translations-update
  DEPENDS phoenix_update_ts
  COMMENT "Update translation catalogs (.ts)"
)

add_custom_target(translations
  DEPENDS phoenix_qm
  COMMENT "Compile translations (.ts -> .qm)"
)

if(TARGET phoenix_qm)
  add_dependencies(phoenix_app phoenix_qm)
else()
  message(WARNING "phoenix_qm target unavailable; translations must be built via the translations target")
endif()

if(APPLE)
  set(PHOENIX_TRANSLATIONS_INSTALL_DIR "Phoenix.app/Contents/Resources/translations" CACHE PATH "Install location for Phoenix translation files")
else()
  include(GNUInstallDirs)
  set(PHOENIX_TRANSLATIONS_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}/../translations" CACHE PATH "Install location for Phoenix translation files")
endif()

set(PHOENIX_RUNTIME_TRANSLATIONS_DIR "$<TARGET_FILE_DIR:phoenix_app>/translations")
if(APPLE)
  set(PHOENIX_RUNTIME_TRANSLATIONS_DIR "$<TARGET_FILE_DIR:phoenix_app>/../Resources/translations")
endif()

get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
if(QT_QMAKE_EXECUTABLE)
  get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
  get_filename_component(QT_INSTALL_PREFIX "${QT_BIN_DIR}/.." ABSOLUTE)
  set(QT_TRANSLATIONS_DIR "${QT_INSTALL_PREFIX}/translations")
endif()

function(wire_translations_for TARGET_NAME)
  if(NOT TARGET ${TARGET_NAME})
    return()
  endif()
  if(APPLE)
    set(_dst "$<TARGET_FILE_DIR:${TARGET_NAME}>/../Resources/translations")
  else()
    set(_dst "$<TARGET_FILE_DIR:${TARGET_NAME}>/translations")
  endif()
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_dst}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PHOENIX_QM_FILES} "${_dst}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_TRANSLATIONS_DIR}/qtbase_en.qm"
            "${QT_TRANSLATIONS_DIR}/qtbase_de.qm"
            "${_dst}"
  )
endfunction()

wire_translations_for(phoenix_app)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

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

if(WIN32)
  target_link_libraries(phoenix_app PRIVATE Psapi)
endif()

# Ensure QML debugging is disabled for the app target as well
target_compile_options(phoenix_app PRIVATE -UQT_QML_DEBUG)

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
