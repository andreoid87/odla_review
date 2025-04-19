#include "database.h"
#include "database.h"
#include <QObject>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include "metadata.h"

Database* Database:: _instance;
QMap<QString,QSqlRelationalTableModel *> Database::_tableMap;

Database *Database::instance(QObject *parent)
{
    if(!_instance)
        _instance = new Database(parent);
    Metadata::addCallableInstance(_instance);
    return _instance;
}

Database::Database(QObject *parent): QObject(parent)
{
    int id = QFontDatabase::addApplicationFont(":/fonts/InriaSans-Bold.ttf");
    _defaultFont = QFont(QFontDatabase::applicationFontFamilies(id).at(0));
    _dbPath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    qRegisterMetaType<modifier_t>("modifier_t");
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _forceReplace = false;
}

void Database::forceReplace()
{
    _forceReplace = true;
}

/*!
 *  \brief Database::initDb
 *  \return success of initialization
 *
 *  Load database file located in a writable path of the computer.
 *  If this file not present or is older than hardcoded one,
 *  replace it with the hardcoded copy.
 */
bool Database::initDb()
{
    QString DBVersion;
    // If database path does not exists create it
    if(!_dbPath.exists())
        _dbPath.mkpath(".");

    //If does not exist yet there was a problem, quit.
    if(!_dbPath.exists())
        return criticalError(tr("Path Error!"), tr("Could not create database directory!"));

    _DBFile.setFileName(_dbPath.filePath("odla.db"));
    setFilePermissions(_DBFile);

    if (!_DBFile.exists())
    {
        // First time starting ODLA, write database
        qDebug() << "No database file found, writing odla.db in" << _dbPath.path();
        QFile::copy(":/odla.db", _dbPath.filePath("odla.db"));
    }
    else
    {
        qDebug() << "Database file found, checking for correct version";
        // Comparing version of current and hardcodeddb
        if(dbBadVersion(&DBVersion) || _forceReplace)
            if(!replaceDBFile())
                return criticalError(tr("File Error!"), tr("Could not replace database file!"));
        // If even after replacing the file, the versions do not match, the hardcoded version needs to be changed
        if(dbBadVersion(&DBVersion))
            return criticalError(tr("File Error!"), tr("Hardcorded DB file (%1) does not corresponding to version (%2)!").arg(DBVersion, VERSION));
    }

    //    _usedDb.setFileName(_dbPath.filePath("odla.db"));

    _DBFile.setFileName(_dbPath.filePath("odla.db"));
    setFilePermissions(_DBFile);
    _db.setDatabaseName(_dbPath.filePath("odla.db"));

    // see https://github.com/devbean/QtCipherSqlitePlugin/issues/16
    //_db.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; SQLCIPHER_LEGACY=1"); //use these options to use db browser
    //_db.setPassword("n]y4ja:EZdfD$^8E");

    if (!_db.open())
        return criticalError(tr("Database Error!"), _db.lastError().driverText());

    QStringList tables = _db.tables();
    for(auto &tables : _db.tables())
        initTable(tables);

    if(_tableMap["setting"] == nullptr)
        return criticalError(tr("Database Error!"), "No data found!");

    loadKeyStrokes();
    _availableLanguages = getAvailableLanguages();
    _alternativeNumpad = getValue("ALTERNATIVE_NUMPAD", "BOOLEAN_VALUE").toBool();
    qDebug() << "Alternative Keypad? " << _alternativeNumpad;

    emit initialized();
    return true;
}

/*!
 * \brief Database::setValuetoDefault
 *
 * Since in SETTINGS table it has value and defaultValue columns
 * This method simply copies "defaultValue" content in "value" and reboots the program.
 */
