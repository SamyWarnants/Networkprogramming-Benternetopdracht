#include "pet.h"
#include <QDateTime>
#include <cstdlib>
#include <iostream>

constexpr int kNeglectIntervalMs = 600000;

Pet::Pet(const QString& name, QObject* parent)
    : QObject(parent), name(name), hunger(100), hygiene(100), happiness(100), warnings(0), secretNumber(-1), alive(true), neglectTimer(nullptr), coins(0) {
    std::cout << "[INIT] Pet created: " << name.toStdString() << std::endl;
}

void Pet::startNeglectTimer(std::function<void()> callback, int intervalMs) {
    neglectTimer = new QTimer(this);
    connect(neglectTimer, &QTimer::timeout, this, [=]() {
        callback();
    });
    neglectTimer->start(intervalMs > 0 ? intervalMs : kNeglectIntervalMs);
    std::cout << "[TIMER] Started neglect timer for " << name.toStdString()
              << " every " << (intervalMs > 0 ? intervalMs : kNeglectIntervalMs) << "ms" << std::endl;
}

void Pet::decay() {
    std::cout << "[DECAY] " << name.toStdString() << " losing stats..." << std::endl;
    hunger -= 10;
    hygiene -= 10;
    happiness -= 5;
    if (hunger <= 0 || hygiene <= 0 || happiness <= 0) {
        warnings++;
        std::cout << "[WARNING] " << name.toStdString() << " warning #" << warnings << std::endl;
        if (warnings >= 3) {
            alive = false;
            std::cout << "[DEAD] " << name.toStdString() << " has died" << std::endl;
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
    coins += 5;
    std::cout << "[FEED] Fed " << name.toStdString() << " (hunger now " << hunger << ")" << std::endl;
    record("Feeding (+5 coins)");
}

void Pet::clean() {
    hygiene = qMin(100, hygiene + 30);
    coins += 5;
    std::cout << "[CLEAN] Cleaned " << name.toStdString() << " (hygiene now " << hygiene << ")" << std::endl;
    record("Cleaning (+5 coins)");
}

void Pet::startGame() {
    secretNumber = (rand() % 10) + 1;
    std::cout << "[GAME] " << name.toStdString() << " starts game with number: " << secretNumber << std::endl;
    record("Play started");
}

QString Pet::playGuess(int guess) {
    std::cout << "[GUESS] " << name.toStdString() << " guessed: " << guess << std::endl;
    if (guess == secretNumber) {
        secretNumber = -1;
        happiness = qMin(100, happiness + 20);
        coins += 20;
        std::cout << "[GUESS] Correct! Happiness now " << happiness << std::endl;
        record("Correct guess: " + QString::number(guess) + " (+20 coins)");
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
        "Tamagotchiland>PetPark?>" + name + ">Stats>Hygiene>" + QString::number(hygiene) + "%",
        "Tamagotchiland>PetPark?>" + name + ">Stats>Coins>" + QString::number(coins)
    };
}

QStringList Pet::getLog() const {
    return log;
}

bool Pet::isAlive() const {
    return alive;
}

bool Pet::isDead() const {
    return !alive;
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

void Pet::revive() {
    alive = true;
    warnings = 0;
    hunger = 100;
    hygiene = 100;
    happiness = 100;
    std::cout << "[REVIVE] " << name.toStdString() << " has been revived" << std::endl;
    record("Revived by dark magic");
}

void Pet::setCoins(int c) {
    coins = c;
}

int Pet::getCoins() const {
    return coins;
}
