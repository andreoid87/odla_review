#include <QDebug>
#include <QMap>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include <QRegularExpression>

#include "app.h"
#include "appdorico.h"
#include "appcustom.h"
#include "appmusescore3.h"
#include "appmusescore4.h"
#include "metadata.h"
#include "qprogressdialog.h"

QScopedPointer<App> App::_instance;

App::App(QObject *parent) : QObject(parent)
{
    qDebug() << "App constructor ";
    _db = Database::instance();
    _vo = VoiceOver::instance();
    _connected = false;
    _statusRequested = false;
    connect(&_checkStateTimer, &QTimer::timeout, this, &App::checkState);
    _checkStateTimer.setSingleShot(false);
    _speechLoop = new QEventLoop(this);
    _speechTimer = new QTimer(this);
    _speechTimer->setSingleShot(true);
    connect(&_webSocket, &QWebSocket::textMessageReceived, this, &App::onIncomingData);
    connect(_speechTimer, &QTimer::timeout, _speechLoop, &QEventLoop::quit);
    connect(&_app, &QProcess::readyReadStandardOutput, this, &App::onOutputFromApp);
    QString dbKey = _appID + "_" + currentOS + "_app_path";
    QString appPath = _db->getSetting(dbKey,"string_value").toString();

#ifdef Q_OS_WIN
    _app.setProgram(appPath);
#elif defined(Q_OS_MAC)
    _app.setProgram("open");
    _app.setArguments(QStringList() << "-a" << appPath);
#endif
    _app.setProcessChannelMode(QProcess::MergedChannels);
}

App* App::instance(QObject *parent, QString appID)
{
    qDebug() << "setting software: " << appID;

    if (_instance.isNull() || typeid(*_instance.get()).name() != appID)
    {
        // Crea l'istanza in base al nome della classe
        if (appID == "app_dorico")
            _instance.reset(new AppDorico(parent));
        else if (appID == "app_musescore_3")
            _instance.reset(new AppMusescore3(parent));
        else if (appID == "app_musescore_4")
            _instance.reset(new AppMusescore4(parent));
        else
            _instance.reset(new AppCustom(parent));
    }

    _instance.get()->_appID = appID;
    return _instance.data();
}

/*!
 * \brief App::connectToApp
 *
 * Allow App Socket to connect with this server
 */
void App::connectToApp()
{
    if(_app.program().isEmpty())
        qDebug() << "app path not set, you must start" << _appID << "by yourself";
    else
    {
        qDebug() << "starting: " << _app.program() << _app.arguments();
        _app.startDetached();
    }
    _checkStateTimer.start(2000);
}


/*!
 * \brief App::checkState
 *
 *  Method called periodically in order to check connection status with App
 *
 *  \warning This methods has to be called periodically because some bugs avoid to use
 *  socket error callback slot
 */
void App::checkState()
{
    if(_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        if(!_connected)
        {
            VoiceOver::instance()->say(_db->speechText("on_" + _appID + "_connected"));
            _connected = true;
            emit appConnected(true);
        }
    }
    else
    {
        if(_connected)
        {
            VoiceOver::instance()->say(_db->speechText("on_" + _appID + "_disconnected"));
            _connected = false;
            emit appConnected(false);
        }
        qDebug() << "attempting to connect to " + _db->writtenText(_appID)+ "..." << _webSocket.state();
        QString address     = _db->getSetting(_appID + "_" + "socket_address","string_value").toString();
        QString port        = _db->getSetting(_appID + "_" + "socket_port","string_value").toString();
        qDebug() << "address: " + address + " port: " << port;
        _webSocket.open(QUrl(address + ":" + port));
    }
}

void App::onOutputFromApp()
{
    qDebug() << _app.readAllStandardOutput();
}

/*!
 * \brief App::onAppConnectionError
 * \param socketError
 */
void App::onAppConnectionError(QAbstractSocket::SocketError socketError)
{
    qDebug() << _db->writtenText("on_socket_error") << socketError;
}

App::~App()
{
    _checkStateTimer.stop();
    if (_webSocket.state() == QAbstractSocket::ConnectedState)
        _webSocket.abort();
    deleteLater();
}

QString App::findAppPath(QString defaultPath, QString appName)
{
    defaultPath += appName;
    QString appPath = QDir::toNativeSeparators(defaultPath);

    // Verifica se il file esiste nella directory di default
    if (QFile::exists(appPath))
    {
        qDebug() << "app executable found at default location:" << appPath;
        return appPath;
    }

    // Altrimenti, cerca l'app a partire dalla root di C:
    QDir rootDir("c:/");

    QRegularExpression rx("^(" + QRegularExpression::escape(appName) + ")\\.exe$", QRegularExpression::CaseInsensitiveOption);

    QProgressDialog progressDialog;
    progressDialog.setLabelText("cercando " + appName + "...");
    progressDialog.setRange(0, 100); // Imposta una barra di avanzamento indeterminata
    progressDialog.setModal(true);
    progressDialog.show();

    // Cerca l'applicazione ricorsivamente nella root di C:
    QDirIterator it(rootDir.absolutePath(), QDir::Files | QDir::NoSymLinks | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        progressDialog.setValue(progressDialog.value() + 1);
        if (rx.match(fileInfo.fileName()).hasMatch())
        {
            qDebug() << "app executable found at:" << filePath;
            return filePath;
        }
    }

    // Se non viene trovata l'app, restituisci una stringa vuota
    qDebug() << "app executable not found.";
    return QString();
}
