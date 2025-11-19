# 🧠 UnderLord Prompt — WP1.C Chunk 1 – Phoenix Repo Hygiene

## Intent

In the Phoenix repo, on the `sprint/4.2` branch, add basic repo hygiene files:
- `.clang-format` — defines C++ style (do NOT reformat existing code)
- `.editorconfig` — update existing file if needed (normalized whitespace/encoding)
- `.gitattributes` — basic line-ending and text/binary rules

This is a pure hygiene/meta chunk: no behavior changes, no code rewrites.

---

## Context

- **Repo:** DesignOpticsFast/phoenix
- **Sprint branch:** `sprint/4.2` (all Sprint 4.2 work happens here)
- **Existing:** `.editorconfig` already exists (check and update if needed)
- **Goal:** Make it easy to enforce consistent formatting later, keep IDEs aligned, avoid line-ending noise

**Critical:** This chunk must NOT reformat existing source files.

---

## Ask

### Step 1: Ensure You're on Sprint Branch

```bash
cd /home/ec2-user/workspace/phoenix
git fetch
git checkout sprint/4.2
git pull
```

**Confirm:**
```bash
git branch -vv
```
Should show: `* sprint/4.2 [origin/sprint/4.2]`

---

### Step 2: Check Existing Files

**Check for existing hygiene files:**
```bash
ls -la | grep -E "\.(clang-format|editorconfig|gitattributes)"
```

**Note:** `.editorconfig` already exists - read it and decide if it needs updates or is sufficient.

**Read existing .editorconfig:**
```bash
cat .editorconfig
```

**Decision:**
- If `.editorconfig` is comprehensive and matches our needs → keep it, just verify
- If `.editorconfig` is minimal or missing patterns → update it
- If `.editorconfig` conflicts with our goals → update it

---

### Step 3: Analyze Existing Code Style (Optional but Recommended)

**Sample a few files to understand current style:**
```bash
head -30 src/plot/XYPlotViewGraphs.cpp
head -30 src/ui/main/MainWindow.cpp
head -20 CMakeLists.txt
```

**Look for:**
- Indentation: spaces vs tabs, width (likely 4 spaces)
- Line endings: LF vs CRLF (likely LF)
- Column width: typical line lengths

**Purpose:** Align `.clang-format` with existing conventions, not impose new ones.

---

### Step 4: Create/Update .clang-format

**File:** `.clang-format` at repo root

**If file exists:** Review and update if needed  
**If file doesn't exist:** Create new

**Proposed Configuration (LLVM-based, matches common C++ style):**
```yaml
---
Language: Cpp
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
NamespaceIndentation: All
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: Never
AllowShortLoopsOnASingleLine: false
BreakBeforeBraces: Attach
BreakConstructorInitializers: BeforeColon
BreakInheritanceList: BeforeColon
ContinuationIndentWidth: 4
FixNamespaceComments: true
IncludeBlocks: Preserve
SortIncludes: false
SpaceAfterCStyleCast: false
SpaceAfterTemplateKeyword: true
SpaceBeforeAssignmentOperators: true
SpaceBeforeParens: ControlStatements
SpacesInAngles: Never
SpacesInParentheses: false
```

**Key Decisions:**
- `IndentWidth: 4` - Common C++ standard
- `ColumnLimit: 100` - Modern screen width
- `UseTab: Never` - Consistent spaces
- `SortIncludes: false` - Avoid unexpected churn
- `NamespaceIndentation: All` - Clear nesting

**CRITICAL:** Do NOT run `clang-format` on existing source files. This file is for future use only.

---

### Step 5: Update/Create .editorconfig

**File:** `.editorconfig` at repo root (already exists)

**If existing file is good:** Verify it covers our needs  
**If existing file needs updates:** Update it

**Proposed Configuration (comprehensive):**
```ini
root = true

[*]
charset = utf-8
end_of_line = lf
insert_final_newline = true
trim_trailing_whitespace = true

[*.{cpp,hpp,h,cxx,cc,c}]
indent_style = space
indent_size = 4

[*.{cmake,txt}]
indent_style = space
indent_size = 2

[*.qml]
indent_style = space
indent_size = 4

[*.{json,jsonc}]
indent_style = space
indent_size = 2

[*.md]
trim_trailing_whitespace = false
indent_style = space
indent_size = 2

[*.{yml,yaml}]
indent_style = space
indent_size = 2

[*.{sh,bash}]
indent_style = space
indent_size = 2
```

**Coverage:**
- C/C++: 4-space indent
- CMake: 2-space indent (CMake convention)
- QML: 4-space indent
- Markdown: Preserve trailing whitespace
- JSON/YAML: 2-space indent
- Shell: 2-space indent

**Action:** Read existing file, update if needed, or keep if sufficient.

---

### Step 6: Create .gitattributes

**File:** `.gitattributes` at repo root

**If file exists:** Review and update if needed  
**If file doesn't exist:** Create new

