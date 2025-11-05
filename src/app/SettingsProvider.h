#pragma once

#include <QObject>
#include <QSettings>
#include <memory>

class SettingsProvider : public QObject {
    Q_OBJECT

public:
    SettingsProvider(QObject* parent, std::unique_ptr<QSettings> s)
        : QObject(parent), s_(std::move(s)) {}

    QSettings& settings() const { return *s_; }

private:
    std::unique_ptr<QSettings> s_;
};

