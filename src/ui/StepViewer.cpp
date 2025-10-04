#include "StepViewer.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

StepViewer::StepViewer(QWidget* parent) : QWidget(parent) {
  auto* lay = new QVBoxLayout(this);
  label_ = new QLabel("(no file)", this);
  label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  auto* openBtn = new QPushButton("Reveal in Finder", this);
  lay->addWidget(new QLabel("<b>STEP file generated:</b>", this));
  lay->addWidget(label_);
  lay->addWidget(openBtn);
  connect(openBtn, &QPushButton::clicked, this, [this]{
    QDesktopServices::openUrl(QUrl::fromLocalFile(label_->text()));
  });
}

void StepViewer::setPath(const QString& p) {
  label_->setText(QFileInfo(p).absoluteFilePath());
}
