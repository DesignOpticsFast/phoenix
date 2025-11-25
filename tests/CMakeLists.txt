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

# transport sanity tests (removed for Phase 2A - Phoenix-only phase)
# Will be restored in Phase 3 (transport scaffolding)
# if(BUILD_TESTING AND PHX_WITH_TRANSPORT_DEPS)
#   add_executable(transport_sanity_tests ...)
# endif()

# envelope helpers tests (Sprint 4.5 - Workstream 1)
if(BUILD_TESTING AND PHX_WITH_TRANSPORT_DEPS)
  add_executable(envelope_helpers_test
    envelope_helpers_test.cpp
  )

  target_link_libraries(envelope_helpers_test PRIVATE
    phoenix_transport
    phoenix_palantir_proto
    Qt6::Test
    Qt6::Core
  )
  
  # Link abseil libraries (required by protobuf 6.33+)
  find_library(ABSL_DIE_IF_NULL_LIB absl_die_if_null PATHS /opt/homebrew/opt/abseil/lib NO_DEFAULT_PATH)
  find_library(ABSL_LOG_INITIALIZE_LIB absl_log_initialize PATHS /opt/homebrew/opt/abseil/lib NO_DEFAULT_PATH)
  find_library(ABSL_STATUSOR_LIB absl_statusor PATHS /opt/homebrew/opt/abseil/lib NO_DEFAULT_PATH)
  find_library(ABSL_LOG_INTERNAL_CHECK_OP_LIB absl_log_internal_check_op PATHS /opt/homebrew/opt/abseil/lib NO_DEFAULT_PATH)
  find_library(ABSL_LOG_INTERNAL_CONDITIONS_LIB absl_log_internal_conditions PATHS /opt/homebrew/opt/abseil/lib NO_DEFAULT_PATH)
  find_library(ABSL_LOG_INTERNAL_MESSAGE_LIB absl_log_internal_message PATHS /opt/homebrew/opt/abseil/lib NO_DEFAULT_PATH)
  find_library(ABSL_HASH_LIB absl_hash PATHS /opt/homebrew/opt/abseil/lib NO_DEFAULT_PATH)
  
  if(ABSL_DIE_IF_NULL_LIB AND ABSL_LOG_INITIALIZE_LIB AND ABSL_STATUSOR_LIB)
    target_link_libraries(envelope_helpers_test PRIVATE
      ${ABSL_DIE_IF_NULL_LIB}
      ${ABSL_LOG_INITIALIZE_LIB}
      ${ABSL_STATUSOR_LIB}
      ${ABSL_LOG_INTERNAL_CHECK_OP_LIB}
      ${ABSL_LOG_INTERNAL_CONDITIONS_LIB}
      ${ABSL_LOG_INTERNAL_MESSAGE_LIB}
    )
    if(ABSL_HASH_LIB)
      target_link_libraries(envelope_helpers_test PRIVATE ${ABSL_HASH_LIB})
    endif()
  endif()

  target_include_directories(envelope_helpers_test
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/..
  )

  target_compile_definitions(envelope_helpers_test PRIVATE PHX_WITH_TRANSPORT_DEPS)

  add_test(NAME envelope_helpers_test COMMAND envelope_helpers_test)
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
    phoenix_feature_registry
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

# XY Plot instrumentation test (Phase 2D)
if(BUILD_TESTING)
  add_executable(test_xy_plot_instrumentation
    test_xy_plot_instrumentation.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/widgets/FeatureParameterPanel.cpp
  )

  target_link_libraries(test_xy_plot_instrumentation PRIVATE
    phoenix_analysis
    phoenix_feature_registry
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
    Qt6::QuickWidgets
  )

  target_include_directories(test_xy_plot_instrumentation
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME test_xy_plot_instrumentation COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen PHOENIX_DEBUG_UI_LOG=1 $<TARGET_FILE:test_xy_plot_instrumentation>)
endif()
# Analysis window manager tests (Phoenix-only)
if(BUILD_TESTING)
  add_executable(test_analysis_window_manager
    test_analysis_window_manager.cpp
  )

  target_link_libraries(test_analysis_window_manager PRIVATE
    phoenix_analysis
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
  )

  target_include_directories(test_analysis_window_manager
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME test_analysis_window_manager COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_analysis_window_manager>)
endif()
# feature registry tests (Phoenix-only)
if(BUILD_TESTING)
  add_executable(feature_registry_tests
    feature_registry_tests.cpp
  )

  target_link_libraries(feature_registry_tests PRIVATE
    phoenix_feature_registry
    Qt6::Core
    Qt6::Test
  )

  target_include_directories(feature_registry_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME feature_registry_tests COMMAND feature_registry_tests)
endif()

# feature parameter panel tests (Phoenix-only)
if(BUILD_TESTING)
  add_executable(feature_parameter_panel_tests
    feature_parameter_panel_tests.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/widgets/FeatureParameterPanel.cpp
  )

  target_link_libraries(feature_parameter_panel_tests PRIVATE
    phoenix_feature_registry
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
  )

  target_include_directories(feature_parameter_panel_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME feature_parameter_panel_tests COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:feature_parameter_panel_tests>)
endif()

# Note: license_manager_tests and echo_dialog_tests removed (transport-dependent, Phase 3+)
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
