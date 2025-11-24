# Phoenix Sprint 4.4 Chunk 4: Proto Source Discovery Report

**Date**: 2025-11-24  
**Chunk**: Sprint 4.4 Chunk 4 - Proto Source Discovery & Plan  
**Scope**: Discovery only - no modifications

---

## Executive Summary

**Finding**: The Palantir repository (`DesignOpticsFast/palantir`) **does NOT contain any `.proto` files** currently, and **has NEVER contained `.proto` files** in its git history.

**Current State**:
- Palantir repo contains only documentation (markdown) and JSON schema examples
- Repository README mentions "protobuf definitions" but none exist
- Phoenix contracts submodule points to Palantir v1.0.0 (initial commit)
- No proto files in any branch or commit history

**Recommendation**: Create the first proto file (`Capabilities.proto`) in the Palantir repository, then sync it to Phoenix/Bedrock via submodule update.

---

## 1. Phoenix Contracts Submodule Configuration

### Submodule Details
- **Path**: `contracts`
- **URL**: `git@github.com:DesignOpticsFast/palantir.git`
- **Branch**: Not specified (defaults to tracking the commit)
- **Current Commit**: `ad0e9882cd3d9cbbf80fc3b4ac23cd1df7547f53`
- **`.contract-version`**: `ad0e9882cd3d9cbbf80fc3b4ac23cd1df7547f53` (matches current commit)

### Submodule Contents
- **Status**: Initialized and checked out
- **Contents**: Documentation and JSON schemas only
- **Proto Files**: ❌ **NONE FOUND**
- **Structure**:
  ```
  contracts/
  └── docs/
      └── data_contracts/
          ├── *.md (markdown documentation)
          ├── examples/*.json (JSON schema examples)
          └── validation/ (Python validation scripts)
  ```

---

## 2. Palantir Repository Direct Inspection

### Repository Access
- **Source**: Used Phoenix `contracts/` submodule as working copy
- **Remote**: `git@github.com:DesignOpticsFast/palantir.git`
- **Branches**: `main` (default), `remotes/origin/main`

### Current State (main branch)
- **Commit**: `7febe5fa424e7516eefd2c3ce917c9d238973a00` (latest)
- **Contents**: 
  - `README.md` (root level)
  - `docs/data_contracts/` (markdown docs and JSON examples)
- **Proto Files**: ❌ **NONE FOUND**

### Phoenix Submodule Commit (ad0e988)
- **Commit**: `ad0e9882cd3d9cbbf80fc3b4ac23cd1df7547f53`
- **Tag**: `v1.0.0` (points to this commit)
- **Message**: "Initial commit: Palantir v1.0.0 data contracts"
- **Contents**: Same structure as main (docs only, no proto files)
- **Proto Files**: ❌ **NONE FOUND**

### File Structure Analysis
**All files found**:
- Markdown documentation files (`.md`)
- JSON schema examples (`.json`)
- Python validation scripts (`.py`)
- **No `.proto` files at any level**

---

## 3. Palantir Repository History Analysis

### Git History Search
**Commands executed**:
```bash
git log --name-only --diff-filter=A -- '*.proto'
git log --name-only -- '*.proto'
git log --all --full-history --source -- '*.proto'
git log --all --full-history -- '*proto*'
```

**Results**: 
- ❌ **No `.proto` files found in any commit**
- ❌ **No references to proto files in commit messages**
- ❌ **No proto-related file additions or removals**

### Repository History
- **Total Commits**: 4
- **Commit History**:
  1. `ad0e988` - Initial commit: Palantir v1.0.0 data contracts
  2. `62667cf` - Update README.md
  3. `50173f3` - Update README.md
  4. `7febe5f` - docs: add root-level README with repository purpose and cross-repo usage

**Conclusion**: The Palantir repository has **never contained `.proto` files** in its entire history.

---

## 4. Contract Version Validation

