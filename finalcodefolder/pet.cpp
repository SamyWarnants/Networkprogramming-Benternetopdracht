#include "pet.h"
#include <QDateTime>
#include <cstdlib>

Pet::Pet(const QString& name, QObject* parent)
    : QObject(parent), name(name), hunger(100), hygiene(100), happiness(100), warnings(0), secretNumber(-1), alive(true) {}

void Pet::startNeglectTimer(std::function<void()> callback) {
    neglectTimer = new QTimer(this);
    connect(neglectTimer, &QTimer::timeout, this, [=]() {
        callback();
    });
    neglectTimer->start(600000); // 10 minutes
}

void Pet::decay() {
    hunger -= 10;
    hygiene -= 10;
    happiness -= 5;
    if (hunger <= 0 || hygiene <= 0 || happiness <= 0) {
        warnings++;
        if (warnings >= 3) {
            alive = false;
            if (neglectTimer) {
                neglectTimer->stop();
                neglectTimer->deleteLater();
                neglectTimer = nullptr;
            }
        }
    }
}

void Pet::feed() {
    hunger = qMin(100, hunger + 30);
    record("Feeding");
}

void Pet::clean() {
    hygiene = qMin(100, hygiene + 30);
    record("Cleaning");
}

void Pet::startGame() {
    secretNumber = (rand() % 10) + 1;
    record("Play started");
}

QString Pet::playGuess(int guess) {
    if (guess == secretNumber) {
        secretNumber = -1;
        happiness = qMin(100, happiness + 20);
        record("Correct guess: " + QString::number(guess));
        return "Yay you guessed it!";
    } else {
        record("Wrong guess: " + QString::number(guess));
        return "n";
    }
}

QStringList Pet::getStats() const {
    return {
        "Tamagotchiland>PetPark?>" + name + ">Stats>Happiness>" + QString::number(happiness) + "%",
        "Tamagotchiland>PetPark?>" + name + ">Stats>Hunger>" + QString::number(hunger) + "%",
        "Tamagotchiland>PetPark?>" + name + ">Stats>Hygiene>" + QString::number(hygiene) + "%"
    };
}

QStringList Pet::getLog() const {
    return log;
}

bool Pet::isAlive() const {
    return alive;
}

QString Pet::getName() const {
    return name;
}

void Pet::addToLog(const QString& entry) {
    log.append(QDateTime::currentDateTime().toString(Qt::ISODate) + ": " + entry);
}

void Pet::record(const QString& action) {
    addToLog(action);
}
