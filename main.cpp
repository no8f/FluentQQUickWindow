#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <customquickwindow.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("appFluentQQUickWindow");

    CustomQQuickWindow::registerQmlType();

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("FluentQQUickWindow", "Main");

    return app.exec();
}
