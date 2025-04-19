#ifndef APP_H
#define APP_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QTimer>
#include <QProcess>
#include <QEventLoop>
#include "database.h"
#include "voiceover.h"

extern QString currentOS;
extern bool isDebug;

class App : public QObject
{
    Q_OBJECT
public:
    
    App();

public:
    static App* instance(QObject *parent = nullptr, QString appID = "");
    virtual ~App();

public slots:
    virtual QString readCursor()    {return "";}
    virtual QString readStatus()    {return "";}
    virtual QString requireInput(QJsonObject command)    {return "";}
    virtual bool isConnected() {return _connected;}
    // Pure virtual slots
    virtual void send(QJsonObject commandKey) = 0;
    virtual void connectToApp();

protected slots:
    virtual void onAppConnectionError(QAbstractSocket::SocketError socketError);
    // Pure virtual slots
    virtual void onIncomingData(const QString &speechMessage) = 0;
    virtual void checkState();
    virtual void onOutputFromApp();

protected:
    App(QObject *parent, QString appID);
    QString findAppPath(QString defaultPath, QString appName);
    static App * _instance;
    static QString _instanceName;
    bool _connected;
    QWebSocket _webSocket;
    bool _cursor;
    QProcess _app;
    QString _appID;
    QTimer _checkStateTimer;
    QEventLoop *_speechLoop;
    QTimer *_speechTimer;
    bool _statusRequested;
    Database *_db;
    VoiceOver *_vo;
    bool runAsAdmin(const QString &command);

signals:
    void appConnected(bool);
};

#endif // APP_H
