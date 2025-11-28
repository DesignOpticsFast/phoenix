# Coding Standards - Phoenix

**Version:** 1.0.0  
**Last Updated:** 2025-01-21  
**Language:** C++17  
**Framework:** Qt 6+  
**Build Tool:** CMake (see [VERSIONS.md](VERSIONS.md) for minimum version)  
**Development Environment:** Local Machines + GitHub Actions CI

> **Note:** For current toolchain versions (C++ standard, Qt, CMake, Protobuf, etc.), see [VERSIONS.md](VERSIONS.md).

---

## Core Principles

### 1. Development Environment

**Local Development + GitHub Actions CI**

- ✅ **Develop locally** - Make code changes on your machine (Crucible for macOS GUI development)
- ✅ **Test locally** - Build and test with Qt (see [VERSIONS.md](VERSIONS.md) for current version)
- ✅ **Commit and push** - Push changes to repository
- ✅ **CI validates** - GitHub Actions runs tests automatically on Linux and macOS

> **Note:** For detailed development workflow, see [DEVELOPMENT_WORKFLOW.md](DEVELOPMENT_WORKFLOW.md). For toolchain versions, see [VERSIONS.md](VERSIONS.md).

### 2. C++17 Standard

Phoenix uses C++17 as the baseline. Use modern C++ features appropriately:

- Use `auto` for type deduction when it improves readability
- Prefer range-based for loops
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers
- Use `std::optional` for nullable values
- Prefer `constexpr` and `const` where possible

### 3. Qt Best Practices

- Use Qt's parent-child ownership model for memory management
- Prefer Qt containers (`QList`, `QHash`, `QString`) over STL equivalents for Qt integration
- Use Qt signals/slots for event-driven programming
- Never block the GUI thread (use async operations, timers, or worker threads)
- Use `Q_OBJECT` macro for classes with signals/slots (AUTOMOC handles `.moc` files)

### 4. Memory Safety

- No raw `new`/`delete` in application code (use smart pointers or Qt parent-child)
- No memory leaks (use tools like Valgrind in CI)
- No dangling pointers (prefer references or `QPointer` for Qt objects)

### 5. Performance

- Profile before optimizing
- Avoid premature pessimization
- Use `QHash` over `QMap` for string keys
- Prefer `QString::constData()` when passing to C APIs

---

## Code Style

### Indentation & Formatting

**Indentation:** 4 spaces (no tabs)  
**Line Length:** 100 characters (soft limit)  
**Braces:** Opening brace on same line (K&R style for functions, same line for control flow)

```cpp
// Good
void MyClass::processData(const QString& input) {
    if (input.isEmpty()) {
        return;
    }
    // ...
}

// Bad - opening brace on new line
void MyClass::processData(const QString& input)
{
    if (input.isEmpty())
    {
        return;
    }
}
```

### Naming Conventions

**Classes:** `PascalCase`

```cpp
class IconProvider {
    // ...
};

class MainWindow : public QMainWindow {
    // ...
};
```

**Functions & Variables:** `camelCase`

```cpp
void processIconRequest(const QString& iconName);
QString getThemeColor();
int iconSize = 32;
```

**Constants:** `UPPER_SNAKE_CASE` or `kCamelCase` (for class/namespace constants)

```cpp
static constexpr int MAX_RETRY_ATTEMPTS = 3;
static constexpr quint32 kMagic = 0x504C5452u;
static constexpr int kHeaderSize = 12;
```

**Member Variables:** `camelCase_` (trailing underscore)

```cpp
class MyClass {
private:
    QString name_;
    int count_;
    QTimer* timer_;
};
```

**Private Methods:** `camelCase` (no special prefix)

```cpp
class MyClass {
private:
    void processInternal();
    QString formatData(const QByteArray& data);
};
```

### Header Organization

Headers should follow this order:

```cpp
// 1. Include guard
#pragma once

// 2. Qt includes (grouped, alphabetical within groups)
#include <QMainWindow>
#include <QString>
#include <QTimer>

// 3. Standard library includes
#include <memory>
#include <vector>

// 4. Project includes
#include "IconProvider.h"
#include "ThemeManager.h"

// 5. Forward declarations (if needed)
class QAction;
class QMenu;

// 6. Class declaration
class MainWindow : public QMainWindow {
    Q_OBJECT
    // ...
};
```

### Source File Organization

```cpp
// 1. Corresponding header
#include "MainWindow.h"

// 2. Other project includes
#include "IconProvider.h"
#include "UILogging.h"

// 3. Qt includes
#include <QApplication>
#include <QMenu>
#include <QMessageBox>

// 4. Standard library includes
#include <algorithm>
#include <memory>

// 5. Implementation
void MainWindow::setupMenu() {
    // ...
}
```

---

## Qt-Specific Guidelines

### Signals & Slots

**Use `Q_OBJECT` macro for classes with signals/slots:**

```cpp
class MyClass : public QObject {
    Q_OBJECT

public:
    explicit MyClass(QObject* parent = nullptr);

signals:
    void dataChanged(const QVariant& data);
    void errorOccurred(const QString& message);

public slots:
    void processRequest();
    void handleResponse(const QByteArray& data);

private slots:
    void onTimeout();
};
```