**Proposed Configuration:**
```gitattributes
# Auto-detect text files and normalize line endings
* text=auto eol=lf

# Explicitly mark text files with LF normalization
*.cpp text eol=lf
*.hpp text eol=lf
*.h   text eol=lf
*.c   text eol=lf
*.cxx text eol=lf
*.cc  text eol=lf
*.cmake text eol=lf
CMakeLists.txt text eol=lf
*.qml text eol=lf
*.qrc text eol=lf
*.md  text eol=lf
*.txt text eol=lf
*.json text eol=lf
*.yml text eol=lf
*.yaml text eol=lf
*.sh text eol=lf
*.bash text eol=lf
*.pro text eol=lf
*.pri text eol=lf
*.prf text eol=lf

# Treat common binary files as binary
*.png  binary
*.jpg  binary
*.jpeg binary
*.gif  binary
*.ico  binary
*.svg  binary
*.ttf  binary
*.otf  binary
*.woff binary
*.woff2 binary
*.pdf  binary
*.zip  binary
*.tar  binary
*.gz   binary
*.bz2  binary
*.xz   binary
*.so   binary
*.dylib binary
*.dll  binary
*.exe  binary
*.a    binary
*.lib  binary
*.o    binary
*.obj  binary

# Qt-specific
*.qm binary
*.qmlc binary
```

**Coverage:**
- Text files: LF normalization
- C++/CMake/QML: Explicit LF
- Binaries: Explicitly marked
- Qt artifacts: Binary

---

### Step 7: Verify No Behavior Change

**Check git status:**
```bash
git status
```

**Expected:** Only these files should appear:
- `.clang-format` (new or modified)
- `.editorconfig` (possibly modified if updated)
- `.gitattributes` (new or modified)

**Verify no source files changed:**
```bash
git diff --name-only
```
Should show only hygiene files, no `.cpp`, `.hpp`, `.qml`, etc.

**Quick build verification (optional but recommended):**
```bash
cmake -S . -B build/quickcheck \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
  -DBUILD_TESTING=OFF
cmake --build build/quickcheck --target phoenix_app --parallel
```
Should build successfully (no behavior change).

---

### Step 8: Commit and Push

**Stage hygiene files:**
```bash
git add .clang-format .editorconfig .gitattributes
```

**Commit:**
```bash
git commit -m "WP1.C Chunk 1: add repo hygiene files (.clang-format, .editorconfig, .gitattributes)

- Add .clang-format with LLVM-based style (4-space indent, 100-char limit)
- Update .editorconfig for consistent editor behavior (UTF-8, LF, indentation)
- Add .gitattributes for line-ending normalization and binary file handling
- No source files reformatted in this chunk"
```

**Push:**
```bash
git push --no-verify origin sprint/4.2
```
(Bypass pre-push hook if libsodium build issue persists)

---

### Step 9: Report Back

**Summary should include:**

1. **Files Created/Modified:**
   - `.clang-format` - LLVM-based style, 4-space indent, 100-char limit, SortIncludes disabled
   - `.editorconfig` - Updated/verified, covers C++/CMake/QML/Markdown/JSON/YAML
   - `.gitattributes` - LF normalization for text, binary markers for assets

2. **Key Decisions:**
   - **.clang-format:** LLVM base, 4-space indent, 100-char column limit, SortIncludes=false
   - **.editorconfig:** 4-space for C++/QML, 2-space for CMake/Markdown/JSON (updated existing or kept as-is)
   - **.gitattributes:** LF normalization, explicit binary patterns

3. **Verification:**
   - ✅ Only hygiene files changed (no source files modified)
   - ✅ Phoenix builds successfully
   - ✅ Branch: sprint/4.2 pushed to origin

4. **Next Steps:**
   - Future: Run clang-format on codebase (separate chunk)
   - Future: Add pre-commit hooks for formatting (optional)
   - Future: Document formatting expectations in CONTRIBUTING.md

---

## Definition of Done

- ✅ `.clang-format` exists with reasonable C++ style
- ✅ `.editorconfig` exists/updated covering common file types
- ✅ `.gitattributes` exists with LF normalization and binary patterns
- ✅ No source files reformatted or modified
- ✅ Phoenix builds successfully
- ✅ Commit on sprint/4.2 and pushed to origin/sprint/4.2
- ✅ Clear summary provided

---

## Important Constraints

1. **Do NOT reformat existing code** - Only create/update config files
2. **Do NOT run clang-format** - Config file is for future use
3. **Check existing .editorconfig** - Update if needed, keep if sufficient
4. **Verify no source changes** - Only hygiene files should be modified

---

## Troubleshooting

**If .editorconfig already exists:**
- Read it first
- Update if it's missing patterns or conflicts
- Keep if it's comprehensive and matches our needs

**If .gitattributes already exists:**
- Read it first
- Merge/update if needed
- Keep if comprehensive

**If build fails:**
- Check that no source files were accidentally modified
- Verify hygiene files are valid (syntax check)
- Revert and investigate

---

## Summary

Add/update repo hygiene files (`.clang-format`, `.editorconfig`, `.gitattributes`) without reformatting any existing code. These files establish the foundation for future formatting enforcement while keeping current code unchanged.

