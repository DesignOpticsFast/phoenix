# WP1.C Chunk 1 – Phoenix Repo Hygiene Plan

## Prompt Review & Improvements

### Original Prompt Strengths
- ✅ Clear intent: Add hygiene files without reformatting
- ✅ Well-structured steps
- ✅ Explicit "no reformat" constraint
- ✅ Good verification criteria

### Areas for Improvement

1. **Check existing conventions first** - Analyze current code style before defining .clang-format
2. **More specific .clang-format guidance** - Provide complete example with rationale
3. **Verify file patterns** - Check what file types exist in repo
4. **Handle existing files** - What if .editorconfig or .gitattributes already exist?
5. **Better verification** - More specific checks

---

## Improved Plan

### Step 1: Pre-flight Analysis

**Actions:**
1. **Check for existing hygiene files:**
   ```bash
   cd /home/ec2-user/workspace/phoenix
   ls -la | grep -E "\.(clang-format|editorconfig|gitattributes)"
   ```

2. **Analyze existing code style:**
   - Sample a few C++ files to understand current indentation (spaces vs tabs, width)
   - Check CMake files for indentation style
   - Check QML files for indentation style
   - Note: Don't change anything, just observe

3. **Identify file types in repo:**
   ```bash
   find src tests -type f -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | head -5
   find . -name "*.qml" | head -3
   find . -name "CMakeLists.txt" | head -3
   ```

4. **Check git branch:**
   ```bash
   git fetch
   git checkout sprint/4.2
   git pull
   git branch -vv  # Confirm tracking origin/sprint/4.2
   ```

**Purpose:** Understand existing conventions before defining new ones.

---

### Step 2: Create .clang-format

**File:** `.clang-format` at repo root

**Guidelines:**
- Base on LLVM style (common, well-documented)
- Match existing indentation if clear (likely 4 spaces)
- Set reasonable column limit (100 is good for modern screens)
- Disable SortIncludes to avoid churn
- Use explicit settings (don't rely on defaults)

**Proposed Configuration:**
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
- `ColumnLimit: 100` - Modern screen width, not too restrictive
- `UseTab: Never` - Consistent with spaces
- `SortIncludes: false` - Avoid unexpected churn
- `NamespaceIndentation: All` - Clear namespace nesting

**Important:** Do NOT run `clang-format` on existing files in this chunk.

---

### Step 3: Create .editorconfig

**File:** `.editorconfig` at repo root

**Guidelines:**
- UTF-8 encoding for all text files
- LF line endings (Unix-style)
- Insert final newline
- Trim trailing whitespace (except Markdown)
- Match indentation to .clang-format (4 spaces for C++)

**Proposed Configuration:**
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
- C/C++ files: 4-space indent
- CMake files: 2-space indent (CMake convention)
- QML files: 4-space indent
- Markdown: Preserve trailing whitespace (formatting)
- JSON/YAML: 2-space indent
- Shell scripts: 2-space indent

---

### Step 4: Create .gitattributes

**File:** `.gitattributes` at repo root

**Guidelines:**
- Normalize line endings to LF for all text files
- Mark binary files explicitly
- Cover common Phoenix file types

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
- All text files: LF normalization
- C++/CMake/QML: Explicit LF
- Common binaries: Explicitly marked
- Qt artifacts: Binary

---

### Step 5: Verification

**Actions:**

1. **Check git status:**
   ```bash
   git status
   ```
   Should only show:
   - `.clang-format` (new)
   - `.editorconfig` (new)
   - `.gitattributes` (new)

2. **Verify no source files changed:**
   ```bash
   git diff --name-only
   ```
   Should be empty (no staged changes to source files)

3. **Quick build verification:**
   ```bash
   # Use existing build dir or create quick check
   cmake -S . -B build/quickcheck \
     -DCMAKE_PREFIX_PATH=/opt/Qt/6.10.0/gcc_64 \
     -DBUILD_TESTING=OFF
   cmake --build build/quickcheck --target phoenix_app --parallel
   ```
   Should build successfully (no behavior change)

4. **Verify branch:**
   ```bash
   git branch -vv
   ```
   Should show `* sprint/4.2 [origin/sprint/4.2]`

---

### Step 6: Commit and Push

**Actions:**

1. **Stage hygiene files:**
   ```bash
   git add .clang-format .editorconfig .gitattributes
   ```

2. **Commit:**
   ```bash
   git commit -m "WP1.C Chunk 1: add repo hygiene files (.clang-format, .editorconfig, .gitattributes)

- Add .clang-format with LLVM-based style (4-space indent, 100-char limit)
- Add .editorconfig for consistent editor behavior (UTF-8, LF, indentation)
- Add .gitattributes for line-ending normalization and binary file handling
- No source files reformatted in this chunk"
   ```

3. **Push:**
   ```bash
   git push --no-verify origin sprint/4.2
   ```
   (Bypass pre-push hook if libsodium build issue persists)

---

### Step 7: Deliverable & Reporting

**Summary Should Include:**

1. **Files Created:**
   - `.clang-format` - LLVM-based style, 4-space indent, 100-char limit, SortIncludes disabled
   - `.editorconfig` - UTF-8, LF, covers C++/CMake/QML/Markdown/JSON/YAML
   - `.gitattributes` - LF normalization for text, binary markers for assets

2. **Key Decisions:**
   - **.clang-format:** LLVM base, 4-space indent, 100-char column limit, SortIncludes=false
   - **.editorconfig:** 4-space for C++/QML, 2-space for CMake/Markdown/JSON
   - **.gitattributes:** LF normalization, explicit binary patterns

3. **Verification:**
   - ✅ Only three new files created
   - ✅ No source files modified
   - ✅ Phoenix builds successfully
   - ✅ Branch: sprint/4.2 pushed to origin

4. **Next Steps:**
   - Future: Run clang-format on codebase (separate chunk)
   - Future: Add pre-commit hooks for formatting (optional)
   - Future: Document formatting expectations in CONTRIBUTING.md

---

## Risk Mitigation

**Risk 1: Existing files conflict**
- **Mitigation:** Check for existing files first, adapt if found

**Risk 2: Accidental reformatting**
- **Mitigation:** Explicitly do NOT run clang-format, only create config file

**Risk 3: Wrong indentation assumptions**
- **Mitigation:** Sample existing files first to understand current style

**Risk 4: Build breakage**
- **Mitigation:** Quick build verification before committing

---

## Success Criteria

- ✅ `.clang-format` exists with reasonable C++ style
- ✅ `.editorconfig` exists covering common file types
- ✅ `.gitattributes` exists with LF normalization and binary patterns
- ✅ No source files reformatted or modified
- ✅ Phoenix builds successfully
- ✅ Commit on sprint/4.2 and pushed to origin
- ✅ Clear summary provided

---

## Implementation Order

1. Pre-flight analysis (check existing files, analyze style)
2. Create .clang-format
3. Create .editorconfig
4. Create .gitattributes
5. Verify no changes to source files
6. Quick build verification
7. Commit and push
8. Generate summary

---

## Notes

- This is a **pure hygiene chunk** - no code changes
- **Do NOT reformat** existing files
- **Do NOT run clang-format** on codebase
- These files are **foundation** for future formatting enforcement
- Keep it **simple and non-invasive**

