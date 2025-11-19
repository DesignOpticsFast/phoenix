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

  # Link Protobuf libraries (needed for both gRPC and LocalSocket)
  if(HAVE_PROTOBUF)
    if(TARGET protobuf::libprotobuf)
      target_link_libraries(transport_sanity_tests PRIVATE protobuf::libprotobuf)
    else()
      target_link_libraries(transport_sanity_tests PRIVATE ${Protobuf_LIBRARIES})
    endif()
  endif()
  
  # Link gRPC libraries if available (only needed for gRPC transport)
  if(HAVE_PROTOBUF AND HAVE_GRPC)
    if(TARGET gRPC::grpc++)
      target_link_libraries(transport_sanity_tests PRIVATE gRPC::grpc++)
    else()
      target_link_libraries(transport_sanity_tests PRIVATE ${gRPC_LIBRARIES})
    endif()
  endif()

  add_test(NAME transport_sanity_tests COMMAND transport_sanity_tests)
endif()

# license verification tests
if(BUILD_TESTING AND PHX_WITH_LIBSODIUM)
  add_executable(license_verification_tests
    license_verification_tests.cpp
  )

  target_link_libraries(license_verification_tests PRIVATE
    phoenix_license
    phoenix_canonical_json
    phoenix_libsodium
    Qt6::Core
    Qt6::Test
  )

  target_include_directories(license_verification_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME license_verification_tests COMMAND license_verification_tests)
endif()

# license manager tests
if(BUILD_TESTING AND PHX_WITH_LIBSODIUM)
  add_executable(license_manager_tests
    license_manager_tests.cpp
  )

  target_link_libraries(license_manager_tests PRIVATE
    phoenix_license_manager
    phoenix_license
    phoenix_canonical_json
    phoenix_libsodium
    Qt6::Core
    Qt6::Test
  )

  target_include_directories(license_manager_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME license_manager_tests COMMAND license_manager_tests)
endif()

# echo dialog tests
if(BUILD_TESTING AND PHX_WITH_LIBSODIUM)
  add_executable(echo_dialog_tests
    echo_dialog_tests.cpp
  )

  target_link_libraries(echo_dialog_tests PRIVATE
    phoenix_transport
    phoenix_license_manager
    phoenix_license
    phoenix_canonical_json
    phoenix_libsodium
    Qt6::Core
    Qt6::Test
  )

  target_include_directories(echo_dialog_tests
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  # Link Protobuf libraries (needed for transport)
  if(HAVE_PROTOBUF)
    if(TARGET protobuf::libprotobuf)
      target_link_libraries(echo_dialog_tests PRIVATE protobuf::libprotobuf)
    else()
      target_link_libraries(echo_dialog_tests PRIVATE ${Protobuf_LIBRARIES})
    endif()
  endif()
  
  # Link gRPC libraries if available
  if(HAVE_PROTOBUF AND HAVE_GRPC)
    if(TARGET gRPC::grpc++)
      target_link_libraries(echo_dialog_tests PRIVATE gRPC::grpc++)
    else()
      target_link_libraries(echo_dialog_tests PRIVATE ${gRPC_LIBRARIES})
    endif()
  endif()

  add_test(NAME echo_dialog_tests COMMAND echo_dialog_tests)
endif()

# feature registry tests
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

# feature parameter panel tests
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

# XY Sine transport tests
if(BUILD_TESTING)
  add_executable(test_xysine_transport
    test_xysine_transport.cpp
  )

  target_link_libraries(test_xysine_transport PRIVATE
    phoenix_transport
    phoenix_feature_registry
    Qt6::Core
    Qt6::Test
  )

  target_include_directories(test_xysine_transport
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  # Link Protobuf libraries (needed for transport)
  if(HAVE_PROTOBUF)
    if(TARGET protobuf::libprotobuf)
      target_link_libraries(test_xysine_transport PRIVATE protobuf::libprotobuf)
    else()
      target_link_libraries(test_xysine_transport PRIVATE ${Protobuf_LIBRARIES})
    endif()
  endif()

  add_test(NAME test_xysine_transport COMMAND test_xysine_transport)
endif()

# async architecture tests
if(BUILD_TESTING)
  add_executable(test_async_architecture
    test_async_architecture.cpp
  )

  target_link_libraries(test_async_architecture PRIVATE
    phoenix_analysis
    Qt6::Core
    Qt6::Test
  )

  target_include_directories(test_async_architecture
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME test_async_architecture COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_async_architecture>)
endif()

# analysis progress tests
if(BUILD_TESTING)
  add_executable(test_analysis_progress
    test_analysis_progress.cpp
  )

  target_link_libraries(test_analysis_progress PRIVATE
    phoenix_analysis
    Qt6::Core
    Qt6::Test
  )

  target_include_directories(test_analysis_progress
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src
  )

  add_test(NAME test_analysis_progress COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:test_analysis_progress>)
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
