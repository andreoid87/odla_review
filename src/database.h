#ifndef DATABASE_H
#define DATABASE_H

#include <QVariant>
#include <QFontDatabase>
#include <QtSql>
#include <QMutex>

typedef QMap<QString, bool> modifier_t;

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
    QString getLanguage(); // language selector
    QFont getFont(int size, QFont::Weight weigth, bool italic = false);
    bool initDb();
    QList<QSqlRecord> allTableRecords(QString table, QString filter = "");

    QString getKeystrokeCommandID(QStringList keys, QString column);
    QJsonObject getCommand(QString commandID);
    QString getTextTranslated(QString textID); // get translated text    
    QString getKeyName(int keyNumber, bool *repeat, bool *modifier);
    quint8 getQwertyCode(QString readableKey);
    bool setButtonPosition(QString buttonID, int position);
    int getButtonPosition(QString objID);
    QStringList getAvailableApps();
    void forceReplace();
    bool insertToRecord(QString tableName, QString filter, QString fieldName, QVariant value, bool createIfNotExist);

public slots:
    QString version() {return QString(VERSION);}
    QVariant getValue(QString key, QString type);
    QString fetchSetting(QJsonObject arguments);
    bool setValue(QString objID, QString type, QVariant value); //setting setter
    void resetDatabase(); //copies "defaultValue" content in "value" and reboots the program
    QString speechText(QString textID);
    QString writtenText(QString textID);
    QString speechText(QJsonObject wrapper);
    QString writtenText(QJsonObject wrapper);
    QString currentSoftware() {return getValue("SOFTWARE","MULTI_CHOICE_VALUE").toString();}
    void setAlternativeKeynum(QJsonObject arguments);

private:
    QSqlDatabase _db;
    static QMap<QString,QSqlRelationalTableModel *> _tableMap;
    static Database * _instance;
    QFont _defaultFont;
    QFile _DBFile;
    QMutex _mutex;
    QMap<QStringList,int> _keystrokeMap;
    QStringList _availableLanguages;
    Database(QObject* parent);
    bool initTable(QString table);
    bool criticalError(QString title, QString message);
    void setFilePermissions(QFile &file);
    bool replaceDBFile();
    QSqlRecord getFirstRecordFrom(QString tableName, QString filter);
    void loadKeyStrokes();
    QStringList getAvailableLanguages();
    QTranslator _qtTranslator;
    QTranslator _qtBaseTranslator;
    QTranslator _odlaTranslator;
    QDir _dbPath;
    bool dbBadVersion(QString *DBVersion);
    bool _forceReplace;
    bool _alternativeNumpad;

signals:
    void initialized();
};

#endif // DATABASE_H
