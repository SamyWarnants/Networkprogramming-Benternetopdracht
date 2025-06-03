#ifndef PETMANAGER_H
#define PETMANAGER_H

#include "pet.h"
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <nzmqt/nzmqt.hpp>

class PetManager {
public:
    PetManager(QObject* parent, nzmqt::ZMQSocket* pusher);
    void handleMessage(const QString& message);
    void sendAnnouncement();
    void loadLyrics(const QString& filename);
    void saveToJson(const QString& filename);
    void loadFromJson(const QString& filename);

private:
    QMap<QString, Pet*> pets;
    nzmqt::ZMQSocket* pusher;
    QObject* context;

    QStringList songLyrics;
    int songIndex = 0;

    int globalCoins = 0;

    void logAndSend(Pet* pet, const QString& response);
    void broadcast(const QString& response);
};

#endif // PETMANAGER_H
