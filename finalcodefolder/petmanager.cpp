#include "petmanager.h"
#include <QTimer>
#include <QDateTime>
#include <QString>

PetManager::PetManager(QObject* parent, nzmqt::ZMQSocket* pusher)
    : pusher(pusher), context(parent) {}

void PetManager::handleMessage(const QString& message) {
    if (message.startsWith("Tamagotchiland>CreatePet!>Login>")) {
        QString name = message.section(">", 3);
        if (name.isEmpty()) {
            pusher->sendMessage(QString("Tamagotchiland>CreatePet?>Invalid pet name").toUtf8());
            return;
        }
        if (pets.contains(name)) {
            pusher->sendMessage(QString("Tamagotchiland>CreatePet?>This pet already exists. Try another.").toUtf8());
            return;
        }

        Pet* pet = new Pet(name);
        pet->startNeglectTimer([=]() {
            pet->decay();
            if (!pet->isAlive()) {
                logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>PetAttention>The pet has died").arg(name));
            }
        });

        pets.insert(name, pet);
        logAndSend(pet, QString("Tamagotchiland>CreatePet?>%1").arg(name));
        logAndSend(pet, QString("Tamagotchiland>CreatePet?>Hi thank you for creating me! To play with me go to Tamagotchiland>PetPark!>%1").arg(name));
    }

    else if (message.startsWith("Tamagotchiland>PetPark!>")) {
        QStringList parts = message.split(">");
        if (parts.size() < 4) return;
        QString name = parts[2];
        QString action = parts[3];

        if (!pets.contains(name)) return;
        Pet* pet = pets[name];
        if (!pet->isAlive()) {
            pusher->sendMessage(QString("Tamagotchiland>PetPark?>%1>PetAttention>The pet has died").arg(name).toUtf8());
            return;
        }

        if (action == "Stats") {
            for (const QString& stat : pet->getStats()) {
                pusher->sendMessage(stat.toUtf8());
            }
        } else if (action == "Play") {
            if (parts.size() == 4) {
                pet->startGame();
                logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>Play>Okay I have a number start guessing").arg(name));
            } else if (parts.size() == 5) {
                bool ok;
                int guess = parts[4].toInt(&ok);
                if (ok) {
                    QString reply = pet->playGuess(guess);
                    logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>Play>%2").arg(name, reply));
                }
            }
        } else if (action == "Petcare" && parts.size() >= 5) {
            QString task = parts[4];
            if (task == "Feeding") {
                pet->feed();
                logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>Petcare>Thank you for Feeding me!").arg(name));
            } else if (task == "Cleaning") {
                pet->clean();
                logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>Petcare>Thank you for Cleaning me!").arg(name));
            }
        } else if (action == "Logs") {
            for (const QString& entry : pet->getLog()) {
                pusher->sendMessage(QString("Tamagotchiland>PetPark?>%1>Logs>%2").arg(name, entry).toUtf8());
            }
        }
    }
}

void PetManager::sendAnnouncement() {
    QString msg =
        "Tamagotchiland>announcement>Welcome to Tamagotchiland!\n"
        "To create a pet, send: Tamagotchiland>CreatePet!>Login>[pet_name]\n"
        "Then interact using Tamagotchiland>PetPark!>[pet_name] with:\n"
        "- Stats\n- Play\n- Play>[guess]\n- Petcare>Feeding\n- Petcare>Cleaning\n- Logs\n";
    pusher->sendMessage(msg.toUtf8());
}

void PetManager::logAndSend(Pet* pet, const QString& response) {
    pet->addToLog(response);
    pusher->sendMessage(response.toUtf8());
}
