#include <QCoreApplication>
#include <nzmqt/nzmqt.hpp>
#include "petmanager.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    auto* context = nzmqt::createDefaultContext(&app);
    auto* pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
    auto* subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);

    pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");
    subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

    subscriber->subscribeTo("Tamagotchiland>CreatePet!>");
    subscriber->subscribeTo("Tamagotchiland>PetPark!");

    PetManager manager(&app, pusher);

    QObject::connect(subscriber, &nzmqt::ZMQSocket::messageReceived, [&manager](const QList<QByteArray>& messages) {
        for (const QByteArray& msg : messages) {
            QString message = QString::fromUtf8(msg).trimmed();
            manager.handleMessage(message);
        }
    });

    QTimer::singleShot(2000, [&]() {
        manager.sendAnnouncement();
    });

    context->start();
    return app.exec();
}
