#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtPlugin>
#include <QtQuick>

#include <cstdlib>
#include <signal.h>

#ifdef EPAPER
// This is required for Qt to display to the reMarkable's display
Q_IMPORT_PLUGIN(QsgEpaperPlugin)
#endif

using namespace std;

void sigHandler(int signal){
    ::signal(signal, SIG_DFL);
    qApp->quit();
}

int main(int argc, char *argv[]){
    auto qt_version = qVersion();
    qDebug() << "Qt Runtime: " << qt_version;
    qDebug() << "Qt Build: " << QT_VERSION_STR;
#ifdef EPAPER
    QCoreApplication::addLibraryPath("/opt/usr/lib/plugins");
    // Setup correct Qt settings for application to load
    // This will overwrite any passed in environment variables
    // If you want to support overriding these values, you will need to either
    //  check that they are not populated here, or move them to a wrapper script
    //  and remove this code
    qputenv("QMLSCENE_DEVICE", "epaper");
    qputenv("QT_QUICK_BACKEND","epaper");
    qputenv("QT_QPA_PLATFORM", "epaper:enable_fonts");
    qputenv("QT_QPA_GENERIC_PLUGINS", "evdevtablet");
    QFile file("/sys/devices/soc0/machine");
    if(!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Couldn't open " << file.fileName();
        return EXIT_FAILURE;
    }
    QTextStream in(&file);
    QString modelName = in.readLine();
    if(modelName.startsWith("reMarkable 2")){
        qDebug() << "RM2 detected...";
        qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "rotate=180:invertx");
        qputenv("QT_QPA_EVDEV_TABLET_PARAMETERS", "");
    }else{
        qDebug() << "RM1 detected...";
        qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "rotate=180");
        qputenv("QT_QPA_EVDEV_TABLET_PARAMETERS", "");
    }
#else
    qDebug() << "Desktop detected...";
#endif
    QGuiApplication app(argc, argv);
    app.setApplicationName("gameboy");
    app.setApplicationDisplayName("Gameboy Emulator");
    app.setApplicationVersion(APP_VERSION);
    // Load main.qml from the embedded Qt resource file.
    // This is defined in qml.qrc
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()){
        qDebug() << "Nothing to display";
        return -1;
    }

    // Setup some signal handlers to make sure to quit the application normally if these signals are recieved
    signal(SIGINT, sigHandler);
    signal(SIGSEGV, sigHandler);
    signal(SIGTERM, sigHandler);

    return app.exec();
}
