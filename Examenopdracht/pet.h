#ifndef PET_H
#define PET_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <functional>

class Pet : public QObject {
    Q_OBJECT

public:
    explicit Pet(const QString& name, QObject* parent = nullptr);
    void startNeglectTimer(std::function<void()> callback, int intervalMs = 600000);
    void decay();

    void feed();
    void clean();
    void startGame();
    QString playGuess(int guess);

    QStringList getStats() const;
    QStringList getLog() const;
    bool isAlive() const;
    bool isDead() const;
    QString getName() const;
    void addToLog(const QString& entry);

    void revive();
    void setCoins(int coins);
    int getCoins() const;

private:
    QString name;
    int hunger;
    int hygiene;
    int happiness;
    int warnings;
    int secretNumber;
    bool alive;
    QTimer* neglectTimer;
    QStringList log;
    int coins;

    void record(const QString& action);
};

#endif // PET_H
