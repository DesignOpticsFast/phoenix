#pragma once
#include <QWidget>
class QLabel;

class StepViewer : public QWidget {
  Q_OBJECT
public:
  explicit StepViewer(QWidget* parent=nullptr);
  void setPath(const QString& stepPath);

private:
  QLabel* label_;
};
