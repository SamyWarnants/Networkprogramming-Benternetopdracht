#include <iostream>
#include <cstdlib>
#include <ctime>
#include <nzmqt/nzmqt.hpp>
#include <QCoreApplication>
#include <unordered_map>

int secretNumber;  // Random number to guess
std::unordered_map<std::string, bool> playerStatus;  // Track players who guessed correctly

void generateNewNumber() {
    secretNumber = rand() % 11;  // Random number between 0 and 10
    std::cout << "New secret number generated: " << secretNumber << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    srand(time(nullptr));  // Seed random number
    generateNewNumber();

    try
    {
        nzmqt::ZMQContext *context = nzmqt::createDefaultContext(&app);
        nzmqt::ZMQSocket *puller = context->createSocket(nzmqt::ZMQSocket::TYP_PULL, context);
        nzmqt::ZMQSocket *publisher = context->createSocket(nzmqt::ZMQSocket::TYP_PUB, context);

        puller->bindTo("tcp://*:24041"); // Clients send guesses here
        publisher->bindTo("tcp://*:24042"); // Server broadcasts responses here

        QObject::connect(puller, &nzmqt::ZMQSocket::messageReceived, [publisher](const QList<QByteArray> &messages) {
            for (const QByteArray &message : messages) {
                std::string msgStr = message.toStdString();
                std::cout << "Received: " << msgStr << std::endl;

                if (msgStr.rfind("guess>", 0) == 0) {
                    std::string username = msgStr.substr(6, msgStr.find(">") - 6);
                    int guess = std::stoi(msgStr.substr(msgStr.find(">") + 1));

                    std::string responseTopic = "result>" + username + ">";
                    std::string response;

                    if (guess == secretNumber) {
                        response = responseTopic + "Correct! You guessed the number!";
                        std::cout << username << " guessed correctly!" << std::endl;
                        generateNewNumber();  // Reset game
                    } else if (guess < secretNumber) {
                        response = responseTopic + "Too low! Try again.";
                    } else {
                        response = responseTopic + "Too high! Try again.";
                    }

                    publisher->sendMessage(QByteArray::fromStdString(response));
                    std::cout << "Sent: " << response << std::endl;
                }
            }
        });

        context->start();
    }
    catch (const nzmqt::ZMQException &ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }

    std::cout << "Number Guessing Game Server Started!" << std::endl;
    return app.exec();
}
