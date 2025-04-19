#include "appcustom.h"
#include <QMap>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDesktopServices>
#include <QScopedPointer>

AppCustom::AppCustom(QObject *parent, QString appID) : App(parent, appID)
{
    qDebug() << "creating instance of" << appID;
    installMusescorePlugin();
}

void AppCustom::onIncomingData(const QString &inputString)
{
    QJsonObject jsonInput = QJsonDocument::fromJson(inputString.toUtf8()).object();
    QString outMessage = VoiceOver::instance()->sort(jsonInput, _statusRequested);
    VoiceOver::instance()->say(outMessage);
}

/*!
 * \brief AppCustom::send
 *
 *  Load MusescoreCommand from DB and relative vocal guide message
 */
void AppCustom::send(QJsonObject command)
{
    if(isDebug)
        qDebug() << "sending: " << command;
    if(_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        QJsonDocument jsDoc(command["command"].toObject());
        QString jsString = QString::fromLatin1(jsDoc.toJson(QJsonDocument::Compact));
        _webSocket.sendTextMessage(jsString);
        if(isDebug)
            qDebug() << _db->writtenText(_appID) + " command sent:" << jsString;
    }
}
/*!
 * \brief AppCustom::requireInput
 *
 *  Get speech info from AppMusescore during insertion
 */
QString AppCustom::requireInput(QJsonObject command)
{
    _statusRequested = false;
    if(_webSocket.state() == QAbstractSocket::ConnectedState)
    {
        QJsonDocument doc(command);
        QString jsString = QString::fromLatin1(doc.toJson(QJsonDocument::Compact));
        _webSocket.sendTextMessage(jsString);
    }
    return ""; //fake string because the real one will arrive with callback
}
/*!
 * \brief AppMusescore3::readCursor
 *
 *  Get speech info from AppMusescore during insertion
 */
QString AppCustom::readCursor()
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
QString AppCustom::readStatus()
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

void AppCustom::installMusescorePlugin()
{
    // Initialize directory path
    QString pluginTargetDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/MuseScore4/Plugins/ODLA";

    // Initialize files path
    QFile logoSourceFile(":/Musescore_Plugin/ODLA.png");
    QFile pluginSourceFile(":/Musescore_Plugin/ODLA.qml");

    // Check if the directory exists, if not, create it
    if (!createFolder(pluginTargetDirPath, false))
        QMessageBox::critical(nullptr, _db->writtenText("ERROR_TITLE"), _db->writtenText("DIR_CREATE_FAIL").arg(pluginTargetDirPath));


    // If files doesn't exist just copy them
    if(! QFile::exists(pluginTargetDirPath + "/ODLA.png"))
    {
        qDebug() << "copying plugin logo...";
        if(!logoSourceFile.copy(pluginTargetDirPath + "/ODLA.png"))
            qDebug() << "falied to copy logo file";
    }
    if(! QFile::exists(pluginTargetDirPath + "/ODLA.qml"))
    {
        qDebug() << "copying plugin qml file...";
        if(!pluginSourceFile.copy(pluginTargetDirPath + "/ODLA.qml"))
            qDebug() << "falied to copy plugin qml file";
    }

    QFile pluginTargetFile = QFile(pluginTargetDirPath + "/ODLA.qml");
    bool updatedPlugin = checkPluginVersion(pluginTargetDirPath + "/ODLA.qml");

    if (!updatedPlugin)
    {
        pluginTargetFile.close();
        setFilePermissions(pluginTargetFile);
        bool removed = pluginTargetFile.remove();
        bool copied = pluginSourceFile.copy(pluginTargetDirPath + "/ODLA.qml");

        if(!removed || !copied)
        {
            qDebug() << "Error target file: " << pluginTargetFile.fileName() << pluginTargetFile.errorString();
            qDebug() << "Error source file: " << pluginSourceFile.fileName() << pluginSourceFile.errorString();
            QMessageBox::critical(nullptr, _db->writtenText("ERROR_TITLE"), _db->writtenText("FILE_COPY_FAIL").arg(pluginSourceFile.fileName(), pluginTargetDirPath));
            // In case of error create folder in order to let the user manually copy files

            QString tempDirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
                                  QDir::separator() + "ODLA_TEMP" + QDir::separator();

            // Create temporary directory
            if (!createFolder(tempDirPath, true)) {
                QMessageBox::critical(nullptr, _db->writtenText("ERROR_TITLE"), _db->writtenText("DIR_CREATE_FAIL").arg(tempDirPath));
                return;
            }

            // Copy logo file in temporary directory
            if (!logoSourceFile.copy(tempDirPath + "ODLA.png"))
                qDebug() << "Copy logo file error:" << logoSourceFile.error() << logoSourceFile.errorString();

            // Copy logo file in temporary directory
            if (!pluginSourceFile.copy(tempDirPath + "ODLA.qml"))
                qDebug() << "Copy plugin file error:" << pluginSourceFile.error() << pluginSourceFile.errorString();

            QDesktopServices::openUrl(tempDirPath);
        }
    }

    // Asserting checkPluginVersion must return true even if compiler must stop
    Q_ASSERT(checkPluginVersion(pluginTargetDirPath + "ODLA.qml"));
}

bool AppCustom::checkPluginVersion(const QString& pluginPath)
{
    qDebug() << "Check plugin version in: " + pluginPath;

    // Usa un QScopedPointer per assicurarti che il file venga chiuso e rilasciato automaticamente
    QScopedPointer<QFile> plugin(new QFile(pluginPath));

    if (plugin->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(plugin.data());
        QString line;
        static const QRegularExpression versionRegex(R"(version:\s*\"(\d+\.\d+\.\d+)\";)");
        QString pluginVersion;
        QRegularExpressionMatch match;
        while (!in.atEnd())
        {
            line = in.readLine();
            match = versionRegex.match(line);
            if (match.hasMatch())
            {
                pluginVersion = match.captured(1);
                break;
            }
        }
        plugin->close();

        qDebug() << "Application version: " << QString(VERSION);
        qDebug() << "Plugin version: " << pluginVersion;

        qDebug() << "QString(VERSION) == pluginVersion" << (QString(VERSION) == pluginVersion);

        return QString(VERSION) == pluginVersion;

    }
    else
    {
        QMessageBox::critical
            (
                nullptr,
                _db->writtenText("ERROR_TITLE"),
                _db->writtenText("FILE_OPEN_FAIL").arg(plugin->fileName())
                );
    }
    return false;
}

bool AppCustom::createFolder(const QString& tempDirPath, bool removeIfExists)
{
    QDir dir(tempDirPath);
    // Se la directory esiste già, la rimuove prima di crearne una nuova
    if (dir.exists())
    {
        if (removeIfExists && !dir.removeRecursively())
        {
            qDebug() << "Unable to remove existing folder: " << tempDirPath;
            return false;
        }
        else
            return true;
    }

    // Crea la directory temporanea
    if (!dir.mkpath("."))
    {
        qDebug() << "Unable to create folder: " << tempDirPath;
        return false;
    }

    qDebug() << "Temporary folder created successfully: " << tempDirPath;
    return true;
}

void AppCustom::setFilePermissions(QFile &file)
{
    file.setPermissions
    (
        QFileDevice::ReadOwner  |    QFileDevice::WriteGroup    |
        QFileDevice::ReadOther  |    QFileDevice::WriteOwner    |
        QFileDevice::ReadGroup  |    QFileDevice::WriteOther
    );
}
