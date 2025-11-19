cmake_minimum_required(VERSION 3.20)

project(Phoenix
  VERSION 0.0.3
  DESCRIPTION "Qt UI for Bedrock"
  LANGUAGES CXX C)

# ---- C++ & Qt automations ---------------------------------------------------
# C++/Qt config (confirmed present)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# === Developer tools toggle (default OFF for release builds) ===
option(PHX_DEV_TOOLS "Build developer tools" OFF)

# === libsodium crypto support (default ON) ===
option(PHX_WITH_LIBSODIUM "Build with libsodium crypto support" ON)

# === Palantir contracts submodule support (default ON) ===
option(PHX_WITH_PALANTIR_CONTRACTS "Enable Palantir contract stub generation" ON)

# === Transport dependencies (gRPC + Protobuf) support (default ON) ===
option(PHX_WITH_TRANSPORT_DEPS "Build Phoenix with gRPC and Protobuf transport" ON)

# (Note: real/stub source selection will be added in a later chunk)

# ---- Version header (generated at configure time)
# Generates ${CMAKE_BINARY_DIR}/generated/version.h containing PHOENIX_VERSION
configure_file(${CMAKE_SOURCE_DIR}/src/version.h.in
               ${CMAKE_BINARY_DIR}/generated/version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated)

# ---- Qt packages (find these BEFORE defining targets) -----------------------
# Qt Graphs replaces old Qt Charts; ensure the component list matches your code.

# Helpful warning for macOS developers if CMAKE_PREFIX_PATH is not set
if(APPLE)
    if(NOT CMAKE_PREFIX_PATH AND NOT Qt6_DIR)
        message(WARNING "CMAKE_PREFIX_PATH not set. On macOS, you probably want: -DCMAKE_PREFIX_PATH=/path/to/Qt/6.10.0/macos")
    endif()
endif()

find_package(Qt6 6.10 REQUIRED COMPONENTS Widgets Concurrent Core Network Graphs GraphsWidgets QuickWidgets LinguistTools PrintSupport)

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
  src/ui/dialogs/AnalysisPage.cpp
  src/ui/dialogs/LicenseDialog.cpp
  src/ui/dialogs/EchoTestDialog.cpp

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
  src/ui/dialogs/AnalysisPage.h
  src/ui/dialogs/LicenseDialog.h
  src/ui/dialogs/EchoTestDialog.h

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

