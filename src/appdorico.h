#ifndef APPDORICO_H
#define APPDORICO_H

#include <QObject>
#include <QProcess>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include "voiceover.h"
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

public:
    static App *instance(QObject *parent = nullptr);
    ~AppDorico() override;

private:
    QWebSocket _webSocket;
    state_t _state;
    bool _connected;
    QString _token;
    QTimer _checkStateTimer;
    bool _inStepTimeInput;
    bool _hasScore;
    bool _hasSelection;

    void sendJsonObj(QJsonObject msg);
    explicit AppDorico(QObject *parent = nullptr);

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
    void onIncomingData(const QString &speechMessage) override;

};

#endif // APPDORICO_H
