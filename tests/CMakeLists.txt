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

# libsodium Ed25519 sanity test
if(BUILD_TESTING AND PHX_WITH_LIBSODIUM)
  add_executable(libsodium_ed25519_sanity
    libsodium_ed25519_sanity.cpp
  )

  target_link_libraries(libsodium_ed25519_sanity
    PRIVATE
      phoenix_libsodium
  )

  add_test(NAME libsodium_ed25519_sanity COMMAND libsodium_ed25519_sanity)
endif()

# canonical_json tests
if(BUILD_TESTING)
  add_executable(canonical_json_tests
    canonical_json_tests.cpp
  )

  target_link_libraries(canonical_json_tests
    PRIVATE
      phoenix_canonical_json
      Qt6::Test
      Qt6::Core
  )

  target_include_directories(canonical_json_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME canonical_json_tests COMMAND canonical_json_tests)
endif()

# transport sanity tests
if(BUILD_TESTING)
  add_executable(transport_sanity_tests
    transport_sanity_tests.cpp
  )

  target_link_libraries(transport_sanity_tests
    PRIVATE
      phoenix_transport
      Qt6::Test
      Qt6::Core
  )

  target_include_directories(transport_sanity_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME transport_sanity_tests COMMAND transport_sanity_tests)
endif()

# XY plot autoscaling tests (Phoenix-only, no transport dependencies)
if(BUILD_TESTING)
  add_executable(test_xyplot_autoscale
    test_xyplot_autoscale.cpp
  )

  target_link_libraries(test_xyplot_autoscale PRIVATE
    phoenix_analysis
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    Qt6::QuickWidgets
  )

  target_include_directories(test_xyplot_autoscale
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME test_xyplot_autoscale COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_xyplot_autoscale>)
endif()
# XY analysis window creation tests (Phoenix-only)
if(BUILD_TESTING)
  add_executable(test_analysis_window_creation
    test_analysis_window_creation.cpp
  )

  target_link_libraries(test_analysis_window_creation PRIVATE
    phoenix_analysis
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    Qt6::QuickWidgets
  )

  target_include_directories(test_analysis_window_creation
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME test_analysis_window_creation COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_analysis_window_creation>)
endif()
# analysis sanity tests
if(BUILD_TESTING)
  add_executable(analysis_sanity_tests
    analysis_sanity_tests.cpp
  )

  target_link_libraries(analysis_sanity_tests
    PRIVATE
      phoenix_analysis
      Qt6::Test
      Qt6::Core
      Qt6::Widgets
      Qt6::PrintSupport
  )

  target_include_directories(analysis_sanity_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME analysis_sanity_tests COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:analysis_sanity_tests>)
endif()

# graphs performance sanity test
if(BUILD_TESTING)
  add_executable(graphs_perf_sanity
    graphs_perf_sanity.cpp
  )

  target_link_libraries(graphs_perf_sanity
    PRIVATE
      phoenix_analysis
      Qt6::Test
      Qt6::Core
      Qt6::Widgets
      Qt6::QuickWidgets
  )

  target_include_directories(graphs_perf_sanity
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME graphs_perf_sanity COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:graphs_perf_sanity>)
endif()