### Phoenix `.contract-version` Analysis
- **Value**: `ad0e9882cd3d9cbbf80fc3b4ac23cd1df7547f53`
- **Matches**: Current contracts submodule HEAD ✅
- **Tag**: Points to `v1.0.0` tag ✅
- **Commit Date**: 2025-10-07 (Initial commit)

### Commit Verification
- **Commit exists**: ✅ Yes, in Palantir repository
- **Commit has proto files**: ❌ No
- **Commit contents**: Only markdown docs and JSON schemas

### Branch Comparison
- **Phoenix uses**: `ad0e988` (v1.0.0 tag, initial commit)
- **Palantir main**: `7febe5f` (latest, has README.md)
- **Difference**: Main branch has additional README updates, but still no proto files

---

## 5. Repository Purpose Analysis

### README Content (from main branch)
The Palantir README states:
> **Palantir contracts: protobuf definitions and transport interface for Phoenix ↔ Bedrock analysis system.**

**Key Points**:
- README explicitly mentions "protobuf definitions" as the purpose
- Repository is intended for contract definitions between Phoenix and Bedrock
- Current implementation uses Vega-Lite JSON + Arrow Flight (not protobuf)
- No proto files exist despite the stated purpose

### Current Contract Format
According to `docs/data_contracts/README.md`:
- **Spec**: Vega-Lite JSON (renderer-agnostic) + optional `phoenix:{}` extensions
- **Data**: Arrow Flight streams (RecordBatches) or shared memory
- **Control**: gRPC/DoAction (mentioned but not defined)

**Gap**: gRPC/DoAction control protocol is mentioned but has no protobuf definitions.

---

## 6. Synthesis and Recommendations

### Current State Summary

1. **Palantir Repository**:
   - ✅ Exists and is accessible
   - ✅ Has proper structure for contracts
   - ❌ Contains NO `.proto` files
   - ❌ Has NEVER contained `.proto` files
   - ⚠️ README mentions protobuf but none exist

2. **Phoenix Contracts Submodule**:
   - ✅ Properly configured
   - ✅ Points to Palantir v1.0.0
   - ❌ Contains no proto files
   - ✅ `.contract-version` matches submodule commit

3. **Bedrock Palantir Submodule**:
   - ✅ Initialized (from Chunk 3)
   - ❌ Contains no proto files (same repo, different path)

### Recommended Proto File Layout

#### Option A: Palantir as Source of Truth (Selected)

**Directory Structure in Palantir**:
```
palantir/
├── proto/                    # New directory for protobuf definitions
│   ├── palantir/            # Package namespace
│   │   ├── capabilities.proto
│   │   ├── compute.proto    # Future: XY Sine, etc.
│   │   └── transport.proto  # Future: RPC messages
│   └── README.md            # Proto documentation
├── docs/
│   └── data_contracts/      # Existing: Vega-Lite/Arrow contracts
└── README.md
```

**Phoenix Consumption**:
- Phoenix `contracts/` submodule will contain `proto/` directory after update
- Path: `contracts/proto/palantir/capabilities.proto`
- CMake proto generation: `contracts/proto/palantir/*.proto`

**Bedrock Consumption**:
- Bedrock `docs/palantir/` submodule will contain `proto/` directory after update
- Path: `docs/palantir/proto/palantir/capabilities.proto`
- CMake proto generation: `docs/palantir/proto/palantir/*.proto`

**Alternative**: Use same submodule path in both repos (e.g., both use `contracts/`)

### Proposed Capabilities.proto Structure

**High-Level Specification** (not the actual proto file):

```protobuf
// Minimal Capabilities message for WP1
// Purpose: Confirm Phoenix can communicate with Bedrock
// Location: proto/palantir/capabilities.proto

syntax = "proto3";
package palantir;

// Capabilities message - minimal for WP1
message Capabilities {
  string server_version = 1;
  repeated string supported_features = 2;
  // Future: Add more capability fields as needed
}

// CapabilitiesRequest - request server capabilities
message CapabilitiesRequest {
  // Empty for now - can add filters later
}

// CapabilitiesResponse - server capabilities
message CapabilitiesResponse {
  Capabilities capabilities = 1;
}
```

