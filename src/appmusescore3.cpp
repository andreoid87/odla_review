#include "appmusescore3.h"
#include <QMap>
#include <QDebug>

App* AppMusescore3::instance(QObject *parent)
{
    if(!_instance)
        return new AppMusescore3(parent);
    return _instance;
}

AppMusescore3::AppMusescore3(QObject *parent) : App(parent, "MUSESCORE3")
{

}


void AppMusescore3::onIncomingData(const QString &speechMessage)
{
    QJsonObject jsonInput = QJsonDocument::fromJson(speechMessage.toUtf8()).object();
    QString outMessage = VoiceOver::instance()->sort(jsonInput, _statusRequested);
    VoiceOver::instance()->say(outMessage);
}

/*!
 * \brief AppMusescore3::send
 *
 *  Load MusescoreCommand from DB and relative vocal guide message
 */
void AppMusescore3::send(QJsonObject command)
{
    if(isDebug)
        qDebug() << "sending: " << command;
    if(_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        QJsonDocument jsDoc(command["command"].toObject());
        QString jsString = QString::fromLatin1(jsDoc.toJson(QJsonDocument::Compact));
        _webSocket.sendTextMessage(jsString);
        if(isDebug)
            qDebug() << "musescore command sent:" << jsString;
    }
}

/*!
 * \brief AppMusescore3::readCursor
 *
 *  Get speech info from AppMusescore during insertion
 */
QString AppMusescore3::readCursor()
{
    QJsonObject command;
    _statusRequested = false;
    command["SpeechFlags"] = VoiceOver::instance()->getCursorFlagsString();

    if(_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        QJsonDocument doc(command);
        QString jsString = QString::fromLatin1(doc.toJson(QJsonDocument::Compact));
        _webSocket.sendTextMessage(jsString);
        //QTextStream(stdout) << doc.toJson(QJsonDocument::Indented) << "\n";
    }
    return ""; //fake string because the real one will arrive with callback
}

/*!
 * \brief AppMusescore3::readStatus
 *
 *  Get speech info from musescore at request
 */
QString AppMusescore3::readStatus()
{
    QJsonObject command;
    _statusRequested = false;
    command["SpeechFlags"] = VoiceOver::instance()->getStatusFlagsString();

    if(_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        QJsonDocument jsDoc(command);
        QString jsString = QString::fromLatin1(jsDoc.toJson(QJsonDocument::Compact));
        _webSocket.sendTextMessage(jsString);
        qDebug() << "musescore command sent:" << jsString;
    }
    return ""; //fake string because the real one will arrive with callback
}
