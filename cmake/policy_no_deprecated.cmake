# cmake/policy_no_deprecated.cmake
# Policy enforcement for "No Deprecated Libraries"
# This file must be included at the top of every repository's top-level CMakeLists.txt

# Check for Qt Charts (deprecated) - FATAL ERROR if found
if (Qt6Charts_FOUND)
  message(FATAL_ERROR "QtCharts is deprecated and forbidden. Use Qt Graphs instead.")
endif()

# Check for other deprecated Qt modules
if (Qt6WebEngine_FOUND)
  message(FATAL_ERROR "Qt6WebEngine is deprecated and forbidden.")
endif()

if (Qt6WebEngineWidgets_FOUND)
  message(FATAL_ERROR "Qt6WebEngineWidgets is deprecated and forbidden.")
endif()

if (Qt6WebView_FOUND)
  message(FATAL_ERROR "Qt6WebView is deprecated and forbidden.")
endif()

if (Qt6Xml_FOUND)
  message(FATAL_ERROR "Qt6Xml is deprecated and forbidden. Use Qt6Core instead.")
endif()

if (Qt6XmlPatterns_FOUND)
  message(FATAL_ERROR "Qt6XmlPatterns is deprecated and forbidden.")
endif()

# Set Qt deprecation baseline to 6.10.0
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x060A00)

# Enable deprecation warnings as errors
add_compile_options(-Werror=deprecated-declarations)

# Log policy enforcement
message(STATUS "Policy: No deprecated libraries enforced")
message(STATUS "Qt deprecation baseline: 6.10.0")
message(STATUS "Deprecation warnings: treated as errors")







