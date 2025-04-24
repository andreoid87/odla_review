#include "appdorico.h"
#include "metadata.h"

AppDorico::AppDorico(QObject *parent) : App(parent)
{
    qDebug() << "costruttore dorico";
    _appID = "appdorico";
    _state = Disconnected;
    _inStepTimeInput = false;
    _hasScore = false;
    _hasSelection = false;
    connect(&_checkStateTimer, &QTimer::timeout, this, &AppDorico::checkState);
    connect(&_webSocket, &QWebSocket::connected, this, &AppDorico::onSocketConnected);
    connect(&_webSocket, &QWebSocket::disconnected, this, &AppDorico::onClosed);
    connect(&_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred), this, &AppDorico::onError);
    Metadata::addCallableInstance(this, false);
}

/*!
 * \brief AppDorico::send
 *
 *  Load Dorico commands from DB and relative vocal guide message
 */
void AppDorico::send(QJsonObject command)
{
    if(_webSocket.state() == QAbstractSocket::ConnectedState && _hasScore)
    {
        QJsonObject commandObj;
        if(!command.value("any_state").isUndefined())
            commandObj = command.take("any_state").toObject();

        else if(!command.value("input_state").isUndefined() && _inStepTimeInput)
            commandObj = command.take("input_state").toObject();

        else if(!command.value("normal_state").isUndefined()  && !_inStepTimeInput)
            commandObj = command.take("normal_state").toObject();

        if(!commandObj.value("inputmode").isUndefined())
            setNoteInputMode(commandObj.take("inputmode").toBool());

        if(commandObj.take("ifhasselection").toBool() && !_hasSelection)
            return;

        sendJsonObj(commandObj);
    }
}

AppDorico::~AppDorico()
{
    if(_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        QJsonObject outMsg;
        outMsg["message"] = "disconnect";
        sendJsonObj(outMsg);
    }
    _checkStateTimer.stop();
    _webSocket.close();
    deleteLater();
}

QString AppDorico::getSpeechData(QJsonObject cursorWrapper)
{
    // Feedback for blid people during navigation, can we se anything here?
    Q_UNUSED(cursorWrapper);
    return "";
}

void AppDorico::sendJsonObj(QJsonObject msg)
{
    if(msg.isEmpty())
        return;
    QString jsString = QString::fromLatin1(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    //QTextStream(stdout) << "sending: " << jsString << "\n"; QTextStream(stdout).flush();
    _webSocket.sendTextMessage(jsString);
}

void AppDorico::onSocketConnected()
{
    _state = WaitingToken;
    connect(&_webSocket, &QWebSocket::textMessageReceived, this, &AppDorico::onTextMessageReceived);
    QJsonObject msg;
    msg["message"] = "connect";
    msg["clientname"] = "odla keyboard";
    msg["handshakeversion"] = "1.0";
    _token = _db->getSetting("dorico_token", "string_value").toString();

    if(!_token.isEmpty())
        msg["sessiontoken"] = _token;
    sendJsonObj(msg);
}

void AppDorico::onClosed()
{
    qDebug() << "socket closed" << _webSocket.closeCode();
}

void AppDorico::onTextMessageReceived(QString message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject inMsg = doc.object();

    if(!inMsg.value("insteptimeinput").isUndefined())
        _inStepTimeInput = inMsg["insteptimeinput"].toBool();

    if(!inMsg.value("hasscore").isUndefined())
        _hasScore = inMsg["hasscore"].toBool();

    if(!inMsg.value("hasselection").isUndefined())
        _hasSelection = inMsg["hasselection"].toBool();

    //QTextStream(stdout) << doc.toJson(QJsonDocument::Indented) << "\n";
    //qDebug() << doc.toJson(QJsonDocument::Indented);
    switch(_state)
    {
        case WaitingToken:
        {
            if(inMsg["message"].toString() == "sessiontoken")
            {
                _token = inMsg["sessiontoken"].toString();
                _db->setSetting("dorico_token", "string_value", _token);
                QJsonObject outMsg;
                outMsg["message"] = "acceptsessiontoken";
                outMsg["sessiontoken"] = _token;
                sendJsonObj(outMsg);
                _state = WaitingResponse;
            }
            else if(inMsg["code"] == "kconnected")
            {
                _state = Connected;
            }
            else
                _state = Disconnected;
        }
        break;

        case WaitingResponse:
        {
            if(inMsg["code"] == "kconnected")
            _state = Connected;
            _vo->say(_db->speechText("on_dorico_connected"));
        }
        break;

        case Disconnected:
        {
            qDebug() << "something went wrong!!!";
        }
        break;

        case Connected:
        {
            //qDebug() << "received data during connection";
        }
        break;
    }
}

void AppDorico::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    _db->setSetting("dorico_token", "string_value", "");
    _checkStateTimer.start(2000);
    _token = "";
}

/*!
 * \brief Dorico::checkState
 *
 *  Method called periodically until connection is established
 */
void AppDorico::checkState()
{
    if(_webSocket.state() != QAbstractSocket::ConnectedState)
    {
        if(_state == Connected)
        {
            _vo->say(_db->speechText("on_dorico_disconnected"));
            emit appConnected(false);
        }
        _state = Disconnected;
        _webSocket.abort();
        _webSocket.open(QUrl(QStringLiteral("ws://127.0.0.1:4560")));
    }
    else
        emit appConnected(true);
}

void AppDorico::onAppConnectionError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
}

void AppDorico::setNoteInputMode(bool enabled)
{
    QJsonObject msg;
    msg["message"] = "command";

    if(!_inStepTimeInput && enabled)
    {
        msg["command"] = "noteinput.enter?set=true";
        sendJsonObj(msg);
    }
    else if(_inStepTimeInput && !enabled)
    {
        msg["command"] = "noteinput.enter?set=false";
        sendJsonObj(msg);
    }
}




