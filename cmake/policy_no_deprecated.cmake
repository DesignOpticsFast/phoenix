# cmake/policy_no_deprecated.cmake
# Purpose: keep deprecation warnings visible without breaking builds,
# and align the baseline to the actually-installed Qt.

# If Qt6::Core is found, use its version; otherwise fall back.
if(DEFINED Qt6Core_VERSION)
  set(QT_DEPRECATED_WARNINGS_SINCE ${Qt6Core_VERSION})
else()
  # Fallback if this file is included very early
  set(QT_DEPRECATED_WARNINGS_SINCE 6.9.0)
endif()

# Do NOT treat deprecated warnings as errors on MSVC (flag is GCC/Clang-specific).
if(MSVC)
  # Optional: silence ubiquitous CRT deprecation warnings in MSVC
  add_compile_options(/wd4996)
else()
  # GCC/Clang: allow warnings, but don't fail the build on deprecated declarations
  add_compile_options(-Wno-error=deprecated-declarations)
endif()

# Keep older APIs available; tighten later if desired.
set(QT_DISABLE_DEPRECATED_BEFORE 0)

message(STATUS "Qt deprecation baseline: ${QT_DEPRECATED_WARNINGS_SINCE}")
message(STATUS "Deprecation warnings are NOT fatal on this compiler.")
