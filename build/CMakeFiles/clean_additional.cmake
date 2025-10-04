# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/phoenix_app_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/phoenix_app_autogen.dir/ParseCache.txt"
  "_bedrock_build/CMakeFiles/bedrock_engine_autogen.dir/AutogenUsed.txt"
  "_bedrock_build/CMakeFiles/bedrock_engine_autogen.dir/ParseCache.txt"
  "_bedrock_build/CMakeFiles/bedrock_geom_autogen.dir/AutogenUsed.txt"
  "_bedrock_build/CMakeFiles/bedrock_geom_autogen.dir/ParseCache.txt"
  "_bedrock_build/CMakeFiles/bedrock_smoke_step_autogen.dir/AutogenUsed.txt"
  "_bedrock_build/CMakeFiles/bedrock_smoke_step_autogen.dir/ParseCache.txt"
  "_bedrock_build/bedrock_engine_autogen"
  "_bedrock_build/bedrock_geom_autogen"
  "_bedrock_build/bedrock_smoke_step_autogen"
  "_bedrock_build/core/CMakeFiles/bedrock_core_autogen.dir/AutogenUsed.txt"
  "_bedrock_build/core/CMakeFiles/bedrock_core_autogen.dir/ParseCache.txt"
  "_bedrock_build/core/bedrock_core_autogen"
  "_bedrock_build/tests/CMakeFiles/bedrock_tests_autogen.dir/AutogenUsed.txt"
  "_bedrock_build/tests/CMakeFiles/bedrock_tests_autogen.dir/ParseCache.txt"
  "_bedrock_build/tests/bedrock_tests_autogen"
  "_deps/googletest-build/googlemock/CMakeFiles/gmock_autogen.dir/AutogenUsed.txt"
  "_deps/googletest-build/googlemock/CMakeFiles/gmock_autogen.dir/ParseCache.txt"
  "_deps/googletest-build/googlemock/CMakeFiles/gmock_main_autogen.dir/AutogenUsed.txt"
  "_deps/googletest-build/googlemock/CMakeFiles/gmock_main_autogen.dir/ParseCache.txt"
  "_deps/googletest-build/googlemock/gmock_autogen"
  "_deps/googletest-build/googlemock/gmock_main_autogen"
  "_deps/googletest-build/googletest/CMakeFiles/gtest_autogen.dir/AutogenUsed.txt"
  "_deps/googletest-build/googletest/CMakeFiles/gtest_autogen.dir/ParseCache.txt"
  "_deps/googletest-build/googletest/CMakeFiles/gtest_main_autogen.dir/AutogenUsed.txt"
  "_deps/googletest-build/googletest/CMakeFiles/gtest_main_autogen.dir/ParseCache.txt"
  "_deps/googletest-build/googletest/gtest_autogen"
  "_deps/googletest-build/googletest/gtest_main_autogen"
  "phoenix_app_autogen"
  )
endif()
