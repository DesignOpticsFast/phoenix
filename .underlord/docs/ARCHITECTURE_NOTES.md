# Phoenix Architecture Notes

**Last Updated:** 2025-10-14

---

## System Architecture

### High-Level Design

Phoenix follows **Model-View-Controller (MVC)** adapted for Qt:

```
┌────────────────────────────────────────────────┐
│              User Interface (View)             │
│         Qt Widgets, Dialogs, Graphics          │
└───────────────────┬────────────────────────────┘
                    │ signals/slots
┌───────────────────▼────────────────────────────┐
│          Application Layer (Controller)        │
│      Commands, Workflows, State Management     │
└───────────────────┬────────────────────────────┘
                    │ API calls
┌───────────────────▼────────────────────────────┐
│         Integration Layer (Model)              │
│         Bedrock Interface, Data Conversion     │
└───────────────────┬────────────────────────────┘
                    │
┌───────────────────▼────────────────────────────┐
│              Bedrock Library                   │
│          Optical Calculations (C++)            │
└────────────────────────────────────────────────┘
```

---

## Module Breakdown

### UI Layer (`src/ui/`)

**Purpose:** Visual components and user interaction

**Key Components:**
- MainWindow (application shell)
- Dialogs (settings, about, wizards)
- Widgets (custom controls)
- Graphics (2D/3D visualization)

**Design Principles:**
- Thin UI layer - minimal logic
- All business logic in application layer
- Signal/slot for loose coupling

**Example:**
```cpp
class MainWindow : public QMainWindow {
  Q_OBJECT
  
signals:
  void designModified();
  
private slots:
  void on_action_New_triggered();
};
```

---

### Application Layer (`src/application/`)

**Purpose:** Business logic and command pattern

**Command Pattern:**
```cpp
class ModifySurfaceCommand : public QUndoCommand {
public:
  void undo() override;
  void redo() override;
  
private:
  Document* doc_;
  SurfaceParams old_params_;
  SurfaceParams new_params_;
};
```

**Document Model:**
```cpp
class Document : public QObject {
  Q_OBJECT
  
signals:
  void surfaceModified(int id);
  
private:
  LensSystem lens_system_;
};
```

---

### Integration Layer (`src/integration/`)

**Purpose:** Bridge between Qt and Bedrock

**Example:**
```cpp
class BedrockInterface {
public:
  // Qt-friendly API
  QVector<TraceResult> trace_rays(
    const QList<SurfaceData>& surfaces,
    double wavelength,
    int num_rays
  );
  
private:
  bedrock::optics::RayTracer tracer_;
  
  bedrock::geometry::Surface convert(const SurfaceData& qt_surface);
};
```

---

## Data Flow

### User Action → Calculation → Display

```
User clicks "Ray Trace"
    ↓
MainWindow::on_action_RayTrace_triggered()
    ↓ signal
Application::performRayTrace()
    ↓ command
RayTraceCommand::redo()
    ↓ async
BedrockInterface::trace_rays()
    ↓ conversion
bedrock::optics::RayTracer::trace()
    ↓ results
ResultsWidget::displayRayPaths(results)
```

---

## Qt Patterns & Conventions

### Signal/Slot Architecture

```cpp
// Producer
class Calculator : public QObject {
  Q_OBJECT
signals:
  void calculationComplete(const Results& results);
};

// Consumer
class Visualizer : public QWidget {
  Q_OBJECT
private slots:
  void on_calculation_complete(const Results& results);
};
```

---

## Threading & Async Operations

### QThread for Background Work

```cpp
class RayTraceWorker : public QObject {
  Q_OBJECT
  
public slots:
  void performTrace(const TraceParams& params) {
    // Runs in worker thread
    auto results = bedrock_interface_.trace_rays(params);
    emit traceComplete(results);
  }
  
signals:
  void traceComplete(const TraceResults& results);
};
```

---

## Memory Management

### Qt Object Ownership

**Rule:** Parent objects delete children

```cpp
QWidget* parent = new QWidget();
QLabel* label = new QLabel("Text", parent);  // parent owns
delete parent;  // Also deletes label
```

---

## UI Design Guidelines

### Layout Strategy

**Preferred:** Code-based layout

```cpp
auto layout = new QVBoxLayout(this);
layout->addWidget(new QLabel("Properties"));
layout->addWidget(property_editor_);
layout->addStretch();
```

---

## Performance Considerations

### UI Rendering

**Batching:**
```cpp
class RayDisplayWidget : public QWidget {
  void finalize_batch() {
    if (needs_redraw_) {
      update();  // Single redraw
      needs_redraw_ = false;
    }
  }
};
```

### Data Conversion

**Caching:**
```cpp
class BedrockInterface {
  std::unordered_map<int, bedrock::geometry::Surface> surface_cache_;
};
```

---

## Testing Strategy

### Unit Tests (Qt Test Framework)

```cpp
class TestSurfaceModel : public QObject {
  Q_OBJECT
  
private slots:
  void test_add_surface() {
    SurfaceListModel model;
    QCOMPARE(model.rowCount(), 0);
    model.addSurface(SurfaceData{});
    QCOMPARE(model.rowCount(), 1);
  }
};
```

---

## Common Pitfalls

### Issue: Signal/Slot Type Mismatch

```cpp
// Wrong
connect(sender, &Sender::signal(int),
        receiver, &Receiver::slot(double));  // Runtime fail!

// Right
connect(sender, &Sender::signal,
        receiver, &Receiver::slot);  // Compiler checks
```

### Issue: Thread Safety

```cpp
// Wrong - UI from worker thread
void Worker::run() {
  label_->setText("Done");  // CRASH!
}

// Right - signal to main thread
void Worker::run() {
  emit workComplete();  // Safe
}
```

---

**This document should be updated when:**
- Major UI changes
- New layers added
- Qt version upgraded
- Architecture patterns evolve

**Next Review:** After Sprint 10 or Qt 7 migration
