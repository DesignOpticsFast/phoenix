# Phoenix Project Overview

**Project Name:** Phoenix  
**Type:** Qt6 Desktop Application  
**Purpose:** Professional optical design tool with AI integration

---

## Mission

Phoenix provides an intuitive, modern desktop interface for optical design work, powered by the Bedrock calculation engine. It aims to combine traditional optical design capabilities with AI-assisted workflows and real-time collaboration.

---

## Architecture

### System Design
Phoenix follows a layered architecture:
- **UI Layer** - Qt6 widgets and dialogs
- **Application Layer** - Business logic and workflow
- **Integration Layer** - Bedrock library interface
- **Data Layer** - Project files and configuration

### Key Components

#### Main Window (`src/ui/`)
- Primary application interface
- Menu system and toolbars
- Workspace management

#### Editors (`src/editors/`)
- Lens design editor
- Surface property editor
- Ray tracing visualizer

#### Bedrock Integration (`src/integration/`)
- Interface to Bedrock calculation engine
- Data conversion between UI and computation

---

## Technology Stack

### Language & Standards
- **C++ 17** - Core application code
- **Qt 6.9+** - UI framework
- **CMake 3.22+** - Build system

### Key Dependencies
- **Qt6** - Widgets, Core, Gui modules
- **Bedrock** - Optical calculations (sibling repo)
- **OpenCASCADE (OCCT)** - Via Bedrock
- **GoogleTest** - Testing framework

### Platform Support
- **Primary:** macOS (Qt 6.9.3)
- **Secondary:** Linux
- **Future:** Windows

---

## Project Structure

```
phoenix/
├── src/
│   ├── main.cpp            # Application entry point
│   ├── ui/                 # User interface components
│   ├── editors/            # Specialized editors
│   ├── integration/        # Bedrock interface
│   └── project/            # Project management
├── resources/              # Icons, UI files, assets
├── tests/                  # Unit and integration tests
├── docs/                   # Documentation
├── .underlord/             # UnderLord development system
├── .github/                # CI workflows
└── CMakeLists.txt          # Build configuration
```

---

## Development Workflow

### Branch Strategy
- **main** - Stable, production-ready code
- **sprint-NNN-slug** - Feature branches
- PR required for all changes
- Squash merge preferred

### Build Process
```bash
# Note: Bedrock must be in sibling directory ../bedrock
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j

# Run
./build/phoenix.app/Contents/MacOS/phoenix  # macOS
./build/phoenix                              # Linux
```

### CI Pipeline
1. **validate** - Syntax and format checks
2. **macOS build** - Full build with Qt 6.9.3 + Bedrock sibling
3. **CodeQL Analysis** - Security scanning

All checks must pass before merge.

---

## Code Standards

### Naming Conventions
- **Namespace:** `phoenix::`
- **Qt Classes:** `PascalCase`
- **Qt Slots/Signals:** `camelCase`
- **Business Logic:** `snake_case`
- **UI Files:** `ClassName.ui`

---

## Integration with Bedrock

### Build Dependency
Phoenix requires Bedrock as a sibling directory:
```
workspace/
├── bedrock/
└── phoenix/
```

CMakeLists.txt includes: `add_subdirectory(../bedrock bedrock)`

### Runtime Interface
```cpp
#include <bedrock/optics/raytrace.h>

// Use Bedrock functionality
bedrock::RayTrace tracer;
auto result = tracer.compute(/* ... */);
```

---

## Common Tasks

### Adding UI Feature
1. Start sprint with UI-focused goals
2. Implement widget or dialog
3. Wire up to business logic
4. Add Bedrock integration if needed
5. Test manually and with automated tests
6. Update user documentation

---

**Last Updated:** 2025-10-14  
**Maintained By:** UnderLord development system
