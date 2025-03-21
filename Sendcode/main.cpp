#include <iostream>
#include <nzmqt/nzmqt.hpp>
#include <QCoreApplication>
#include <QTimer>
#include <QThread>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::cout << "Initializing..." << std::endl;

    try
    {
        nzmqt::ZMQContext *context = nzmqt::createDefaultContext(&app);
        nzmqt::ZMQSocket *pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        nzmqt::ZMQSocket *subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);

        std::string name = "Samy";
        std::string sub_topic = "service>subcounter!>" + name + ">";
        std::string push_message = "service>subcounter?>" + name + ">";

        // Subscribe to messages for Samy
        subscriber->subscribeTo(QString::fromStdString(sub_topic));

        // Handle incoming messages
        QObject::connect(subscriber, &nzmqt::ZMQSocket::messageReceived, [](const QList<QByteArray>& messages) {
            for (const QByteArray &message : messages) {
                std::cout << "Received: " << message.toStdString() << std::endl;
            }
        });

        // Send subscription request
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

        // Wait until sockets are connected before sending
        QTimer::singleShot(2000, [pusher, push_message]() {
            std::cout << "Sending: " << push_message << std::endl;
            pusher->sendMessage(QByteArray::fromStdString(push_message));
        });

        // Ensure sockets are connected
        if (!pusher->isConnected() || !subscriber->isConnected()) {
            std::cerr << "ERROR: Not connected!" << std::endl;
        }

        context->start();
    }
    catch (const nzmqt::ZMQException &ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }

    std::cout << "Started!" << std::endl;
    return app.exec();
}
