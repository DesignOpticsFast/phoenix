# UnderLord Mac Environment Map

**Last Updated**: 2025-01-27  
**Status**: Active

## Environment Overview

This document provides context about the Mac development environment. **This is context only, not permission for Mac access.**

## Account Information

- **UnderLord Account**: `underlord`
- **Workspace**: `/Users/underlord/workspace/`
- **Mark's Account**: `mark` (read-only access for Qt only)

## Qt Installation

- **Location**: `/Users/mark/Qt/6.10.0/macos`
- **Access**: Read-only for build purposes only
- **Version**: Qt 6.10.0
- **Purpose**: CMake build system references this path for Qt6_DIR

## Workspace Structure

```
/Users/underlord/workspace/
├── phoenix/          # Phoenix project
├── bedrock/           # Bedrock project
└── rosetta/          # Rosetta project
```

## Build Environment

- **CMake**: Required for builds
- **Qt6_DIR**: Points to `/Users/mark/Qt/6.10.0/macos/lib/cmake/Qt6`
- **CMAKE_PREFIX_PATH**: Includes Qt 6.10.0 path

## Important Notes

1. **Read-Only Qt Access**: UnderLord may read Qt installation for build purposes only
2. **No Mark Account Access**: UnderLord must never access `/Users/mark/` except for read-only Qt access
3. **Workspace Boundaries**: All work occurs in `/Users/underlord/workspace/`
4. **This is context only**: This document provides information, not permission for broader Mac access

## Daily Ritual Acknowledgment

During the daily ritual, UnderLord acknowledges:
- ✅ Valid account: underlord
- ✅ Workspace: /Users/underlord/workspace/
- ✅ Qt read-only: /Users/mark/Qt/6.10.0/macos
- ⚠️  This is context only, not permission for Mac access

---

**This environment map is for informational purposes only and does not grant access permissions.**