void Database::resetDatabase()
{
    qDebug() << "Database reset";
    _db.close();
    replaceDBFile();
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

/*!
 *  \brief Database::getValue
 *  \param key
 *
 *  Get setting from SETTINGS table called by key and return it as
 *  QVariant, so caller has to choice  the variable-type to convert to
 */
QVariant Database::getValue(QString objID, QString type)
{
    //qDebug() << "get value" << objID << type;
    QString tableName = "setting";
    QString filter = QString("obj_ref_id='%1' AND type='%2'").arg(objID,type);
    auto retVal = getFirstRecordFrom(tableName, filter).value("value");
    //qDebug() << "found" << retVal;
    return retVal;
}

/*!
 *  \brief Database::fetchSetting
 *  \param arguments
 *
 * Do the same of getValue but only for text - can be used external
 * \todo: create a standard for this kind of call
 */
QString Database::fetchSetting(QJsonObject arguments)
{
    return getValue(arguments["obj_id"].toString(), arguments["type"].toString()).toString();
}

/*!
 *  \brief Database::setAlternativeKeynum
 *  \param argument(value)
 *
 * Invert numpad keys 7 8 9 with 1 2 3 and vice versa
  */
void Database::setAlternativeKeynum(QJsonObject arguments)
{
    switch (arguments["value"].toInt())
    {
    case 0:
        _alternativeNumpad = false;
        break;
        _alternativeNumpad = true;
    default:
        _alternativeNumpad = !_alternativeNumpad;
        break;
    }
    setValue("ALTERNATIVE_NUMPAD", "BOOLEAN_VALUE", _alternativeNumpad);
}

/*!
 *  \brief Database::getButtonPosition
 *  \param buttonID
 *
 *  Get button position from DB
 */
int Database::getButtonPosition(QString buttonID)
{
    QString tableName = "button";
    QString filter = QString("button_id='%1'").arg(buttonID);
    QString fieldName = currentSoftware() + "_position";
    QSqlRecord record = getFirstRecordFrom(tableName, filter);

    if(record.isEmpty())
        return -1;
    else
        return   record.value(fieldName).toInt();
}

/*!
 *  \brief Database::setButtonPosition
 *  \param buttonID
 *  \param position
 *
 *  Save button position in DB
 */
bool Database::setButtonPosition(QString buttonID, int position)
{
    QString tableName = "button";
    QString fieldName = currentSoftware() + "_position";
    QString filter = QString("button_id='%1'").arg(buttonID);
    return insertToRecord(tableName, filter, fieldName, position, false);
}

/*!
 *  \brief Database::getAvailableApps
 *
 *  Get from DB the columns of available apps
 */
QStringList Database::getAvailableApps()
{
    QStringList retval;
    QSqlRecord record = getFirstRecordFrom("command", "");

    for (int i = 0; i < record.count(); ++i)
        retval.append(record.field(i).name());
    retval.removeAll("cmd_id");
    return retval;
}

/*!
 *  \brief Database::setValue
 *  \param key
 *  \param value
 *
 *  Save new Setting in SETTINGS table if it is present, else create it
 */
bool Database::setValue(QString objID, QString type, QVariant value)
{
    QString tableName = "setting";
    QString fieldName = "value";
    QString filter = QString("obj_ref_id='%1' AND type='%2'").arg(objID, type);
    return insertToRecord(tableName, filter, fieldName, value, true);
}

/*!
 *  \brief Database::getTextTranslated
 *  \param textID
 *
 *  Get translated text (according languageSet) in TRANSLATIONS
 *  table and create new row if it's not present.
 *
 *  Primary key is given by the class name of caller + text
 */
QString Database::getTextTranslated(QString textID)
{
    if(textID.isEmpty()) return "";
    QString tableName = "translation";
    QString filter = QString("text_id='%1'").arg(textID);
    auto record = getFirstRecordFrom(tableName, filter);
    return record.value(getLanguage()).toString();
}

/*!
 * \brief Database::getLanguage
 *
 *  get current setted language as string "it" "en", ecc.
 */
QString Database::getLanguage()
{
    QString lang = getValue("LANGUAGE", "MULTI_CHOICE_VALUE").toString();
    if(lang == "system")
    {
        QString systemLanguage = QLocale::system().uiLanguages().first().split('-').first();
        return _availableLanguages.contains(systemLanguage) ? systemLanguage : "en";
    }
    return lang;
}

/*!
 * \brief Database::speechText
 *
 *  get speech version of translated Text: internal version
 */
QString Database::speechText(QString textID)
{
    return Metadata::resolveString(Database::instance()->getTextTranslated(textID)).split("|").last();
}
/*!
 * \brief Database::speechText
 *
 *  get speech version of translated Text : internal version
 */
QString Database::writtenText(QString textID)
{
    return Metadata::resolveString(Database::instance()->getTextTranslated(textID)).split("|").first();
}

/*!
 * \brief Database::speechText
 *
 *  get speech version of translated Text: external version
 */
QString Database::speechText(QJsonObject wrapper)
{
    return Metadata::resolveString(Database::instance()->getTextTranslated(wrapper.value("textID").toString())).split("|").last();
}
/*!
 * \brief Database::speechText
 *
 *  get speech version of translated Text : external version
 */
QString Database::writtenText(QJsonObject wrapper)
{
    return Metadata::resolveString(Database::instance()->getTextTranslated(wrapper.value("textID").toString())).split("|").first();
}

QStringList Database::getAvailableLanguages()
{
    QStringList retVal;
    for (int i = 1; i < _tableMap["translation"]->columnCount(); i++)
        retVal << _tableMap["translation"]->record().fieldName(i);
    return retVal;
}
/*!
 * \brief Database::getFont
 * \param size
 * \param weigth
 * \param italic
 *
 * Get Font given size, weigth and italic
 */
QFont Database::getFont(int size, QFont::Weight weigth, bool italic)
{
    _defaultFont.setPointSize(size);
    _defaultFont.setWeight(weigth);
    _defaultFont.setItalic(italic);
    return _defaultFont;
}

/*!
 *  \brief Database::criticalError
 *  \param title
 *  \param message
 *
 *  Show blocking messagebox with critical error with given title and message (why is it placed here?)
 */
bool Database::criticalError(QString title, QString message)
{
    QMessageBox msgBox(QMessageBox::Critical, title, message, QMessageBox::Ok);
    msgBox.exec();
    return false;
}

/*!
 *  \brief Database::setFilePermissions
 *  \param file
 *
 *  Set all read and write permissions for file
 */
void Database::setFilePermissions(QFile &file)
{
    file.setPermissions(QFileDevice::ReadOwner |
                        QFileDevice::ReadGroup |
                        QFileDevice::ReadOther |
                        QFileDevice::WriteOwner|
                        QFileDevice::WriteGroup|
                        QFileDevice::WriteOther);
}

/*!
 *  \brief Database::replaceDBFile
 *  \return success (boolean)
 *
 *  Replace current database file stored in appdata folder
 */
bool Database::replaceDBFile()
{
    // New version of ODLA installed: remove old database
    qDebug() << "replacing db file";
    // Close file if opened
    if (_DBFile.isOpen())
        _DBFile.close();

    // Remove old file
    if (!_DBFile.remove())
        return criticalError(tr("File Error!"), tr("Could not remove old database!\nMaybe in use by another program?"));

    // Copy new file in local folder
    if (!QFile::copy(":/odla.db", _dbPath.filePath("odla.db")))
        return criticalError(tr("File Error!"), tr("Could not copy default database file!"));

    // Check if file was correctly copied
    if (!_DBFile.exists())
        return criticalError(tr("Database Error!"), tr("Could not create database file!"));

    // If we come from a forced replacement avoid a future unintentioned replacement
    _forceReplace = false;
    return true;
}

/*!
 *  \brief Database::dbBadVersion
 *  \param path
 *
 *  Returns true if DB returns error, otherwise false
 */
bool Database::dbBadVersion(QString *DBVersion)
{
    QFile dbFile(_dbPath.filePath("odla.db"));

    if (!dbFile.exists())
        return true;

    _db.setDatabaseName(_dbPath.filePath("odla.db"));
    //tmpDB.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; SQLCIPHER_LEGACY=1");
    //tmpDB.setPassword("n]y4ja:EZdfD$^8E");
    if (!_db.open())
        return true;

    QSqlRelationalTableModel  table(nullptr, _db);
    table.setTable("setting");
    table.setFilter("obj_ref_id='VERSION'");
    table.select();

    *DBVersion= table.record(0).value("value").toString();
    QString appVersion = QString(VERSION);

    _db.close();

    if(*DBVersion != appVersion)
        return true;

    return false;
}

/*!
 *  \brief Database::initTable
 *  \param table
 *
 *  Init db table and put it in a QMAP data structure
 *  in order to map all the tables by its name
 */
bool Database::initTable(QString table)
{
    _tableMap[table] = new QSqlRelationalTableModel (nullptr, _db);
    _tableMap[table]->setEditStrategy(QSqlRelationalTableModel ::OnFieldChange);
    _tableMap[table]->setTable(table);
    _tableMap[table]->select();
    return _tableMap[table]->rowCount() != 0;
}

/*!
 *  \brief Database::getKeystrokeCommand
 *  \param fn
 *  \param select
 *  \param chord
 *  \param slur
 *  \param key
 *  \param column
 *
 *  Get from db command associated with keystroke
 *  \warning should pass modifers (fn, select, ...) as flags
 */
QString Database::getKeystrokeCommandID(QStringList keys, QString commandIDColumn)
{
    //Getting comand ID from kestrole table
    keys.sort();
    QString filter = QString("keystroke_id = '%1'").arg(_keystrokeMap.value(keys));
    return getFirstRecordFrom("keystroke", filter).value(commandIDColumn).toString();
}

/*!
 * \brief Database::getKeyName
 * \param keyNumber
 * \param repeat
 * \param modifier
 *
 * Retrieves the name of the key based on its number.
 * Returns key properties such as whether it's a repeat key or a modifier.
 */
QString Database::getKeyName(int keyNumber, bool *repeat, bool *modifier)
{
    //Getting comand ID from kestrole table
    QString tableName = "key_map";
    QString filter = QString(_alternativeNumpad ? "keyNumAlternative='%1'" : "keyNum='%1'").arg(keyNumber);
    auto record = getFirstRecordFrom(tableName, filter);

    *repeat = record.value("repeat").toBool();
    *modifier = record.value("modifier").toBool();
    return record.value("keyName").toString();
}

/*!
 * \brief Database::getQwertyCode
 * \param readableKey
 *
 * Retrieves the QWERTY code for a readable key.
 */
quint8 Database::getQwertyCode(QString readableKey)
{
    //Getting comand ID from kestrole table
    QString tableName = "qwerty_map";
    QString filter = QString("keyName='%1'").arg(readableKey);
    auto record = getFirstRecordFrom(tableName, filter);

    if(record.value("keyNum").isValid())
        return record.value("keyNum").toUInt();
    else
        return 0;
}

/*!
 * \brief Database::getCommand
 * \param commandID
 *
 * Retrieves the command associated with the given command ID from the database.
 */
QJsonObject Database::getCommand(QString commandID)
{
    //Getting method ID from command table
    QString tableName = "command";
    QString filter = QString("cmd_id='%1'").arg(commandID);
    auto record = getFirstRecordFrom(tableName, filter);
    if(record.isEmpty())
        return QJsonObject();
    QString commandRec = Metadata::resolveString(record.value(currentSoftware()).toString());
    if(commandRec == "MS3")
        commandRec = Metadata::resolveString(record.value("MUSESCORE3").toString());
    QJsonDocument doc = QJsonDocument::fromJson(commandRec.toUtf8());
    QJsonObject retVal = doc.object();
    return retVal;
}

/*!
 *  \brief Database::allTableRecords
 *  \param table
 *  \param filter
 *
 *  Get all records as QList of QSqlRecord given table and filter (@giorgio: to be continued.....)
 */
QList<QSqlRecord> Database::allTableRecords(QString table, QString filter)
{
    QList<QSqlRecord> list;
    if(_tableMap[table] != nullptr)
    {
        _mutex.lock();
        _tableMap[table]->setFilter(filter);
        _tableMap[table]->select();
        _mutex.unlock();
        while (_tableMap[table]->canFetchMore()) _tableMap[table]->fetchMore();
        for(int i = 0; i < _tableMap[table]->rowCount(); i++)
            list.append(_tableMap[table]->record(i));
    }
    return list;
}

QSqlRecord Database::getFirstRecordFrom(QString tableName, QString filter)
{
    QSqlRecord retVal;
    auto table = _tableMap[tableName];
    if(table == nullptr)
        return retVal;

    _mutex.lock();
    table->setFilter(filter);
    table->select();
    _mutex.unlock();

    if(table->rowCount() == 0)
        return retVal;

    return table->record(0);
}

bool Database::insertToRecord(QString tableName, QString filter, QString fieldName, QVariant value, bool createIfNotExist)
{
    auto record = getFirstRecordFrom(tableName, filter);

    // If not exist record create new
    if(!record.value(fieldName).isValid() && createIfNotExist)
    {
        record.setValue(fieldName, value);
        _mutex.lock();
        bool retVal = _tableMap[tableName]->insertRecord(-1, record);
        _mutex.unlock();
        return retVal;
    }

    // If exist record set new value
    if (record.value(fieldName) != value)
    {
        record.setValue(fieldName, value);
        _mutex.lock();
        bool retVal = _tableMap[tableName]->setRecord(0, record);
        _mutex.unlock();
        return retVal;
    }
    return false;
}

void Database::loadKeyStrokes()
{
    auto records = allTableRecords("keystroke");

    for(auto &record : records)
    {
        QStringList keystroke;
        if(!record.value("key1").isNull())
            keystroke.append(record.value("key1").toString());
        if(!record.value("key2").isNull())
            keystroke.append(record.value("key2").toString());
        if(!record.value("key3").isNull())
            keystroke.append(record.value("key3").toString());
        keystroke.sort();
        _keystrokeMap[keystroke] = record.value("keystroke_id").toInt();
    }
}

