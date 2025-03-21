#include <iostream>
#include <nzmqt/nzmqt.hpp>
#include <QCoreApplication>

const QString NAME = "Samy Warnants"; // Your name
const QString QUEST_TOPIC_PUSH = "example>quest?>";
const QString QUEST_TOPIC_SUB = "example>quest!>";
bool questCompleted = false; // Ensure the exercise completes correctly

void processResponse(const QString& message, nzmqt::ZMQSocket* pusher, nzmqt::ZMQSocket* subscriber)
{
    std::cout << "Ontvangen opdracht: " << message.toStdString() << std::endl;

    // Check if we are given a new subscription & message to send
    if (message.contains("subscribe op") && message.contains("stuur volgende topic en tekst"))
    {
        // Extract the new topic & message
        QStringList parts = message.split("\"");
        if (parts.size() > 3)
        {
            QString newSubscription = parts[1]; // Subscription topic
            QString newMessage = parts[3];      // Message to send

            std::cout << "Nieuwe subscription: " << newSubscription.toStdString() << std::endl;
            std::cout << "Verzenden: " << newMessage.toStdString() << std::endl;

            // Subscribe to the new topic
            subscriber->subscribeTo(newSubscription.toUtf8());

            // Send the new message only once
            pusher->sendMessage(newMessage.toUtf8());

            questCompleted = true; // Mark as completed to prevent unnecessary sending
            return;
        }
    }

    // Default fallback (only if not completed)
    if (!questCompleted)
    {
        QString responseMessage = QUEST_TOPIC_PUSH + NAME + ">";
        pusher->sendMessage(responseMessage.toUtf8());
        std::cout << "Verzonden: " << responseMessage.toStdString() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::cout << "Starting ZMQ Quest Service (Qt Version)..." << std::endl;

    try
    {
        // Create ZMQ context and sockets
        nzmqt::ZMQContext* context = nzmqt::createDefaultContext(&a);
        nzmqt::ZMQSocket* pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        nzmqt::ZMQSocket* subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);

        // Connect sockets to Benternet
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

        // Subscribe to messages with the correct topic
        subscriber->subscribeTo((QUEST_TOPIC_SUB + NAME + ">").toUtf8());

        // Set up message reception handling
        QObject::connect(subscriber, &nzmqt::ZMQSocket::messageReceived, [pusher, subscriber](const QList<QByteArray>& messages)
                         {
                             for (const QByteArray& message : messages)
                             {
                                 QString msgStr = QString::fromUtf8(message);

                                 // Print raw message (debugging)
                                 std::cout << "RAW MESSAGE: " << msgStr.toStdString() << std::endl;

                                 processResponse(msgStr, pusher, subscriber);
                             }
                         });

        // Send initial message after a short delay
        QTimer::singleShot(1000, [pusher]()
                           {
                               QString message = QUEST_TOPIC_PUSH + NAME + ">";
                               pusher->sendMessage(message.toUtf8());
                               std::cout << "Verzonden: " << message.toStdString() << std::endl;
                           });

        // Start the ZMQ event loop
        context->start();
    }
    catch (nzmqt::ZMQException& ex)
    {
        std::cerr << "Caught an exception: " << ex.what() << std::endl;
    }

    return a.exec();
}
