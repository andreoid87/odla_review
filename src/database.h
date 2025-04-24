#ifndef DATABASE_H
#define DATABASE_H

#include <QVariant>
#include <QFontDatabase>
#include <QtSql>
#include <QMutex>

/*!
 *  \brief Database class
 *
 *  This class manage database (only uninque instance: singleton)
 *  It uses QSqlRelationalTableModel  to load tables
 */
class Database : public QObject
{
    Q_OBJECT

public:
    static Database* instance(QObject *parent = nullptr);
    QFont getFont(int size, QFont::Weight weigth, bool italic = false);
    bool initDb();
    QList<QSqlRecord> allTableRecords(QString table, QString filter = "");

    QSqlRecord getKeystrokeRecord(QList<int> keys, QString event, bool panel);
    QSqlRecord getCommand(QString commandID);
    QString getTextTranslated(QString textID); // get translated text
    QMap<int, bool> getRepeatKeys();
    quint8 getQwertyCode(QString readableKey);
    bool setButtonPosition(QString buttonID, int position);
    QSqlRecord getButtonRecord(QString buttonID);
    QSqlRecord getButtonPositionRecord(QString objID);
    QList<QSqlRecord> getMenuButtons(QString menuID);
    QMap<QString, QString> getAvailableApps();
    void forceReplace();
    bool insertToRecord(QString tableName, QString filter, QString fieldName, QVariant value, bool createIfNotExist);
    QJsonObject extractJson(QString string);
    QVariant getActiveToggleExButtons(QString menuID);
    bool setActiveToggleButtons(QString buttonID, bool value);
    bool getButtonState(QString buttonID);
    //QMap<QString, QString> getButtonCommands(QString buttonID);

public slots:
    QString version() {return QString(VERSION);}
    QVariant getSetting(QString key, QString type);
    QString fetchSetting(QJsonObject arguments);
    bool setSetting(QString objID, QString type, QVariant value); //setting setter
    void resetDatabase(); //copies "defaultValue" content in "value" and reboots the program
    QString speechText(QString textID);
    QString writtenText(QString textID);
    QString speechText(QJsonObject wrapper);
    QString writtenText(QJsonObject wrapper);
    void updateSoftware();
    void updateLanguage();
    void setAlternativeKeynum(QJsonObject arguments);
    QString getActiveToggleExButtonTitle(QJsonObject arguments);
    QString getVersion();
    bool isMethodOffline(QMap<QString, QString> command);
    bool haveMethodToClosePanel(QMap<QString, QString> command);

private:
    QSqlDatabase _db;
    static QMap<QString,QSqlRelationalTableModel *> _tableMap;
    static Database * _instance;
    QFont _defaultFont;
    QFile _DBFile;
    QMutex _mutex;
    QMap<QList<int>,int> _keystrokeMap;
    QStringList _availableLanguages;
    Database(QObject* parent);
    bool initTable(QString table);
    bool criticalError(QString title, QString message);
    void setFilePermissions(QFile &file);
    bool replaceDBFile();
    QSqlRecord getFirstRecordFrom(QString tableName, QString filter);
    void loadKeyStrokes();
    QStringList getAvailableLanguages();
    QString getSystemLanguage();
    QTranslator _qtTranslator;
    QTranslator _qtBaseTranslator;
    QTranslator _odlaTranslator;
    QDir _dbPath;
    bool dbBadVersion();
    bool _forceReplace;
    bool _alternativeNumpad;
    QSqlQuery safeQuery(QString QueryString);
    QString _currentLanguage;
    QString _currentSoftware;

signals:
    void initialized();
};

#endif // DATABASE_H
