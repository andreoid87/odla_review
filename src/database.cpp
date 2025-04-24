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
    return _instance;
}

Database::Database(QObject *parent): QObject(parent)
{
    int id = QFontDatabase::addApplicationFont(":/fonts/InriaSans-Bold.ttf");
    _defaultFont = QFont(QFontDatabase::applicationFontFamilies(id).at(0));
    _dbPath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _forceReplace = false;
    Metadata::addCallableInstance(this);
    updateSoftware();
    updateLanguage();
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

        qDebug() << "dbBadVersion()" << dbBadVersion() << "_forceReplace" << _forceReplace;
        if(dbBadVersion() || _forceReplace)
            if(!replaceDBFile())
                return criticalError(tr("File Error!"), tr("Could not replace database file!"));
        // If even after replacing the file, the versions do not match, the hardcoded version needs to be changed
        if(dbBadVersion())
            return criticalError(tr("File Error!"), tr("Hardcorded DB file does not corresponding to version (%1)!").arg(VERSION));
    }

    _DBFile.setFileName(_dbPath.filePath("odla.db"));
    setFilePermissions(_DBFile);
    _db.setDatabaseName(_dbPath.filePath("odla.db"));

    // see https://github.com/devbean/QtCipherSqlitePlugin/issues/16
    //_db.setConnectOptions("QSQLITE_USE_CIPHER=sqlcipher; SQLCIPHER_LEGACY=1"); //use these options to use db browser
    //_db.setPassword("n]y4ja:EZdfD$^8E");

    if (!_db.open())
        return criticalError(tr("Database Error!"), _db.lastError().driverText());

    for(QString &tables : _db.tables())
        initTable(tables);

    if(_tableMap["menu"] == nullptr)
        return criticalError(tr("Database Error!"), "No data found!");

    _availableLanguages = getAvailableLanguages();
    updateSoftware(); // those two are needed
    updateLanguage(); // to do all queryes
    _alternativeNumpad = getSetting("alternative_numpad", "boolean_value").toBool();
    loadKeyStrokes();
    qDebug() << "Alternative Keypad? " << _alternativeNumpad;

    emit initialized();
    return true;
}

/*!
 * \brief Database::setSettingtoDefault
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
 *  \brief Database::getSetting
 *  \param key
 *
 *  Get setting from SETTINGS table called by key and return it as
 *  QVariant, so caller has to choice  the variable-type to convert to
 */
QVariant Database::getSetting(QString objID, QString type)
{
    QString tableName = "setting";
    QString filter = QString("obj_ref_id='%1' AND type='%2'").arg(objID,type);
    auto retVal = getFirstRecordFrom(tableName, filter).value("value");
    //qDebug() << "found" << retVal;
    return retVal;
}

/*!
 * \brief Database::getButtonState
 * \param buttonID
 * \return state of button
 * Get the state of a button from the database
 */
bool Database::getButtonState(QString buttonID)
{
    QString tableName = "button";
    QString filter = QString("button_id='%1' and software_id = '%2'").arg(buttonID, _currentSoftware);
    return getFirstRecordFrom(tableName, filter).value("type").toString().contains("toggle_on");
}

/*!
 *  \brief Database::getActiveToggleExButtons
 *  \param menuID
 *
 *  Get values of active button (active = true) from "button_toggle"
 *  for all buttons joined with in button table have parent_menu
 *  equal to menuID, using safeQuery method
 */
QVariant Database::getActiveToggleExButtons(QString menuID)
{
    QString query = QString(
                        "SELECT json_extract(arguments, '$.value') AS extracted_value "
                        "FROM button "
                        "WHERE parent_menu = '%1' "
                        "AND button.type = 'toggle_on_ex' "
                        "AND software_id = '%2'"
                        ).arg(menuID, _currentSoftware);

    //qDebug() << "query" << query;

    QSqlQuery q = safeQuery(query);
    if (q.next())
    {
        return q.value("extracted_value");
    }
    return QVariant();
}

/*!
 *  \brief Database::setActiveToggleButtons
 *  \param menuID
 *
 *  Set values of active button (active = 1/0) of "button_toggle" table
 * using safeQuery method and waiting for eventual trigger of the query
 */
