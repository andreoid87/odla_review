#ifndef APPCUSTOM_H
#define APPCUSTOM_H

#include <QObject>
#include "app.h"

class AppCustom : public App
{
    Q_OBJECT

private:
    AppCustom(QObject *parent);
    int _lowerPitch;
    int _currentKeysig;
    int keyToPitch(int line);
    QList<int> _currentScale = {0, 2, 4, 5, 7, 9, 11};
    QList<int> _keyPitchMap;
    void updateKeyPitchMap();
    // Base scale in MIDI pitches (C major)
    constexpr static const int C_Scale[7] = {0, 2, 4, 5, 7, 9, 11};

    // Order of accidentals from 0 to 6 given to note from C to B
    constexpr static const int accidental[7] = {1, 3, 5, 0, 2, 4, 6};

public slots:
    void send(QJsonObject commandKey) override {};
    void sendMidi(QJsonObject commandKey);
    void sendKeystroke(QJsonObject commandKey);
    void setClef(QJsonObject clefWrapper);
    void setKeysig(QJsonObject keysigWrapper);

private slots:
    void onIncomingData(const QString &speechMessage) override {} // Custom app doesn't handle incoming data yet
    void checkState() override;

    friend class App;
};

#endif // APPCUSTOM_H
