#ifndef METADATA_H
#define METADATA_H

#include "qsqlrecord.h"
#include <QObject>

class Metadata : public QObject
{
    Q_OBJECT
public:
    static bool invokeVoid(QSqlRecord record);
    static QString invokeWithReturn(QString command);
    static QObject *getObject(QString instanceName);
    static QString resolveString(QString string);
    static void addCallableInstance(QObject *instance, bool mapToBaseClassName = false);
    static void removeCallableInstance(QObject *instance, bool mapToBaseClassName = false);

private:
    static int recursionLevel;
    static QString message;
    static bool recursiveClosePanel;
    static QRegularExpression filter;
    static QMap<QString, QObject*> objectsMap;
};

#endif // METADATA_H
