# Contributing to Phoenix

Thanks for your interest in contributing! Phoenix is a Qt-based GUI for optical design built on Bedrock. We welcome issues, discussions, and pull requests.

## How to Contribute
1. **Check Issues**: Look through open issues before starting new work.
2. **Discuss First**: For major features, open a proposal or RFC before coding.
3. **Fork & Branch**: Create a feature branch in your fork (`feature/your-topic`).
4. **Code Style**:
   - Follow the existing C++ and QML style; keep functions small and testable.
   - Add comments for clarity.
5. **Tests**: Add unit tests where possible (see `/tests`).
6. **Pull Requests**: Link to the issue being fixed, and explain your changes clearly.

## Reporting Issues
- Use the **Issues** tab for bug reports or feature requests.
- Provide steps to reproduce bugs and expected behavior.

## Code of Conduct
All contributors are expected to follow our [Code of Conduct](CODE_OF_CONDUCT.md).

## Sprint 4 Scope & Temporary Policies

- **Palantir integration**: deferred. Build flag `PHX_WITH_PALANTIR` defaults to **OFF**.
  - When OFF, adding new blocking calls (e.g., `waitFor*`, `sleep`) under `src/palantir/` is prohibited; CI will fail if detected.
  - Existing blocking calls remain for now; they'll be guarded/reworked in the next sprint chunk.

- **Plotting**: deferred to Sprint 5. Only prepare constants/APIs; no new plotting code this sprint.

- **Icons**: Font Awesome **glyphs** are the primary icon system for UI chrome. SVGs remain for branding/diagrams; SVGs must use `fill="currentColor"` to tint with theme.

- **Constants**: Use `phx::*` constants from `src/app/PhxConstants.h` instead of magic numbers. Plotting is stubbed until Sprint 5.

- **Assets**: Font Awesome glyphs are used for UI chrome icons (via IconProvider). SVGs are reserved for branding/illustration only (e.g., `phoenix-icon.svg`). Monochrome SVGs must use `fill="currentColor"` to tint with theme.

## File I/O Pattern

Phoenix uses a centralized `FileIO` utility namespace for all file operations to ensure consistent error handling and path safety.

### Standard Helpers

Use `FileIO` namespace functions from `src/app/io/FileIO.h`:

- **`FileIO::canonicalize(path)`**: Normalize file paths (uses `QFileInfo::canonicalFilePath()` with fallback to `absolutePath()`)
- **`FileIO::ensureDir(dirPath, &errOut)`**: Create directory and parent directories if needed; returns `false` on failure
- **`FileIO::readTextFile(path, &out, &errOut)`**: Safely read text files with error reporting
- **`FileIO::writeTextFileAtomic(path, text, &errOut)`**: Write text files atomically using `QSaveFile` (prevents corruption on failure)

### Guidelines

1. **Always validate paths**: Check for non-empty paths before file operations
2. **Use QSaveFile for user-visible writes**: All user-initiated saves/exports should use `FileIO::writeTextFileAtomic()` for atomic writes
3. **Check return values**: All `FileIO` functions return `bool`; check and handle failures appropriately
4. **Error reporting**: Use the optional `errOut` parameter to get detailed error messages for UI feedback
5. **Directory creation**: Always ensure parent directories exist using `FileIO::ensureDir()` before writing files
6. **Non-blocking operations**: For long file operations, use `QtConcurrent::run` + `QFutureWatcher` to avoid blocking the GUI thread

### Example

```cpp
#include "app/io/FileIO.h"

QString targetPath = QFileDialog::getSaveFileName(...);
if (targetPath.isEmpty()) {
    return; // User cancelled
}

QString canonicalPath = FileIO::canonicalize(targetPath);
QFileInfo fileInfo(canonicalPath);
QString dirError;
if (!FileIO::ensureDir(fileInfo.absolutePath(), &dirError)) {
    QMessageBox::warning(this, tr("Save"), 
        tr("Failed to create directory:\n%1\n\n%2").arg(fileInfo.absolutePath(), dirError));
    return;
}

QString error;
if (!FileIO::writeTextFileAtomic(canonicalPath, content, &error)) {
    QMessageBox::warning(this, tr("Save"), 
        tr("Failed to save file:\n%1\n\n%2").arg(canonicalPath, error));
    return;
}
```

---

By contributing, you agree your work will be licensed under the Apache 2.0 License.