bool Database::setActiveToggleButtons(QString buttonID, bool value)
{
    QString query = QString("UPDATE button SET type = REPLACE(type, '%1', '%2') WHERE button_id = '%3' AND software_id = '%4')")
    .arg(value ? "on" : "off")  // Sostituisce 'on' o 'off' in base a 'value'
        .arg(value ? "off" : "on")  // Sostituisce 'off' o 'on' in base a 'value'
        .arg(buttonID)
        .arg(_currentSoftware);

    //qDebug() << query;
    return safeQuery(query).exec();
}

/*
 *  \brief Database::isMethodOffline
 *  \param QMap<QString, QString> command (with software_id, class_name and method_name)
 *  Check if a method can be called offline
 *  checking on function table into "offline" column
 */
bool Database::isMethodOffline(QMap<QString, QString> command)
{
    QString tableName = "function";
    QString filter = QString("software_id = '%1' AND class_name='%2' AND method_name='%3'")
                         .arg(_currentSoftware, command["class_name"], command["method_name"]);
    return getFirstRecordFrom(tableName, filter).value("offline").toBool();
}

/*
 *  \brief Database::haveMethodToClosePanel
 *  \param QMap<QString, QString> command (with software_id, class_name and method_name)
 *  Check if a method have close panel after be called
 *  checking on function table into "close_panel" column
 */
bool Database::haveMethodToClosePanel(QMap<QString, QString> command)
{
    QString tableName = "function";
    QString filter = QString("software_id = '%1' AND class_name='%2' AND method_name='%3'")
                         .arg(_currentSoftware, command["class_name"], command["method_name"]);
    return getFirstRecordFrom(tableName, filter).value("close_panel").toBool();
}

/*!
 *  \brief Database::getActiveToggleButtons
 *  \param menuID
 *
 *  Get title of active button (active = true) from "button_toggle"
 *  for all buttons joined with in button_position table have parent_menu
 *  equal to menuID, using safeQuery method
 */
QString Database::getActiveToggleExButtonTitle(QJsonObject arguments)
{

    QString query = QString("SELECT title_id FROM button "
                            "JOIN button_position ON button_toggle.button_id = button_position.button_id "
                            "JOIN button ON button.button_id = button_position.button_id "
                            "WHERE button_position.parent_menu = '%1' AND button_toggle.active = 1").arg(arguments.value("parent_menu").toString());
    QSqlQuery q = safeQuery(query);
    if (q.next())
        return q.value("title_id").toString();
    return "";
}

/*!
 *  \brief Database::fetchSetting
 *  \param arguments
 *
 * Do the same of getSetting but only for text - can be used external
 * \todo: create a standard for this kind of call
 */
QString Database::fetchSetting(QJsonObject arguments)
{
    //TODO:REMOVE
    return getSetting(arguments["obj_ref_id"].toString(), arguments["type"].toString()).toString();
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
    case 1:
        _alternativeNumpad = true;
        break;
    default:
        _alternativeNumpad = !_alternativeNumpad;
        break;
    }
    setSetting("alternative_numpad", "boolean_value", _alternativeNumpad);
}

/*!
 *  \brief Database::getButtonRecord
 *  \param buttonID
 *
 *  Get button info record from DB
 */
QSqlRecord Database::getButtonRecord(QString buttonID)
{
    QString tableName = "button";
    QString filter = QString("button_id='%1'").arg(buttonID);
    return getFirstRecordFrom(tableName, filter);
}

/*!
 *  \brief Database::getMenuButtons
 *  \param menuID
 *
 *  Get all records of buttons from DB of a specific menu
 *  joining button and function table, foreign keys are
 *  software_id, class_name and method_name
 *  software_id is _currentSoftware
 */
QList<QSqlRecord> Database::getMenuButtons(QString menuID)
{
    QString query = QString(
                        "SELECT button.button_id, button.software_id, button.type, button.class_name, "
                        "button.method_name, button.arguments, button.parent_menu, button.position, "
                        "button.commands, button.title_id, function.close_panel, function.offline, button.message_done "
                        "FROM button "
                        "JOIN function ON button.software_id = function.software_id "
                        "AND button.class_name = function.class_name "
                        "AND button.method_name = function.method_name "
                        "WHERE button.parent_menu = '%1' AND button.software_id = '%2'"
                        ).arg(menuID, _currentSoftware);

    QSqlQuery q = safeQuery(query);
    QList<QSqlRecord> retVal;
    while (q.next())
        retVal.append(q.record());
    return retVal;
}