# ---- libsodium library ----------------------------------------------------
if(PHX_WITH_LIBSODIUM)
  # Collect all libsodium source files
  file(GLOB_RECURSE LIBSODIUM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium/src/libsodium/*.c"
  )
  list(LENGTH LIBSODIUM_SOURCES LIBSODIUM_SOURCES_COUNT)
  message(STATUS "Found ${LIBSODIUM_SOURCES_COUNT} libsodium source files")
  if(LIBSODIUM_SOURCES_COUNT EQUAL 0)
    message(WARNING "No libsodium source files found! Check path: ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium/src/libsodium/")
  endif()

  add_library(phoenix_libsodium STATIC
    ${LIBSODIUM_SOURCES}
  )

  target_include_directories(phoenix_libsodium PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium/src/libsodium/include
  )
  
  # Libsodium source files need include paths for headers
  # include/sodium for core.h etc, and include for sodium/version.h
  target_include_directories(phoenix_libsodium PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium/src/libsodium/include/sodium
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium/src/libsodium/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium/src/libsodium
  )

  # Platform-specific compilation flags
  if(WIN32)
    # Windows-specific flags for libsodium
    target_compile_definitions(phoenix_libsodium PRIVATE
      SODIUM_STATIC=1
    )
  endif()

  if(APPLE)
    # macOS-specific flags if needed
    target_compile_definitions(phoenix_libsodium PRIVATE
      SODIUM_STATIC=1
    )
  endif()

  if(UNIX AND NOT APPLE)
    # Linux-specific flags
    target_compile_definitions(phoenix_libsodium PRIVATE
      SODIUM_STATIC=1
    )
  endif()
endif()

# ---- canonical_json library ----------------------------------------------------
add_library(phoenix_canonical_json STATIC
  src/common/canonical_json.cpp
)

target_include_directories(phoenix_canonical_json PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(phoenix_canonical_json PUBLIC
  Qt6::Core
)

# ---- gRPC and Protobuf dependencies (for transport) ------------------------
if(PHX_WITH_TRANSPORT_DEPS)
find_package(Protobuf CONFIG QUIET)
set(Protobuf_FOUND_CONFIG ${Protobuf_FOUND})
if(NOT Protobuf_FOUND)
    find_package(Protobuf MODULE QUIET)
    set(Protobuf_FOUND_MODULE ${Protobuf_FOUND})
endif()

message(STATUS "Protobuf CONFIG found: ${Protobuf_FOUND_CONFIG}, MODULE found: ${Protobuf_FOUND_MODULE}, Final: ${Protobuf_FOUND}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}, Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")

# Reset Protobuf_FOUND if we don't have the necessary components
# Check if LIBRARIES is empty string (CMake treats empty string as falsy)
set(PROTOBUF_WAS_FOUND ${Protobuf_FOUND})
if(Protobuf_FOUND)
    if("${Protobuf_LIBRARIES}" STREQUAL "" OR "${Protobuf_INCLUDE_DIRS}" STREQUAL "")
        message(STATUS "Protobuf_FOUND is TRUE but missing LIBRARIES or INCLUDE_DIRS, resetting")
        set(Protobuf_FOUND FALSE CACHE BOOL "Protobuf found" FORCE)
    else()
        message(STATUS "Protobuf is properly configured with LIBRARIES and INCLUDE_DIRS")
        # Explicitly ensure it stays TRUE
        set(Protobuf_FOUND TRUE CACHE BOOL "Protobuf found" FORCE)
    endif()
endif()

# If Protobuf still not found, try manual detection
if(NOT Protobuf_FOUND)
    message(STATUS "Protobuf not found via find_package, trying manual detection")
    # Check for protoc
    find_program(PROTOC protoc)
    message(STATUS "PROTOC found: ${PROTOC}")
    if(PROTOC)
        # Try to find protobuf library
        find_library(PROTOBUF_LIB protobuf PATHS /usr/lib64 /usr/lib)
        message(STATUS "PROTOBUF_LIB: ${PROTOBUF_LIB}")
        if(PROTOBUF_LIB)
            set(Protobuf_FOUND TRUE)
            set(Protobuf_LIBRARIES ${PROTOBUF_LIB})
            # Try common include paths
            if(EXISTS "/usr/include/google/protobuf")
                set(Protobuf_INCLUDE_DIRS "/usr/include")
            elseif(EXISTS "/usr/local/include/google/protobuf")
                set(Protobuf_INCLUDE_DIRS "/usr/local/include")
            else()
                # Use pkg-config to get include dirs
                execute_process(
                    COMMAND pkg-config --cflags-only-I protobuf
                    OUTPUT_VARIABLE PROTOBUF_CFLAGS
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                )
                string(REGEX REPLACE "-I" "" Protobuf_INCLUDE_DIRS "${PROTOBUF_CFLAGS}")
                string(STRIP "${Protobuf_INCLUDE_DIRS}" Protobuf_INCLUDE_DIRS)
            endif()
            message(STATUS "Found Protobuf manually: ${PROTOBUF_LIB}")
            # Explicitly set FOUND flag
            set(Protobuf_FOUND TRUE CACHE BOOL "Protobuf found" FORCE)
            # Create imported target
            if(NOT TARGET protobuf::libprotobuf)
                add_library(protobuf::libprotobuf INTERFACE IMPORTED)
                set_target_properties(protobuf::libprotobuf PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${Protobuf_INCLUDE_DIRS}"
                    INTERFACE_LINK_LIBRARIES "${Protobuf_LIBRARIES}"
                )
            endif()
        endif()
    endif()
endif()

find_package(gRPC CONFIG QUIET)
set(gRPC_FOUND_CONFIG ${gRPC_FOUND})
if(NOT gRPC_FOUND)
    find_package(gRPC MODULE QUIET)
    set(gRPC_FOUND_MODULE ${gRPC_FOUND})
endif()

message(STATUS "gRPC CONFIG found: ${gRPC_FOUND_CONFIG}, MODULE found: ${gRPC_FOUND_MODULE}, Final: ${gRPC_FOUND}")
message(STATUS "gRPC_LIBRARIES: ${gRPC_LIBRARIES}, gRPC_INCLUDE_DIRS: ${gRPC_INCLUDE_DIRS}")

# Reset gRPC_FOUND if we don't have the necessary components
# Check if LIBRARIES is empty string (CMake treats empty string as falsy)
set(GRPC_WAS_FOUND ${gRPC_FOUND})
if(gRPC_FOUND)
    if("${gRPC_LIBRARIES}" STREQUAL "" OR "${gRPC_INCLUDE_DIRS}" STREQUAL "")
        message(STATUS "gRPC_FOUND is TRUE but missing LIBRARIES or INCLUDE_DIRS, resetting")
        set(gRPC_FOUND FALSE CACHE BOOL "gRPC found" FORCE)
    else()
        message(STATUS "gRPC is properly configured with LIBRARIES and INCLUDE_DIRS")
        # Explicitly ensure it stays TRUE
        set(gRPC_FOUND TRUE CACHE BOOL "gRPC found" FORCE)
    endif()
endif()

# If gRPC still not found, try manual detection
if(NOT gRPC_FOUND)
    message(STATUS "gRPC not found via find_package, trying manual detection")
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    message(STATUS "GRPC_CPP_PLUGIN found: ${GRPC_CPP_PLUGIN}")
    if(GRPC_CPP_PLUGIN)
        # Try to find gRPC libraries
        find_library(GRPCXX_LIB grpc++ PATHS /usr/lib64 /usr/lib)
        find_library(GRPC_LIB grpc PATHS /usr/lib64 /usr/lib)
        message(STATUS "GRPCXX_LIB: ${GRPCXX_LIB}, GRPC_LIB: ${GRPC_LIB}")
        if(GRPCXX_LIB AND GRPC_LIB)
            set(gRPC_FOUND TRUE)
            set(gRPC_LIBRARIES ${GRPCXX_LIB} ${GRPC_LIB})
            # Try common include paths
            if(EXISTS "/usr/include/grpcpp/grpcpp.h")
                set(gRPC_INCLUDE_DIRS "/usr/include")
            elseif(EXISTS "/usr/local/include/grpcpp/grpcpp.h")
                set(gRPC_INCLUDE_DIRS "/usr/local/include")
            else()
                # Use pkg-config to get include dirs
                execute_process(
                    COMMAND pkg-config --cflags-only-I grpc++
                    OUTPUT_VARIABLE GRPC_CFLAGS
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                )
                string(REGEX REPLACE "-I" "" gRPC_INCLUDE_DIRS "${GRPC_CFLAGS}")
                string(STRIP "${gRPC_INCLUDE_DIRS}" gRPC_INCLUDE_DIRS)
            endif()
            message(STATUS "Found gRPC manually: ${GRPCXX_LIB}")
            # Explicitly set FOUND flag
            set(gRPC_FOUND TRUE CACHE BOOL "gRPC found" FORCE)
            # Create imported target
            if(NOT TARGET gRPC::grpc++)
                add_library(gRPC::grpc++ INTERFACE IMPORTED)
                set_target_properties(gRPC::grpc++ PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${gRPC_INCLUDE_DIRS}"
                    INTERFACE_LINK_LIBRARIES "${gRPC_LIBRARIES}"
                )
            endif()
        endif()
    endif()
endif()

# Proto generation for bedrock_echo.proto
set(ECHO_PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto/bedrock_echo.proto)
set(ECHO_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated/echo)
file(MAKE_DIRECTORY ${ECHO_GENERATED_DIR})

# Proto generation for palantir.proto (LocalSocket transport)
set(PALANTIR_PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto/palantir.proto)
set(PALANTIR_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated/palantir)
file(MAKE_DIRECTORY ${PALANTIR_GENERATED_DIR})

# Determine if we have working Protobuf and gRPC based on actual library availability
set(HAVE_PROTOBUF FALSE)
set(HAVE_GRPC FALSE)

if(Protobuf_LIBRARIES AND Protobuf_INCLUDE_DIRS)
    set(HAVE_PROTOBUF TRUE)
    message(STATUS "Protobuf available: ${Protobuf_LIBRARIES}")
else()
    message(STATUS "Protobuf not available (missing LIBRARIES or INCLUDE_DIRS)")
endif()

if(gRPC_LIBRARIES AND gRPC_INCLUDE_DIRS)
    set(HAVE_GRPC TRUE)
    message(STATUS "gRPC available: ${gRPC_LIBRARIES}")
else()
    message(STATUS "gRPC not available (missing LIBRARIES or INCLUDE_DIRS)")
endif()

message(STATUS "Final check - HAVE_PROTOBUF: ${HAVE_PROTOBUF}, HAVE_GRPC: ${HAVE_GRPC}")

# Proto generation for bedrock_echo.proto
set(ECHO_PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto/bedrock_echo.proto)
set(ECHO_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated/echo)
file(MAKE_DIRECTORY ${ECHO_GENERATED_DIR})

# Proto generation for palantir.proto (LocalSocket transport)
set(PALANTIR_PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto/palantir.proto)
set(PALANTIR_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated/palantir)
file(MAKE_DIRECTORY ${PALANTIR_GENERATED_DIR})

# Determine if we have working Protobuf and gRPC based on actual library availability
set(HAVE_PROTOBUF FALSE)
set(HAVE_GRPC FALSE)

if(Protobuf_LIBRARIES AND Protobuf_INCLUDE_DIRS)
    set(HAVE_PROTOBUF TRUE)
    message(STATUS "Protobuf available: ${Protobuf_LIBRARIES}")
else()
    message(STATUS "Protobuf not available (missing LIBRARIES or INCLUDE_DIRS)")
endif()

if(gRPC_LIBRARIES AND gRPC_INCLUDE_DIRS)
    set(HAVE_GRPC TRUE)
    message(STATUS "gRPC available: ${gRPC_LIBRARIES}")
else()
    message(STATUS "gRPC not available (missing LIBRARIES or INCLUDE_DIRS)")
endif()

message(STATUS "Final check - HAVE_PROTOBUF: ${HAVE_PROTOBUF}, HAVE_GRPC: ${HAVE_GRPC}")

if(HAVE_PROTOBUF AND HAVE_GRPC)
    # Find protoc and grpc_cpp_plugin (may already be found)
    if(NOT PROTOC)
        find_program(PROTOC protoc)
    endif()
    if(NOT GRPC_CPP_PLUGIN)
        find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    endif()
    
    if(PROTOC AND GRPC_CPP_PLUGIN)
        # Generate protobuf and gRPC stubs for bedrock_echo.proto
        set(ECHO_PROTO_SRCS ${ECHO_GENERATED_DIR}/bedrock_echo.pb.cc)
        set(ECHO_PROTO_HDRS ${ECHO_GENERATED_DIR}/bedrock_echo.pb.h)
        set(ECHO_GRPC_SRCS ${ECHO_GENERATED_DIR}/bedrock_echo.grpc.pb.cc)
        set(ECHO_GRPC_HDRS ${ECHO_GENERATED_DIR}/bedrock_echo.grpc.pb.h)
        
        add_custom_command(
            OUTPUT ${ECHO_PROTO_SRCS} ${ECHO_PROTO_HDRS} ${ECHO_GRPC_SRCS} ${ECHO_GRPC_HDRS}
            COMMAND ${PROTOC}
            ARGS --cpp_out=${ECHO_GENERATED_DIR}
                 --grpc_out=${ECHO_GENERATED_DIR}
                 --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                 --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
                 ${ECHO_PROTO_FILE}
            DEPENDS ${ECHO_PROTO_FILE}
            COMMENT "Generating gRPC stubs for bedrock_echo.proto"
        )
        
        message(STATUS "Proto generation configured for bedrock_echo.proto")
        message(STATUS "Generated files will be in: ${ECHO_GENERATED_DIR}")
    else()
        message(WARNING "protoc or grpc_cpp_plugin not found - gRPC transport will not be available")
        set(ECHO_PROTO_SRCS "")
        set(ECHO_PROTO_HDRS "")
        set(ECHO_GRPC_SRCS "")
        set(ECHO_GRPC_HDRS "")
    endif()
else()
    message(WARNING "Protobuf or gRPC not found - gRPC transport will not be available")
    set(ECHO_PROTO_SRCS "")
    set(ECHO_PROTO_HDRS "")
    set(ECHO_GRPC_SRCS "")
    set(ECHO_GRPC_HDRS "")
endif()

# Palantir proto generation (protobuf only, no gRPC)
if(HAVE_PROTOBUF)
    if(NOT PROTOC)
        find_program(PROTOC protoc)
    endif()
    
    if(PROTOC)
        # Generate protobuf stubs for palantir.proto
        set(PALANTIR_PROTO_SRCS ${PALANTIR_GENERATED_DIR}/palantir.pb.cc)
        set(PALANTIR_PROTO_HDRS ${PALANTIR_GENERATED_DIR}/palantir.pb.h)
        
        add_custom_command(
            OUTPUT ${PALANTIR_PROTO_SRCS} ${PALANTIR_PROTO_HDRS}
            COMMAND ${PROTOC}
            ARGS --cpp_out=${PALANTIR_GENERATED_DIR}
                 --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
                 ${PALANTIR_PROTO_FILE}
            DEPENDS ${PALANTIR_PROTO_FILE}
            COMMENT "Generating protobuf stubs for palantir.proto"
        )
        
        message(STATUS "Proto generation configured for palantir.proto")
        message(STATUS "Generated files will be in: ${PALANTIR_GENERATED_DIR}")
    else()
        message(WARNING "protoc not found - LocalSocket transport will not be available")
        set(PALANTIR_PROTO_SRCS "")
        set(PALANTIR_PROTO_HDRS "")
    endif()
else()
    message(WARNING "Protobuf not found - LocalSocket transport will not be available")
    set(PALANTIR_PROTO_SRCS "")
    set(PALANTIR_PROTO_HDRS "")
endif()
else()
    # Transport disabled - set all flags to FALSE and clear proto vars
    set(HAVE_PROTOBUF FALSE)
    set(HAVE_GRPC FALSE)
    set(ECHO_PROTO_SRCS "")
    set(ECHO_PROTO_HDRS "")
    set(ECHO_GRPC_SRCS "")
    set(ECHO_GRPC_HDRS "")
    set(PALANTIR_PROTO_SRCS "")
    set(PALANTIR_PROTO_HDRS "")
    message(WARNING "Building Phoenix WITHOUT transport (gRPC/Protobuf). Only demo/local analyses will work.")
endif()

# ---- transport library ----------------------------------------------------
if(PHX_WITH_TRANSPORT_DEPS)
add_library(phoenix_transport STATIC
  src/transport/TransportClient.hpp
  src/transport/GrpcUdsChannel.hpp
  src/transport/GrpcUdsChannel.cpp
  src/transport/LocalSocketChannel.hpp
  src/transport/LocalSocketChannel.cpp
  src/transport/TransportFactory.hpp
  src/transport/TransportFactory.cpp
  ${ECHO_PROTO_SRCS}
  ${ECHO_GRPC_SRCS}
  ${PALANTIR_PROTO_SRCS}
)

target_include_directories(phoenix_transport PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${ECHO_GENERATED_DIR}
  ${PALANTIR_GENERATED_DIR}
)

target_link_libraries(phoenix_transport PUBLIC
  Qt6::Core
  Qt6::Network
)

# Link Protobuf (needed for both gRPC and LocalSocket)
if(HAVE_PROTOBUF)
    if(TARGET protobuf::libprotobuf)
        target_link_libraries(phoenix_transport PRIVATE protobuf::libprotobuf)
    else()
        target_link_libraries(phoenix_transport PRIVATE ${Protobuf_LIBRARIES})
    endif()
    
    # Add include directories for protobuf
    if(Protobuf_INCLUDE_DIRS)
        target_include_directories(phoenix_transport PRIVATE ${Protobuf_INCLUDE_DIRS})
    endif()
endif()

# Link gRPC (only needed for gRPC transport)
if(HAVE_PROTOBUF AND HAVE_GRPC AND PROTOC AND GRPC_CPP_PLUGIN)
    if(TARGET gRPC::grpc++)
        target_link_libraries(phoenix_transport PRIVATE gRPC::grpc++)
    else()
        target_link_libraries(phoenix_transport PRIVATE ${gRPC_LIBRARIES})
    endif()
    
    # Add include directories for gRPC
    if(gRPC_INCLUDE_DIRS)
        target_include_directories(phoenix_transport PRIVATE ${gRPC_INCLUDE_DIRS})
    endif()
endif()
else()
    # Transport disabled - create empty stub library to satisfy dependencies
    add_library(phoenix_transport INTERFACE)
    message(STATUS "Transport library disabled - creating stub interface library")
endif()

# ---- license library ----------------------------------------------------
add_library(phoenix_license STATIC
  src/license/License.hpp
  src/license/License.cpp
  src/license/LicenseVerifier.hpp
  src/license/LicenseVerifier.cpp
)

target_include_directories(phoenix_license PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(phoenix_license PUBLIC
  Qt6::Core
  phoenix_canonical_json
)

if(PHX_WITH_LIBSODIUM)
  target_link_libraries(phoenix_license PRIVATE
    phoenix_libsodium
  )
  # Ensure libsodium include directories are available
  target_include_directories(phoenix_license PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium/src/libsodium/include
  )
endif()

# ---- license manager library ----------------------------------------------------
add_library(phoenix_license_manager STATIC
  src/app/LicenseManager.cpp
)

target_include_directories(phoenix_license_manager PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(phoenix_license_manager PUBLIC
  phoenix_license
  Qt6::Core
)

# ---- feature registry library --------------------------------------------
add_library(phoenix_feature_registry STATIC
  src/features/ParamSpec.hpp
  src/features/ParamSpec.cpp
  src/features/FeatureDescriptor.hpp
  src/features/FeatureDescriptor.cpp
  src/features/FeatureRegistry.hpp
  src/features/FeatureRegistry.cpp
)

target_include_directories(phoenix_feature_registry PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(phoenix_feature_registry PUBLIC
  Qt6::Core
)

# ---- analysis library ----------------------------------------------------
add_library(phoenix_analysis STATIC
  src/ui/analysis/IAnalysisView.hpp
  src/ui/analysis/AnalysisWindow.cpp
  src/ui/analysis/AnalysisWindow.hpp
  src/ui/widgets/FeatureParameterPanel.cpp
  src/ui/widgets/FeatureParameterPanel.hpp
  src/analysis/AnalysisWorker.cpp
  src/analysis/AnalysisWorker.hpp
  src/analysis/AnalysisProgress.hpp
  src/analysis/demo/XYSineDemo.cpp
  src/plot/XYPlotViewGraphs.cpp
  src/plot/XYPlotViewGraphs.hpp
  src/qml/phoenix_qml.qrc
)

target_include_directories(phoenix_analysis PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(phoenix_analysis PUBLIC
  phoenix_feature_registry
  phoenix_license_manager
  Qt6::Core
  Qt6::Widgets
  Qt6::Graphs
  Qt6::GraphsWidgets
  Qt6::QuickWidgets
)

# Conditionally link transport (only when enabled)
if(PHX_WITH_TRANSPORT_DEPS)
  target_link_libraries(phoenix_analysis PUBLIC phoenix_transport)
endif()

# ---- Palantir contracts proto generation scaffolding ---------------------------
if(PHX_WITH_PALANTIR_CONTRACTS)
  # Discover proto files in contracts/ submodule
  file(GLOB_RECURSE PALANTIR_PROTO_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/contracts/*.proto"
  )

  if(PALANTIR_PROTO_FILES)
    # Future: proto generation will go here
    # Set up generated code directory
    set(PALANTIR_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated/contracts)
    file(MAKE_DIRECTORY ${PALANTIR_GENERATED_DIR})

    # Find protoc and grpc_cpp_plugin (for future use)
    find_program(PROTOC protoc)
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)

    if(PROTOC AND GRPC_CPP_PLUGIN)
      message(STATUS "Palantir proto files found: ${PALANTIR_PROTO_FILES}")
      message(STATUS "Proto generation scaffolding ready (protoc: ${PROTOC}, grpc_cpp_plugin: ${GRPC_CPP_PLUGIN})")
      message(STATUS "Generated code will be placed in: ${PALANTIR_GENERATED_DIR}")
      # TODO: When .proto files exist, add code generation here
    else()
      if(NOT PROTOC)
        message(WARNING "protoc not found - proto generation will not be available")
      endif()
      if(NOT GRPC_CPP_PLUGIN)
        message(WARNING "grpc_cpp_plugin not found - proto generation will not be available")
      endif()
    endif()
  else()
    message(STATUS "No Palantir .proto files found under contracts/. Skipping stub generation.")
  endif()
endif()

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
    phoenix_feature_registry
    phoenix_analysis
    phoenix_license_manager
)

# Conditionally link transport (only when enabled)
if(PHX_WITH_TRANSPORT_DEPS)
  target_link_libraries(phoenix_app PRIVATE phoenix_transport)
endif()

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
      message(FATAL_ERROR
        "Missing Phoenix.icns — required for macOS app bundle.\n"
        "To generate the icon:\n"
        "  1. Run: scripts/generate_macos_icon.sh\n"
        "  2. Or manually create: resources/macos/Phoenix.icns\n"
        "Expected location: ${APP_ICNS}")
    endif()

elseif (WIN32)
    # Windows icon if available
    set(WIN_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/Phoenix.ico")
    if (EXISTS "${WIN_ICON}")
        target_sources(phoenix_app PRIVATE ${WIN_ICON})
        set_target_properties(phoenix_app PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()
