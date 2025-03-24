#include <iostream>
#include <cstdlib>
#include <ctime>
#include <nzmqt/nzmqt.hpp>
#include <QCoreApplication>
#include <QThread>

const QString GUESS_TOPIC_PUSH = "example>guess?>";  // Clients send guesses
const QString GUESS_TOPIC_SUB = "example>guess!>";  // Server responds to users
const QString ANNOUNCEMENT_TOPIC = "example>announcement>"; // Game instructions
int secretNumber;

void generateNewNumber()
{
    secretNumber = (rand() % 10) + 1;
    std::cout << "New Secret Number: " << secretNumber << std::endl;
}

void processGuess(const QString& message, nzmqt::ZMQSocket* pusher)
{
    std::cout << "Received Guess: " << message.toStdString() << std::endl;

    if (message.startsWith(GUESS_TOPIC_PUSH))
    {
        QStringList parts = message.split(">");
        if (parts.size() < 4) {
            std::cout << "Invalid guess format received. Ignoring message." << std::endl;
            return;
        }

        QString username = parts[2]; // Extract username
        int guess = parts[3].toInt();

        QString responseTopic = GUESS_TOPIC_SUB + username + ">";
        QString responseMessage = (guess == secretNumber) ? "y" : "n";

        std::cout << "Sending Response to " << username.toStdString() << ": " << responseMessage.toStdString() << std::endl;
        pusher->sendMessage((responseTopic + responseMessage).toUtf8());

        if (guess == secretNumber)
        {
            std::cout << "Correct guess! Resetting the game." << std::endl;
            generateNewNumber();
        }
    }
}

void sendAnnouncement(nzmqt::ZMQSocket* pusher)
{
    QString message = ANNOUNCEMENT_TOPIC + "Welcome to the number guessing game. Send 'example>guess?>[your_name]>[number]' to play.";
    pusher->sendMessage(message.toUtf8());
    std::cout << "Announcement sent to all users." << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    srand(time(nullptr));
    generateNewNumber();

    try
    {
        nzmqt::ZMQContext* context = nzmqt::createDefaultContext(&app);
        nzmqt::ZMQSocket* pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        nzmqt::ZMQSocket* subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);

        // ✅ Wait for successful connection before proceeding
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

        QThread::sleep(2); // Ensures connection before game starts

        if (!pusher->isConnected() || !subscriber->isConnected()) {
            std::cerr << "Error: Unable to connect to Benternet!" << std::endl;
            return -1;
        }

        std::cout << "Connected successfully! Starting the game..." << std::endl;

        subscriber->subscribeTo(GUESS_TOPIC_PUSH.toUtf8());

        QObject::connect(subscriber, &nzmqt::ZMQSocket::messageReceived, [pusher](const QList<QByteArray>& messages)
                         {
                             for (const QByteArray& message : messages)
                             {
                                 processGuess(QString::fromUtf8(message), pusher);
                             }
                         });

        QTimer::singleShot(2000, [pusher]() {
            sendAnnouncement(pusher);
        });

        context->start();
    }
    catch (nzmqt::ZMQException& ex)
    {
        std::cerr << "Caught an exception: " << ex.what() << std::endl;
    }

    return app.exec();
}
