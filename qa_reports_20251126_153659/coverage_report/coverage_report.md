# Coverage Report

## Summary
- Build directory: build_coverage
- Test binary: build_coverage/palantir_integration_tests

## Coverage Statistics
- Line coverage: 0%
- Function coverage: 0%
- Branch coverage: 0%

## Palantir Coverage
- Line coverage: 0%
- Target: ≥80%

## Status: 🔴 FAIL

Coverage collection succeeded, but Palantir coverage (0%) is below target (80%).

---
Coverage info file (raw): [coverage.info](coverage.info)
Coverage info file (filtered): [coverage_filtered.info](coverage_filtered.info)
HTML report: [coverage_html/index.html](coverage_html/index.html)
Full log: [coverage.log](coverage.log)

## Detailed Summary (from lcov)
```
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/map.h":708: function _ZN6google8protobuf8internal10KeyMapBaseINSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEECI2NS1_14UntypedMapBaseEEPNS0_5ArenaENS1_14UntypedMapBase8TypeInfoE end line 678 less than start line 708.  Cannot derive function end line.  See lcovrc man entry for 'derive_function_end_line'.
	(use "lcov --ignore-errors inconsistent,inconsistent ..." to suppress this warning)
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/map.h":708: failed to set end line for function _ZN6google8protobuf8internal10KeyMapBaseINSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEECI2NS1_14UntypedMapBaseEEPNS0_5ArenaENS1_14UntypedMapBase8TypeInfoE.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/capabilities.pb.h":413: function _ZN8palantir12Capabilities5Impl_D1Ev end line 326 less than start line 413.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/capabilities.pb.h":413: failed to set end line for function _ZN8palantir12Capabilities5Impl_D1Ev.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/xysine.pb.cc":852: function '_ZNK8palantir14XYSineResponse11GetMetadataEv' is not hit but line 864 is: function marked 'hit'.
	To skip consistency checks, see the 'check_data_consistency' section in man lcovrc(5).
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/message.h":389:  function _ZN6google8protobuf7MessageCI2NS0_11MessageLiteEEPNS0_5ArenaE found on line but no corresponding 'line' coverage data point.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/message.h":264: function '_ZN6google8protobuf7MessageD2Ev' is hit but no contained lines are hit: function marked 'not hit'.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/envelope.pb.h":381: function _ZN8palantir15MessageEnvelope5Impl_D1Ev end line 296 less than start line 381.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/envelope.pb.h":381: failed to set end line for function _ZN8palantir15MessageEnvelope5Impl_D1Ev.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/envelope.pb.h":130: function '_ZN8palantir38MessageEnvelope_MetadataEntry_DoNotUseD0Ev' is not hit but line 143 is: function marked 'hit'.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/capabilities.pb.cc":867: function '_ZNK8palantir20CapabilitiesResponse11GetMetadataEv' is not hit but line 879 is: function marked 'hit'.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/xysine.pb.h":289: function _ZN8palantir14XYSineResponse5Impl_D1Ev end line 156 less than start line 289.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/xysine.pb.h":289: failed to set end line for function _ZN8palantir14XYSineResponse5Impl_D1Ev.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_util.h":280: function _ZN6google8protobuf8internal12MapSorterPtrINS0_3MapINSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESA_EEED1Ev end line 273 less than start line 280.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_util.h":280: failed to set end line for function _ZN6google8protobuf8internal12MapSorterPtrINS0_3MapINSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESA_EEED1Ev.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_util.h":296: function _ZN6google8protobuf8internal12MapSorterPtrINS0_3MapINSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESA_EEE14const_iteratorCI1NS1_11MapSorterItIPKvEEEPSG_ end line 273 less than start line 296.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_util.h":296: failed to set end line for function _ZN6google8protobuf8internal12MapSorterPtrINS0_3MapINSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESA_EEE14const_iteratorCI1NS1_11MapSorterItIPKvEEEPSG_.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/repeated_ptr_field.h":718: function '_ZN6google8protobuf8internal20RepeatedPtrFieldBase13ClearNonEmptyINS1_18GenericTypeHandlerINSt3__112basic_stringIcNS5_11char_traitsIcEENS5_9allocatorIcEEEEEEEEvv' is not hit but line 778 is: function marked 'hit'.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/envelope.pb.cc":0: function '__cxx_global_var_init' is hit but no contained lines are hit: function marked 'not hit'.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/envelope.pb.cc":673: function '_ZNK8palantir15MessageEnvelope11GetMetadataEv' is not hit but line 685 is: function marked 'hit'.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_bases.h":43:  function _ZN6google8protobuf8internal14ZeroFieldsBaseCI2NS0_11MessageLiteEEPNS0_5ArenaE found on line but no corresponding 'line' coverage data point.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_bases.h":57:  function _ZN6google8protobuf8internal14ZeroFieldsBaseUt_C1Ev found on line but no corresponding 'line' coverage data point.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/xysine.pb.h":289: function _ZN8palantir14XYSineResponse5Impl_D1Ev end line 156 less than start line 289.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/xysine.pb.h":289: failed to set end line for function _ZN8palantir14XYSineResponse5Impl_D1Ev.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_util.h":280: function _ZN6google8protobuf8internal12MapSorterPtrINS0_3MapINSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESA_EEED1Ev end line 273 less than start line 280.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_util.h":280: failed to set end line for function _ZN6google8protobuf8internal12MapSorterPtrINS0_3MapINSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESA_EEED1Ev.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_util.h":296: function _ZN6google8protobuf8internal12MapSorterPtrINS0_3MapINSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESA_EEE14const_iteratorCI1NS1_11MapSorterItIPKvEEEPSG_ end line 273 less than start line 296.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_util.h":296: failed to set end line for function _ZN6google8protobuf8internal12MapSorterPtrINS0_3MapINSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESA_EEE14const_iteratorCI1NS1_11MapSorterItIPKvEEEPSG_.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_bases.h":43:  function _ZN6google8protobuf8internal14ZeroFieldsBaseCI2NS0_11MessageLiteEEPNS0_5ArenaE found on line but no corresponding 'line' coverage data point.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/generated_message_bases.h":57:  function _ZN6google8protobuf8internal14ZeroFieldsBaseUt_C1Ev found on line but no corresponding 'line' coverage data point.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/capabilities.pb.h":413: function _ZN8palantir12Capabilities5Impl_D1Ev end line 326 less than start line 413.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/capabilities.pb.h":413: failed to set end line for function _ZN8palantir12Capabilities5Impl_D1Ev.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/map.h":708: function _ZN6google8protobuf8internal10KeyMapBaseINSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEECI2NS1_14UntypedMapBaseEEPNS0_5ArenaENS1_14UntypedMapBase8TypeInfoE end line 678 less than start line 708.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/map.h":708: failed to set end line for function _ZN6google8protobuf8internal10KeyMapBaseINSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEECI2NS1_14UntypedMapBaseEEPNS0_5ArenaENS1_14UntypedMapBase8TypeInfoE.
lcov: WARNING: (inconsistent) "/opt/homebrew/include/google/protobuf/message.h":389:  function _ZN6google8protobuf7MessageCI2NS0_11MessageLiteEEPNS0_5ArenaE found on line but no corresponding 'line' coverage data point.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/envelope.pb.h":381: function _ZN8palantir15MessageEnvelope5Impl_D1Ev end line 296 less than start line 381.  Cannot derive function end line.
lcov: WARNING: (inconsistent) "/Users/underlord/workspace/phoenix/build_coverage/generated/contracts/palantir/envelope.pb.h":381: failed to set end line for function _ZN8palantir15MessageEnvelope5Impl_D1Ev.
Reading tracefile qa_reports_20251126_153659/coverage_report/coverage.info.
Summary coverage rate:
  source files: 123
  lines.......: 56.2% (2371 of 4217 lines)
  functions...: 68.5% (824 of 1203 functions)
Message summary:
  37 warning messages:
    inconsistent: 37
  1 ignore message:
    unsupported: 1
```
