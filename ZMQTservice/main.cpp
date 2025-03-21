#include <iostream>
#include <nzmqt/nzmqt.hpp>
#include <nzmqt/impl.hpp>
#include <QCoreApplication>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QDateTime>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::cout << "Prep!" << std::endl;

    try
    {
        nzmqt::ZMQContext *context = nzmqt::createDefaultContext(&a);
        nzmqt::ZMQSocket *pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        nzmqt::ZMQSocket *subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);

        // Handle received messages
        QObject::connect(subscriber, &nzmqt::ZMQSocket::messageReceived, [](const QList<QByteArray>& messages)
                         {
                             if (messages.size() < 1) {
                                 std::cout << "Received empty message!" << std::endl;
                             } else {
                                 std::cout << "Received: " << messages.constFirst().toStdString() << std::endl;
                             }
                         });

        // Timer to send "Aardappel" every 5 seconds
        QTimer *aardappelTimer = new QTimer(context);
        QObject::connect(aardappelTimer, &QTimer::timeout, [pusher]() {
            QByteArray message = "Aardappel";
            pusher->sendMessage(message);
            std::cout << "Sent: Aardappel" << std::endl;
        });
        aardappelTimer->setInterval(5000); // Every 5 seconds
        aardappelTimer->start();

        // Thread to handle manual user input
        QThread *thread = QThread::create([pusher] {
            QTextStream s(stdin);
            while (true) {
                QString input = s.readLine();
                pusher->sendMessage(input.toUtf8());
                std::cout << "Message sent: " << input.toStdString() << std::endl;
            }
        });

        // Connect sockets to Benternet
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

        // Subscribe to all messages
        if (argc > 1) {
            for (int i = 1; i < argc; i++) {
                subscriber->subscribeTo(argv[i]);
            }
        } else {
            subscriber->subscribeTo("");
        }

        // Check connections
        if (!pusher->isConnected() || !subscriber->isConnected()) {
            std::cerr << "NOT CONNECTED!!!" << std::endl;
        }

        context->start();
        thread->start();

    }
    catch (nzmqt::ZMQException &ex)
    {
        std::cerr << "Caught an exception: " << ex.what() << std::endl;
    }

    std::cout << "Start!" << std::endl;
    return a.exec();
}
