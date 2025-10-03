cmake_minimum_required(VERSION 3.20)

project(Phoenix
  VERSION 0.0.1
  DESCRIPTION "Qt UI for Bedrock"
  LANGUAGES CXX)

# ---------------------------------------
# Toolchain / language
# ---------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------
# Qt
# ---------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ---------------------------------------
# Bring in Bedrock (sibling repo)
#   Expect layout: ~/Projects/phoenix and ~/Projects/bedrock
#   Builds Bedrock inside Phoenix build dir at _bedrock_build/
# ---------------------------------------
add_subdirectory(../bedrock ${CMAKE_BINARY_DIR}/_bedrock_build)

# ---------------------------------------
# Phoenix application target
#   If your app target already exists with a different name or source layout,
#   you can rename 'phoenix_app' and adjust the source list below.
# ---------------------------------------
add_executable(phoenix_app
  src/main.cpp                      # <-- adjust if your main is elsewhere
  src/adapters/bedrock_client.cpp
)

# ---------------------------------------
# Include paths
# ---------------------------------------
target_include_directories(phoenix_app
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ../bedrock/include              # Bedrock public headers
)

# ---------------------------------------
# Link libs
# ---------------------------------------
target_link_libraries(phoenix_app
  PRIVATE
    Qt6::Widgets
    bedrock_engine                  # brings bedrock_geom (OCCT) and bedrock_som
)

# ---------------------------------------
# macOS bundle niceties (optional)
# ---------------------------------------
if(APPLE)
  set_target_properties(phoenix_app PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "dev.phoenix.ui"
    MACOSX_BUNDLE_BUNDLE_NAME "Phoenix"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
  )
endif()

# ---------------------------------------
# Notes:
# 1) Ensure your environment has OCCT visible for Bedrock during configure:
#    export CMAKE_PREFIX_PATH="/opt/homebrew/opt/opencascade:${CMAKE_PREFIX_PATH}"
#    export OpenCASCADE_DIR="/opt/homebrew/opt/opencascade/lib/cmake/opencascade-7.9.1"
#
# 2) If your existing project already defines a different app target,
#    replace 'phoenix_app' with that target name and add:
#      target_sources(<your-target> PRIVATE src/adapters/bedrock_client.cpp)
#      target_include_directories(<your-target> PRIVATE ../bedrock/include)
#      target_link_libraries(<your-target> PRIVATE bedrock_engine)
# ---------------------------------------