**Note**: This is a specification only. The actual `.proto` file should be created in Palantir repo.

### Contract Versioning Strategy

**Recommendation**: Continue using `.contract-version` to track Palantir commit SHA
- When proto files are added to Palantir, update Phoenix `.contract-version` to new commit
- Both Phoenix and Bedrock should track the same Palantir commit for proto parity
- Tag Palantir releases (e.g., `v1.1.0` for first proto addition)

---

## 7. Open Decisions for Capo

### Critical Decisions Needed

1. **Who creates the first proto file?**
   - Should UnderLord create `Capabilities.proto` in Palantir repo?
   - Or should Capo create it manually?
   - **Recommendation**: UnderLord can create minimal `Capabilities.proto` per spec above

2. **Proto file location in Palantir**:
   - Proposed: `proto/palantir/capabilities.proto`
   - Alternative: `proto/capabilities.proto` (flatter structure)
   - **Recommendation**: Use `proto/palantir/` namespace directory

3. **Capabilities.proto fields**:
   - Minimal for WP1: `server_version`, `supported_features`
   - Or more comprehensive?
   - **Recommendation**: Start minimal, expand in later sprints

4. **Submodule path consistency**:
   - Phoenix uses `contracts/`
   - Bedrock uses `docs/palantir/`
   - Should both use same path?
   - **Recommendation**: Keep as-is for now (different paths are fine)

5. **When to update Phoenix submodule**:
   - After proto file is added to Palantir?
   - Before or after WP1 code changes?
   - **Recommendation**: Update submodule first, then proceed with WP1

6. **Proto generation in CMake**:
   - Should Phoenix generate proto code during build?
   - Or pre-generate and commit?
   - **Recommendation**: Generate during build (WP1 scope)

### Non-Critical Decisions

1. **Additional proto files**:
   - XY Sine proto (WP2+)
   - Transport/RPC messages (future)
   - **Recommendation**: Defer to later sprints

2. **Proto versioning**:
   - Use proto3 (recommended)
   - Backward compatibility strategy
   - **Recommendation**: Use proto3, plan for evolution

---

## 8. Next Steps

### Immediate Actions (Before WP1)

1. **Create Capabilities.proto in Palantir**:
   - Location: `proto/palantir/capabilities.proto`
   - Content: Minimal spec as outlined above
   - Commit to Palantir main branch

2. **Update Phoenix contracts submodule**:
   - `cd /Users/underlord/workspace/phoenix`
   - `git submodule update --remote contracts`
   - `git add contracts .contract-version`
   - Update `.contract-version` to new commit SHA
   - Commit submodule update

3. **Update Bedrock palantir submodule** (if needed):
   - `cd /Users/underlord/workspace/bedrock`
   - `git submodule update --remote docs/palantir`
   - Commit submodule update

4. **Verify proto file availability**:
   - Confirm `contracts/proto/palantir/capabilities.proto` exists in Phoenix
   - Test protoc code generation
   - Proceed to WP1

### WP1 Readiness After Proto Creation

- [x] OpenMP installed ✅
- [x] Protobuf toolchain installed ✅
- [x] Submodules initialized ✅
- [ ] **Capabilities.proto created in Palantir** ❌ **BLOCKER**
- [ ] **Phoenix contracts submodule updated** ❌ **BLOCKER**
- [ ] **Proto file verified in Phoenix** ❌ **BLOCKER**

---

## 9. Summary

**Discovery Results**:
- ✅ Palantir repository exists and is properly configured
- ✅ Phoenix contracts submodule is correctly set up
- ❌ **No `.proto` files exist in Palantir (current or history)**
- ❌ **This is a blocker for WP1**

**Recommendation**:
1. Create `proto/palantir/capabilities.proto` in Palantir repository
2. Update Phoenix contracts submodule to include proto file
3. Verify proto file is accessible for WP1 code generation
4. Proceed with WP1 transport reintroduction

**Status**: Ready for proto file creation once Capo approves the structure and location.

---

**End of Proto Discovery Report**

