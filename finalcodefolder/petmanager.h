#ifndef PETMANAGER_H
#define PETMANAGER_H

#include "pet.h"
#include <QObject>
#include <QMap>
#include <QString>
#include <nzmqt/nzmqt.hpp>

class PetManager {
public:
    PetManager(QObject* parent, nzmqt::ZMQSocket* pusher);
    void handleMessage(const QString& message);
    void sendAnnouncement();

private:
    QMap<QString, Pet*> pets;
    nzmqt::ZMQSocket* pusher;
    QObject* context;

    void createPet(const QString& name);
    void logAndSend(Pet* pet, const QString& response);
};

#endif // PETMANAGER_H
