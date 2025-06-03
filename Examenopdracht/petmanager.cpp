#include "petmanager.h"
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static const bool Debug = true;  // 🔧 Enable to allow debug commands

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

    else if (message == "Tamagotchiland>DarkMagic") {
        if (globalCoins >= 250) {
            bool revivedAny = false;
            for (Pet* pet : pets) {
                if (pet->isDead()) {
                    pet->revive();
                    revivedAny = true;
                }
            }
            if (revivedAny) {
                globalCoins -= 250;
                broadcast("Tamagotchiland>DarkMagic?>All pets have been resurrected through dark magic.");
            } else {
                pusher->sendMessage(QString("Tamagotchiland>DarkMagic?>No pets needed resurrection.").toUtf8());
            }
        } else {
            pusher->sendMessage(QString("Tamagotchiland>DarkMagic?>Not enough coins. You need 250.").toUtf8());
        }
    }

    else if (Debug && message.startsWith("Tamagotchiland>Debug>")) {
        QString command = message.section(">", 2);
        if (command == "KillAll") {
            for (Pet* pet : pets) {
                while (pet->isAlive()) {
                    pet->decay(); // Force stats down
                }
                logAndSend(pet, QString("Tamagotchiland>Debug?>%1>Killed").arg(pet->getName()));
            }
            pusher->sendMessage(QString("Tamagotchiland>Debug?>All pets killed").toUtf8());
        }
        else if (command == "GiveMoney") {
            globalCoins = 9999;
            pusher->sendMessage(QString("Tamagotchiland>Debug?>Coins set to 9999").toUtf8());
        }
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
        }
        else if (action == "Play") {
            if (parts.size() == 4) {
                pet->startGame();
                logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>Play>Okay I have a number start guessing").arg(name));
            }
            else if (parts.size() == 5) {
                QString param = parts[4];

                if (param == "Song") {
                    if (songLyrics.isEmpty() || songIndex >= songLyrics.size()) return;
                    QString lyric = songLyrics[songIndex++];
                    for (Pet* p : pets.values()) {
                        QString reply = QString("Tamagotchiland>PetPark?>%1>Sing>%2").arg(p->getName(), lyric);
                        logAndSend(p, reply);
                    }
                }
                else if (param == "FullSong") {
                    if (songLyrics.isEmpty()) return;
                    for (const QString& line : songLyrics) {
                        for (Pet* p : pets.values()) {
                            QString reply = QString("Tamagotchiland>PetPark?>%1>Sing>%2").arg(p->getName(), line);
                            logAndSend(p, reply);
                        }
                    }
                }
                else {
                    bool ok;
                    int guess = param.toInt(&ok);
                    if (ok) {
                        QString reply = pet->playGuess(guess);
                        if (reply.contains("Yay")) {
                            globalCoins += 20;
                            reply += " (+20 coins)";
                        }
                        logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>Play>%2").arg(name, reply));
                    }
                }
            }
        }
        else if (action == "Petcare" && parts.size() >= 5) {
            QString task = parts[4];
            if (task == "Feeding") {
                pet->feed();
                globalCoins += 5;
                logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>Petcare>Thank you for Feeding me! (+5 coins)").arg(name));
            } else if (task == "Cleaning") {
                pet->clean();
                globalCoins += 5;
                logAndSend(pet, QString("Tamagotchiland>PetPark?>%1>Petcare>Thank you for Cleaning me! (+5 coins)").arg(name));
            }
        }
        else if (action == "Logs") {
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
        "- Stats\n- Play\n- Play>[guess]\n"
        "- Play>Song (pets sing one line)\n"
        "- Play>FullSong (pets sing entire song)\n"
        "- Petcare>Feeding\n- Petcare>Cleaning\n- Logs\n"
        "- To revive pets: Tamagotchiland>DarkMagic\n"
        "- If debug is enabled: Tamagotchiland>Debug>KillAll / GiveMoney";
    pusher->sendMessage(msg.toUtf8());
}

void PetManager::logAndSend(Pet* pet, const QString& response) {
    pet->addToLog(response);
    pusher->sendMessage(response.toUtf8());
}

void PetManager::broadcast(const QString& response) {
    for (Pet* pet : pets) {
        pet->addToLog(response);
    }
    pusher->sendMessage(response.toUtf8());
}

void PetManager::loadLyrics(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "[SONG LOAD] Failed to open: " << filename.toStdString() << std::endl;
        return;
    }

    songLyrics.clear();

    if (filename.endsWith(".json", Qt::CaseInsensitive)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject root = doc.object();
        QJsonArray lines = root["lines"].toArray();

        for (const QJsonValue& line : lines) {
            QString text = line.toString().trimmed();
            if (!text.isEmpty())
                songLyrics.append(text);
        }
        std::cout << "[SONG LOAD] Loaded " << songLyrics.size() << " lines from JSON." << std::endl;
    } else {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty())
                songLyrics.append(line);
        }
        std::cout << "[SONG LOAD] Loaded " << songLyrics.size() << " lines from text." << std::endl;
    }

    file.close();

    if (songLyrics.isEmpty()) {
        std::cerr << "[SONG LOAD] No valid lyrics found in " << filename.toStdString() << std::endl;
    }
}

void PetManager::saveToJson(const QString& filename) {
    QJsonObject root;
    QJsonArray petArray;
    for (Pet* pet : pets) {
        QJsonObject obj;
        obj["name"] = pet->getName();
        obj["alive"] = pet->isAlive();
        obj["coins"] = pet->getCoins();
        petArray.append(obj);
    }
    root["pets"] = petArray;
    root["globalCoins"] = globalCoins;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson());
        file.close();
    }
}

void PetManager::loadFromJson(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    globalCoins = root["globalCoins"].toInt();

    QJsonArray petArray = root["pets"].toArray();
    for (const QJsonValue& val : petArray) {
        QJsonObject obj = val.toObject();
        QString name = obj["name"].toString();
        Pet* pet = new Pet(name);
        pet->setCoins(obj["coins"].toInt());
        if (!obj["alive"].toBool()) pet->decay(); // simulate death
        pets.insert(name, pet);
    }
}
