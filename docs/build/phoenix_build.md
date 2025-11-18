# Building Phoenix

## Requirements

- Qt 6.10.0 or later
- CMake 3.20 or later
- C++17-capable compiler (GCC 11+, Clang 14+, MSVC 2019+)
- Linux (dev-01) or macOS

## Quick Start

### Linux (dev-01)

```bash
git clone <repo-url>
cd phoenix
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
  -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### macOS

```bash
git clone <repo-url>
cd phoenix
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.10.0/macos \
  -DBUILD_TESTING=OFF
cmake --build build --parallel
```

## Build Targets

- `phoenix_app` - Main application executable
- `phoenix_analysis` - Analysis window library
- `analysis_sanity_tests` - Analysis window tests
- `graphs_perf_sanity` - Qt Graphs performance test

## Notes

- On dev-01: Tests can be built and run
- On macOS: `BUILD_TESTING=OFF` currently used due to libsodium/test harness work-in-progress
- Qt Graphs requires Qt 6.10+ (GraphsWidgets module)

