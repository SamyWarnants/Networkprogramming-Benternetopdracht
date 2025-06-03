#include <QCoreApplication>
#include <QDir>
#include <QTimer>
#include <nzmqt/nzmqt.hpp>
#include "petmanager.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    auto* context = nzmqt::createDefaultContext(&app);
    auto* pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
    auto* subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);

    pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");
    subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

    // ✅ Subscriptions for all topics used
    subscriber->subscribeTo("Tamagotchiland>CreatePet!>");
    subscriber->subscribeTo("Tamagotchiland>PetPark!");
    subscriber->subscribeTo("Tamagotchiland>DarkMagic");
    subscriber->subscribeTo("Tamagotchiland>Debug>");  // ✅ Added for debug support

    PetManager manager(&app, pusher);

    // Locate project root and load song
    QDir dir(QCoreApplication::applicationDirPath());
    while (dir.dirName() != "finalcodefolderExamen" && dir.cdUp()) {}
    manager.loadLyrics(dir.filePath("C:/Users/samna/Documents/school/netwerkprogramming/netwerkppg/backup/BackupNetwerkprogramming/finalcodefolderExamen/song.json"));

    // Load pets and coins from JSON
    manager.loadFromJson(dir.filePath("C:/Users/samna/Documents/school/netwerkprogramming/netwerkppg/backup/BackupNetwerkprogramming/finalcodefolderExamen/petdata.json"));

    // Handle all incoming messages
    QObject::connect(subscriber, &nzmqt::ZMQSocket::messageReceived, [&manager](const QList<QByteArray>& messages) {
        for (const QByteArray& msg : messages) {
            QString message = QString::fromUtf8(msg).trimmed();
            manager.handleMessage(message);
        }
    });

    // Initial announcement
    QTimer::singleShot(2000, [&]() {
        manager.sendAnnouncement();
    });

    // Auto-save every 10 seconds
    QTimer* saver = new QTimer(&app);
    saver->setInterval(10000);
    QObject::connect(saver, &QTimer::timeout, [&]() {
        QDir saveDir(QCoreApplication::applicationDirPath());
        while (saveDir.dirName() != "C:/Users/samna/Documents/school/netwerkprogramming/netwerkppg/backup/BackupNetwerkprogramming/finalcodefolderExamen/finalcodefolderExamen" && saveDir.cdUp()) {}
        manager.saveToJson(saveDir.filePath("C:/Users/samna/Documents/school/netwerkprogramming/netwerkppg/backup/BackupNetwerkprogramming/finalcodefolderExamen/petdata.json"));
    });
    saver->start();

    context->start();
    return app.exec();
}
