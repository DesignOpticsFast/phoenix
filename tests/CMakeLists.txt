find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(vega_localizer_test
  vega/VegaLiteLocalizer_test.cpp
  ${CMAKE_SOURCE_DIR}/src/graphs/VegaLiteLocalizer.cpp
)

target_link_libraries(vega_localizer_test
  PRIVATE
    Qt6::Test
    Qt6::Core
)

target_include_directories(vega_localizer_test
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME vega_localizer_test COMMAND vega_localizer_test)
