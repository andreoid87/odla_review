#ifndef APPDORICO_H
#define APPDORICO_H

#include "app.h"

class AppDorico : public App
{
    Q_OBJECT

    enum state_t
    {
        Disconnected,
        WaitingToken,
        WaitingResponse,
        Connected
    };


private:
    AppDorico(QObject *parent = nullptr);
    ~AppDorico() override;
    state_t _state;
    bool _connected;
    QString _token;
    QTimer _checkStateTimer;
    bool _inStepTimeInput;
    bool _hasScore;
    bool _hasSelection;
    void sendJsonObj(QJsonObject msg);

public slots:
    void send(QJsonObject command) override;
    bool isConnected() override {return _state == Connected;}
    QString getSpeechData(QJsonObject cursorWrapper);

private slots:
    void onSocketConnected();
    void onClosed();
    void onTextMessageReceived(QString message);
    void onError(QAbstractSocket::SocketError error);
    void setNoteInputMode(bool enabled);

    void checkState() override;
    void onAppConnectionError(QAbstractSocket::SocketError socketError) override;
    void onIncomingData(const QString &speechMessage) override {} // Dorico doesn't handle incoming data

    friend class App;
};

#endif // APPDORICO_H