**Prefer new-style syntax (compile-time checked):**

```cpp
// Good - new style (compile-time checked)
connect(button, &QPushButton::clicked, this, &MyClass::onButtonClicked);
connect(timer, &QTimer::timeout, this, &MyClass::onTimeout);

// Acceptable - old style (only if needed for overloaded signals)
connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MyClass::onIndexChanged);
```

**Disconnect in destructor if needed:**

```cpp
MyClass::~MyClass() {
    disconnect(); // Disconnect all signals/slots
    // Or disconnect specific connections
}
```

### Memory Management

**Use Qt's parent-child ownership:**

```cpp
// Good - parent manages lifetime
auto* button = new QPushButton("Click", this);
auto* menu = new QMenu(this);
menu->addAction(action);

// Good - smart pointer for non-Qt objects
auto data = std::make_unique<DataProcessor>();
auto buffer = std::make_shared<Buffer>();

// Bad - manual memory management
QPushButton* button = new QPushButton("Click"); // Who owns this?
```

### Qt Containers

**Prefer Qt containers for Qt integration:**

```cpp
// Good - Qt containers
QList<QString> names;
QHash<QString, int> counts;
QStringList paths;
QByteArray data;

// Acceptable - STL for algorithm-heavy code
std::vector<double> calculations;
std::map<int, std::string> lookup; // But prefer QHash for QString keys
```

### String Handling

**Use `QString` for user-visible text, `QByteArray` for binary data:**

```cpp
// Good
QString userName = tr("User Name");
QString path = QFileInfo(file).absolutePath();
QByteArray rawData = socket->readAll();
QString text = QString::fromUtf8(rawData);

// Good - use QStringLiteral for compile-time string literals
QString title = QStringLiteral("Phoenix");
setWindowTitle(QStringLiteral("Phoenix %1").arg(version));
```

### Event-Driven Programming

**Never block the GUI thread:**

```cpp
// Good - async with signals/slots
void MyClass::startRequest() {
    auto* socket = new QLocalSocket(this);
    connect(socket, &QLocalSocket::connected, this, &MyClass::onConnected);
    connect(socket, &QLocalSocket::readyRead, this, &MyClass::onReadyRead);
    socket->connectToServer(serverName);
}

// Bad - blocking call
void MyClass::startRequest() {
    auto* socket = new QLocalSocket(this);
    socket->connectToServer(serverName);
    socket->waitForConnected(); // BLOCKS GUI THREAD!
    // ...
}
```

---

## Error Handling

### Return Values

**Use return values for expected errors, exceptions for exceptional cases:**

```cpp
// Good - return bool for expected failures
bool loadConfig(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(phxConfig) << "Failed to open config:" << path;
        return false;
    }
    // ...
    return true;
}

// Good - throw for exceptional cases
void criticalOperation() {
    if (!precondition) {
        throw std::runtime_error("Critical precondition failed");
    }
    // ...
}
```

### Logging

**Use Qt's logging categories:**

```cpp
// Declare in header (PhxLogging.h)
Q_DECLARE_LOGGING_CATEGORY(phxUI)

// Define in source (PhxLogging.cpp)
Q_LOGGING_CATEGORY(phxUI, "phx.ui")

// Use in code
qCDebug(phxUI) << "User clicked button:" << buttonName;
qCWarning(phxUI) << "Failed to load icon:" << iconName;
qCCritical(phxUI) << "Critical error:" << errorMessage;
```

---

## Testing Standards

### Unit Tests

**Use Qt Test framework:**

```cpp
#include <QtTest/QtTest>

class IconProviderTest : public QObject {
    Q_OBJECT

private slots:
    void testIconCreation();
    void testIconCaching();
    void testThemeChange();
};

void IconProviderTest::testIconCreation() {
    QIcon icon = IconProvider::icon("save", QSize(32, 32), nullptr);
    QVERIFY(!icon.isNull());
    QVERIFY(icon.availableSizes().contains(QSize(32, 32)));
}

QTEST_MAIN(IconProviderTest)
#include "IconProviderTest.moc"
```

---

## Git Commit Standards

**Format:** `<type>(<scope>): <subject>`

**Examples:**

```bash
feat(icons): add theme-aware icon tinting
fix(menu): correct icon refresh on theme change
refactor(palantir): replace blocking calls with async FSM
docs(readme): update Palantir IPC architecture
test(icons): add icon provider unit tests
```

---

## Pre-Commit Checklist

Before submitting a PR:

- [ ] **Code compiles:** `cmake --build build`
- [ ] **No warnings:** Fix all compiler warnings
- [ ] **Memory safe:** No raw `new`/`delete`, use smart pointers or Qt parent-child
- [ ] **Non-blocking:** No `waitFor*` calls on GUI thread
- [ ] **Logging:** Use appropriate logging categories
- [ ] **Documentation:** Update relevant docs if API changes
- [ ] **Tests pass:** Run unit tests if applicable

---

## Version History

### 1.0.0 (2025-01-21)

- Initial C++17/Qt 6.9 coding standards
- Qt best practices and memory management guidelines
- Signal/slot patterns and event-driven programming
- Error handling and logging standards

---

**Maintained By:** Engineering Team  
**Questions:** See `docs/DEVELOPMENT.md` for development workflow

