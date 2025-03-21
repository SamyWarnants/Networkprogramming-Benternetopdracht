#include <iostream>
#include <nzmqt/nzmqt.hpp>
#include <QCoreApplication>
#include <QTimer>

const QString PUBLISH_TOPIC = "example>Aardappel>";  // Topic for sending messages

void sendPoop(nzmqt::ZMQSocket* pusher)
{
    QByteArray message = PUBLISH_TOPIC.toUtf8() + "Aardappel REBORN";
    pusher->sendMessage(message);
    std::cout << "Sent: " << message.toStdString() << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::cout << "Starting Aardappel Sender..." << std::endl;

    try
    {
        // Create ZMQ context and PUSH socket
        nzmqt::ZMQContext* context = nzmqt::createDefaultContext(&app);
        nzmqt::ZMQSocket* pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);

        // ✅ Connect to Benternet's PUSH endpoint
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");

        // Timer to send "poop" every 10 seconds
        QTimer* poopTimer = new QTimer(context);
        QObject::connect(poopTimer, &QTimer::timeout, [pusher]() {
            sendPoop(pusher);
        });

        poopTimer->setInterval(5000); // Send every 10 seconds
        poopTimer->start();

        // Start the ZMQ event loop
        context->start();
    }
    catch (nzmqt::ZMQException& ex)
    {
        std::cerr << "Caught an exception: " << ex.what() << std::endl;
    }

    return app.exec();
}
