add_test([=[BedrockMath.Add]=]  /Users/mark/Projects/phoenix/build/_bedrock_build/tests/bedrock_tests [==[--gtest_filter=BedrockMath.Add]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BedrockMath.Add]=]  PROPERTIES WORKING_DIRECTORY /Users/mark/Projects/phoenix/build/_bedrock_build/tests SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set(  bedrock_tests_TESTS BedrockMath.Add)