// /*!
//  *  \brief Database::getButtonCommands
//  *  \param buttonID
//  *  \return QMap of button commands
//  *
//  *  Get the association between button and commands from DB
//  */
// QMap<QString, QString> Database::getButtonCommands(QString buttonID)
// {
//     QString query = QString("SELECT * FROM button_command WHERE button_id = '%1'").arg(buttonID);
//     QSqlQuery q = safeQuery(query);
//     QMap<QString, QString> retVal;
//     while (q.next())
//         retVal[q.value("key").toString()] = q.value("command").toString();
//     return retVal;
// }

/*!
 *  \brief Database::getButtonPositionRecord
 *  \param buttonID
 *
 *  Get button position record from DB
 */
QSqlRecord Database::getButtonPositionRecord(QString buttonID)
{
    QString tableName = "button";
    QString filter = QString("button_id='%1' and software_id = '%2'").arg(buttonID, _currentSoftware);
    return getFirstRecordFrom(tableName, filter);
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
    QString filter = QString("button_id='%1' and software_id = '%2'").arg(buttonID, _currentSoftware);
    return insertToRecord(tableName, filter, "position", position, false);
}

/*!
 *  \brief Database::getAvailableApps
 *
 *  Get from DB the columns of available apps
 */
QMap<QString, QString> Database::getAvailableApps()
{
    QMap<QString, QString> retval;
    // Get a List of all software in the software table ordere by "rank" excluding order = -1 (any)
    QSqlQuery q = safeQuery("SELECT * FROM enum_type_software WHERE rank >= 0 ORDER BY rank ASC");
    while (q.next())
        retval[q.value("software_id").toString()] = q.value("title_id").toString();
    return retval;
}

/*!
 *  \brief Database::safeQuery
 *  \param QueryString
 *
 *  Get QSqlQuery from query string using _mutex
 */
QSqlQuery Database::safeQuery(QString QueryString)
{
    QMutexLocker locker(&_mutex);
    QSqlQuery q(_db);
    q.exec(QueryString);
    return q;
}

/*!
 *  \brief Database::setSetting
 *  \param key
 *  \param value
 *
 *  Save new Setting in SETTINGS table if it is present, else create it
 */
bool Database::setSetting(QString objID, QString type, QVariant value)
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
    QString query = QString("SELECT * FROM translation WHERE text_id = '%1' LIMIT 1").arg(textID);
    QSqlQuery q = safeQuery(query);
    if (q.next())
    {
        return q.value(_currentLanguage).toString();
    }
    return "boh";
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
    return Metadata::resolveString(Database::instance()->getTextTranslated(wrapper.value("text_id").toString())).split("|").last();
}
/*!
 * \brief Database::speechText
 *
 *  get speech version of translated Text : external version
 */
QString Database::writtenText(QJsonObject wrapper)
{
    return Metadata::resolveString(Database::instance()->getTextTranslated(wrapper.value("text_id").toString())).split("|").first();
}

/*!
 * \brief Database::updateSoftware
 *
 *  get current software querying table "enum_type_software" where in_use is 1
 */
void Database::updateSoftware()
{
    _currentSoftware = getFirstRecordFrom("enum_type_software", "in_use = 1").value("software_id").toString();
    loadKeyStrokes();
}


/*!
 * \brief Database::updateLanguage
 *
 *  update current setted language as string "it" "en", ecc.
 */
void Database::updateLanguage()
{
    _currentLanguage = getActiveToggleExButtons("language").toString();

    if(_currentLanguage == "system")
        _currentLanguage = _availableLanguages.contains(getSystemLanguage()) ? getSystemLanguage() : "en";
}

/*!
 * \brief Database::getSystemLanguage
 * \return systemLanguage
 *
 * Return a string with the system language in the format "it" "en" ecc.
 */
