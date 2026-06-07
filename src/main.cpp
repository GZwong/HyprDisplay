#include <QApplication>
#include <QPalette>
#include "services/FileLogService.h"
#include "services/LogService.h"
#include "views/MainWindow.h"

static void applyDarkPalette(QApplication &app)
{
    QApplication::setStyle("Fusion");

    QPalette p;
    p.setColor(QPalette::Window,          QColor(0x2D, 0x2D, 0x3A));
    p.setColor(QPalette::WindowText,      QColor(0xE0, 0xE0, 0xE0));
    p.setColor(QPalette::Base,            QColor(0x22, 0x22, 0x2E));
    p.setColor(QPalette::AlternateBase,   QColor(0x35, 0x35, 0x45));
    p.setColor(QPalette::Text,            QColor(0xE0, 0xE0, 0xE0));
    p.setColor(QPalette::BrightText,      Qt::white);
    p.setColor(QPalette::Button,          QColor(0x3A, 0x3A, 0x4A));
    p.setColor(QPalette::ButtonText,      QColor(0xE0, 0xE0, 0xE0));
    p.setColor(QPalette::Highlight,       QColor(0x42, 0xA5, 0xF5));
    p.setColor(QPalette::HighlightedText, Qt::black);
    p.setColor(QPalette::Link,            QColor(0x42, 0xA5, 0xF5));
    p.setColor(QPalette::ToolTipBase,     QColor(0x3A, 0x3A, 0x4A));
    p.setColor(QPalette::ToolTipText,     QColor(0xE0, 0xE0, 0xE0));
    // Disabled colours — slightly dimmer
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(0x80, 0x80, 0x80));
    p.setColor(QPalette::Disabled, QPalette::Text,       QColor(0x80, 0x80, 0x80));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x80, 0x80, 0x80));

    app.setPalette(p);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("HyprDisplay");
    app.setApplicationVersion("0.1");
    app.setOrganizationName("HyprDisplay");

    LogService::init(std::make_unique<FileLogService>("hyprdisplay.log"));

    applyDarkPalette(app);

    MainWindow window;
    window.show();

    return app.exec();
}
