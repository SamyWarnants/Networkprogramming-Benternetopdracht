#ifndef PET_H
#define PET_H

#include <QObject>
#include <QTimer>
#include <QStringList>

class Pet : public QObject {
    Q_OBJECT

public:
    explicit Pet(const QString& name, QObject* parent = nullptr);
    void startNeglectTimer(std::function<void()> callback);
    void decay();

    void feed();
    void clean();
    void startGame();
    QString playGuess(int guess);

    QStringList getStats() const;
    QStringList getLog() const;
    bool isAlive() const;
    QString getName() const;
    void addToLog(const QString& entry);  // ✅ NEW METHOD

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

    void record(const QString& action);
};

#endif // PET_H
