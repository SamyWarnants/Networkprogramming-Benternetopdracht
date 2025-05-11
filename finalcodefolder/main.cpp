// TamagotchilandService.cpp
#include <QCoreApplication>
#include <nzmqt/nzmqt.hpp>
#include <QTimer>
#include <QDateTime>
#include <QMap>
#include <QStringList>
#include <iostream>
#include <cstdlib>
#include <ctime>

struct Pet {
    bool alive = true;
    int hunger = 100;
    int hygiene = 100;
    int happiness = 100;
    int warnings = 0;
    int secretNumber = -1;
    QStringList log;
    QTimer* neglectTimer = nullptr;
};

QMap<QString, Pet*> pets;

void log(Pet* pet, const QString& entry) {
    pet->log.append(QDateTime::currentDateTime().toString(Qt::ISODate) + ": " + entry);
}

void send(nzmqt::ZMQSocket* socket, const QString& message) {
    socket->sendMessage(message.toUtf8());
    std::cout << "[SEND] " << message.toStdString() << std::endl;
}

void handleNeglect(const QString& name, nzmqt::ZMQSocket* pusher) {
    if (!pets.contains(name)) return;
    Pet* pet = pets[name];
    if (!pet->alive) return;

    pet->hunger -= 10;
    pet->hygiene -= 10;
    pet->happiness -= 5;

    if (pet->hunger <= 0 || pet->hygiene <= 0 || pet->happiness <= 0) {
        if (pet->warnings < 3) {
            send(pusher, "Tamagotchiland>PetPark?>" + name + ">PetAttention>HEY ATTENTION PLEASE");
            pet->warnings++;
        } else {
            send(pusher, "Tamagotchiland>PetPark?>" + name + ">PetAttention>The pet has died");
            pet->alive = false;
            log(pet, "Pet has died");
            if (pet->neglectTimer) {
                pet->neglectTimer->stop();
                pet->neglectTimer->deleteLater();
                pet->neglectTimer = nullptr;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    srand(static_cast<unsigned>(time(nullptr)));

    auto* context = nzmqt::createDefaultContext(&app);
    auto* pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
    auto* subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);

    pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");
    subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

    subscriber->subscribeTo("Tamagotchiland>CreatePet!>");
    subscriber->subscribeTo("Tamagotchiland>PetPark!");

    QObject::connect(subscriber, &nzmqt::ZMQSocket::messageReceived, [=](const QList<QByteArray>& messages) {
        for (const QByteArray& msg : messages) {
            QString message = QString::fromUtf8(msg).trimmed();
            std::cout << "[RECEIVED] " << message.toStdString() << std::endl;

            if (message.startsWith("Tamagotchiland>CreatePet!>Login>")) {
                QString petName = message.section(">", 3);
                if (pets.contains(petName)) {
                    send(pusher, "Tamagotchiland>CreatePet?>" + petName);
                    send(pusher, "Tamagotchiland>CreatePet?>This pet already exists. Try a different name.");
                    continue;
                }

                bool success = (rand() % 100) < 90;
                send(pusher, "Tamagotchiland>CreatePet?>" + petName);

                if (success) {
                    Pet* pet = new Pet();
                    QTimer* timer = new QTimer();
                    QObject::connect(timer, &QTimer::timeout, [=]() {
                        handleNeglect(petName, pusher);
                    });
                    timer->start(600000); // 10 minutes
                    pet->neglectTimer = timer;
                    log(pet, "Pet created");
                    pets.insert(petName, pet);

                    send(pusher, "Tamagotchiland>CreatePet?>Hi thank you for creating me! To play with me go to Tamagotchiland>PetPark!>" + petName);
                } else {
                    send(pusher, "Tamagotchiland>CreatePet?>Oh no it seems that the dark magic hasn't worked please try again");
                }
            }

            else if (message.startsWith("Tamagotchiland>PetPark!>")) {
                QStringList parts = message.split(">");
                if (parts.size() < 4) continue;
                QString name = parts[2];
                QString action = parts[3];

                if (!pets.contains(name)) continue;
                Pet* pet = pets[name];

                if (!pet->alive) {
                    send(pusher, "Tamagotchiland>PetPark?>" + name + ">PetAttention>The pet has died");
                    continue;
                }

                if (action == "Stats") {
                    send(pusher, "Tamagotchiland>PetPark?>" + name + ">Stats>Happiness>" + QString::number(pet->happiness) + "%");
                    send(pusher, "Tamagotchiland>PetPark?>" + name + ">Stats>Hunger>" + QString::number(pet->hunger) + "%");
                    send(pusher, "Tamagotchiland>PetPark?>" + name + ">Stats>Hygiene>" + QString::number(pet->hygiene) + "%");
                    log(pet, "Stats requested");
                }

                else if (action == "Play") {
                    if (parts.size() == 4) {
                        pet->secretNumber = (rand() % 10) + 1;  // 1–10
                        send(pusher, "Tamagotchiland>PetPark?>" + name + ">Play>Okay I have a number start guessing");
                        log(pet, "Play started");
                    }
                    else if (parts.size() == 5) {
                        bool ok;
                        int guess = parts[4].toInt(&ok);
                        if (!ok) continue;

                        if (guess == pet->secretNumber) {
                            send(pusher, "Tamagotchiland>PetPark?>" + name + ">Play>Yay you guessed it!");
                            pet->secretNumber = -1;
                            pet->happiness = qMin(100, pet->happiness + 20);
                            log(pet, "Correct guess: " + QString::number(guess));
                        } else {
                            send(pusher, "Tamagotchiland>PetPark?>" + name + ">Play>n");
                            log(pet, "Wrong guess: " + QString::number(guess));
                        }
                    }
                }

                else if (action == "Petcare" && parts.size() >= 5) {
                    QString task = parts[4];
                    if (task == "Feeding") {
                        pet->hunger = qMin(100, pet->hunger + 30);
                        send(pusher, "Tamagotchiland>PetPark?>" + name + ">Petcare>Thank you for Feeding me!");
                        log(pet, "Feeding");
                    } else if (task == "Cleaning") {
                        pet->hygiene = qMin(100, pet->hygiene + 30);
                        send(pusher, "Tamagotchiland>PetPark?>" + name + ">Petcare>Thank you for Cleaning me!");
                        log(pet, "Cleaning");
                    }
                }

                else if (action == "Logs") {
                    for (const QString& entry : pet->log) {
                        send(pusher, "Tamagotchiland>PetPark?>" + name + ">Logs>" + entry);
                    }
                    log(pet, "Logs viewed");
                }
            }
        }
    });

    QTimer::singleShot(2000, [=]() {
        QString announcement =
            "Welcome to Tamagotchiland!\n"
            "To create a pet, send: Tamagotchiland>CreatePet!>Login>[pet_name]\n"
            "Then interact using Tamagotchiland>PetPark!>[pet_name] with:\n"
            "- Stats\n- Play\n- Play>[guess]\n- Petcare>Feeding\n- Petcare>Cleaning\n- Logs\n";
        send(pusher, "Tamagotchiland>announcement>" + announcement);
    });

    context->start();
    return app.exec();
}