QString Database::getSystemLanguage()
{
    return QLocale::system().uiLanguages().first().split('-').first();
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
bool Database::dbBadVersion()
{
    QFile file(":/odla.db");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open database file!";
        return -1;
    }

    // SQLite header: user_version is at offset 60-63 (4 bytes, big-endian)
    file.seek(60);
    QByteArray data = file.read(4);
    if (data.size() != 4) {
        qWarning() << "Failed to read user_version!";
        return -1;
    }

    // Convert bytes to integer (big-endian)
    int userVersion = (static_cast<unsigned char>(data[0]) << 24) |
                      (static_cast<unsigned char>(data[1]) << 16) |
                      (static_cast<unsigned char>(data[2]) << 8) |
                      static_cast<unsigned char>(data[3]);
    qWarning() << "App Version trimmed: " << QString(VERSION).remove(".").toInt();
    qWarning() << "DB PRAGMA user_version: " << userVersion;

    return userVersion != QString(VERSION).remove(".").toInt();
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
 *  \brief Database::getKeystrokeRecord
 *  \param keys (QList of keys listed on key_code_odla table)
 *  \param event (event type)
 *  \param panel (if the command with visible panel)
 *
 *  Get from db command associated with keystroke, combinations
 *  of keystrokes could be found in keystroke_keys swap table for example:
 *  keystroke_id key
 *  0	0
 *  1	0
 *  1	function // 1 is function + 0
 *  2	0
 *  2	select // 2 is select + 0
 *  3	1
 *  4	1
 *  4	function // 4 is function + 1
 *  5	1
 *  5	select
 *  6	2
 *  7	2
 *  7	function
 *
 */
QSqlRecord Database::getKeystrokeRecord(QList<int> keys, QString event, bool panel)
{
    //Getting comand ID from kestroke table
    std::sort(keys.begin(), keys.end());
    QString filter = QString("keystroke_id='%1' and event='%2' and panel='%3' and software_id='%4'")
                         .arg(_keystrokeMap[keys]).arg(event).arg(panel).arg(_currentSoftware);
    return getFirstRecordFrom("keystroke_commands", filter);
}

/*!
 * \brief Database::getRepeatKeys
 * \param keyNumber
 * \param repeat
 * \param modifier
 *
 * Retrieves the name of the key based on its number.
 * Returns key properties such as whether it's a repeat key or a modifier.
 */
QMap<int,bool> Database::getRepeatKeys()
{
    QMap<int,bool> retVal;
    QList<QSqlRecord> records = allTableRecords("key_code_odla");
    for(QSqlRecord record : records)
        retVal[record.value("keynum").toInt()] = record.value("repeat").toBool();
    return retVal;
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
    QString tableName = "key_code_qwerty";
    QString filter = QString("keyname='%1'").arg(readableKey);
    QSqlRecord record = getFirstRecordFrom(tableName, filter);

    if(record.value("keynum").isValid())
        return record.value("keynum").toUInt();
    else
        return 0;
}

/*!
 * \brief Database::getCommand
 * \param commandID
 *
 * Retrieves the command associated with the given command ID from the database.
 */
QSqlRecord Database::getCommand(QString commandID)
{
    //Getting method ID from command table
    QString tableName = "software_command";
    QString filter = QString("cmd_id='%1' and software_id='%2'").arg(commandID).arg(_currentSoftware);
    return getFirstRecordFrom(tableName, filter);
}

QJsonObject Database::extractJson(QString string)
{
    QJsonDocument doc = QJsonDocument::fromJson(string.toUtf8());
    QJsonObject retVal = doc.object();
    return retVal;
}

/*!
 *  \brief Database::allTableRecords
 *  \param table
 *  \param filter
 *
 *  Get all records as QList of QSqlRecord given table and filter
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
    QSqlRelationalTableModel *table = _tableMap[tableName];
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
    QSqlRecord record = getFirstRecordFrom(tableName, filter);

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

/*!
 * \brief Database::loadKeyStrokes
 *
 */
void Database::loadKeyStrokes()
{
    QString query = QString("SELECT keystroke_id, GROUP_CONCAT(keyNum, '+') AS keys FROM keystroke_keys GROUP BY keystroke_id");
    QSqlQuery q = safeQuery(query);
    while (q.next())
    {
        QStringList keysString = q.value("keys").toString().split('+');
        QList<int> keys;
        foreach(QString key, keysString)
            keys.append(key.toInt());
        std::sort(keys.begin(), keys.end());
        _keystrokeMap[keys] = q.value("keystroke_id").toInt();
    }
}

/*!
 * \brief Database::getVersion
 * \return
 *
 * Get the version of this application
 */
QString Database::getVersion()
{
    return QString(VERSION);
}
