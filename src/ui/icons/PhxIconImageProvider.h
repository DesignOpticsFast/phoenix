#pragma once
#include <QQuickImageProvider>
#include <QIcon>
#include <QSize>
#include <QColor>

enum class IconStyle; // Forward declaration

class PhxIconImageProvider : public QQuickImageProvider {
public:
    PhxIconImageProvider();
    QPixmap requestPixmap(const QString& id, QSize* requestedSize, const QSize& defaultSize) override;

private:
    void parseIconId(const QString& id, QString& name, IconStyle& style, int& size, bool& dark);
};