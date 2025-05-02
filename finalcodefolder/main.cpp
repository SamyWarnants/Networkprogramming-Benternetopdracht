#include <QCoreApplication>
#include <nzmqt/nzmqt.hpp>
#include <QTimer>
#include <cstdlib>
#include <ctime>
#include <iostream>

const QString loginTopic = "Tamagotchiland>CreatePet!>Login";
const QString createPetTopic = "Tamagotchiland>CreatePet?>Larry";

void sendCreationMessages(nzmqt::ZMQSocket* pusher, bool success) {
    if (success) {
        pusher->sendMessage("Tamagotchiland>CreatePet?>Larry");
        pusher->sendMessage("Tamagotchiland>CreatePet?>Hi thank you for creating me! To play with me go to Tamagotchiland>PetPark!>Larry");
        std::cout << "[INFO] Pet created: Larry\n";
    } else {
        pusher->sendMessage("Tamagotchiland>CreatePet?>Oh no it seems that the dark magic hasn't worked please try again");
        std::cout << "[ERROR] Pet creation failed.\n";
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    srand(static_cast<unsigned>(time(nullptr)));

    try {
        nzmqt::ZMQContext* context = nzmqt::createDefaultContext(&app);
        nzmqt::ZMQSocket* pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        nzmqt::ZMQSocket* subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);

        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

        subscriber->subscribeTo(loginTopic.toUtf8());

        QObject::connect(subscriber, &nzmqt::ZMQSocket::messageReceived, [pusher](const QList<QByteArray>& messages) {
            for (const QByteArray& message : messages) {
                QString received = QString::fromUtf8(message);
                std::cout << "[RECEIVED] " << received.toStdString() << std::endl;

                if (received == loginTopic) {
                    // 90% success rate to simulate "dark magic"
                    bool success = (rand() % 10) < 9;
                    sendCreationMessages(pusher, success);
                }
            }
        });

        context->start();
    } catch (const nzmqt::ZMQException& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }

    return app.exec();
}
