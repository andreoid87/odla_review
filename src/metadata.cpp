#include "metadata.h"
#include "panel.h"

extern bool isDebug;
int Metadata::recursionLevel = 0;
QString Metadata::message = "";
bool Metadata::recursiveClosePanel = false;
QRegularExpression Metadata::filter = QRegularExpression("\\$\\((.+?)\\)");
QMap<QString, QObject*> Metadata::objectsMap;


/*!
 *  \brief Metadata::resolveString
 *  \par baseString
 *
 *  This static method reads a string and tries to replace
 *  $(objectName:nameMethod:arg) string with the QString
 *  output with:
 *  > object named "objectName"
 *  > method named "nameMethod"
 *  > with optional QString "arg" argument.
 */
QString Metadata::resolveString(QString baseString)
{
    if(!baseString.contains("$"))
        return baseString;
    // Create regular expression in order to match some function
    QString finalString = baseString;
    QRegularExpressionMatchIterator i = filter.globalMatch(finalString);
    if(!i.hasNext())
        return baseString;
    // If function detected call and replace it with its return value
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        QString commandID = match.captured(1);
        QString output = invokeWithReturn(commandID);
        finalString.replace("$(" + match.captured(1) + ")", output);
    }
    return finalString;
}

/*!
 *  \brief Metadata::invokeVoid
 *  \par command
 *
 *  This static method uses meta-object features of QT MOC
 *  enginge in order to invoke a QMap<QString, QString>
 *  parameter with key/value pair
 *
 *  The keys are:
 *  > "objName" is the name of the object
 *  > "methodName" is the name of the method
 *  > "value", ... "par4" are four optional arguments
 *  > "arg" is an optional QString argument
 *  > "close_panel" containts directve that can hide panel after command
 */
bool Metadata::invokeVoid(QSqlRecord record)  {
    if(record.isEmpty())
        return false;
    bool retVal = false;

    if(!record.value("method_name").isValid())
        return false;
    QString className = record.value("class_name").toString();
    QString methodName = record.value("method_name").toString();
    QString messageID = record.value("message_done").toString();
    QString argumentsString = resolveString(record.value("arguments").toString());
    bool offlineMethod = record.value("offline").toBool();
    bool closePanel = record.value("close_panel").toBool();

    // if(isDebug)
    //     qDebug() << "Command:"  << className << ":" << methodName + "(" + argumentsString + ")";

    bool allowedCommand = offlineMethod | Panel::instance()->isAppConnected();
    recursiveClosePanel |= closePanel;

    // check recursion level in order to avoid to notify only the latest command message
    recursionLevel++;
    // debug all condition in next if

    if(!className.isEmpty() && !methodName.isEmpty() && allowedCommand)
    {
        QObject * obj = objectsMap.value(className);
        if(obj)
        {
            QJsonObject arguments = Database::instance()->extractJson(argumentsString);
            if(arguments.isEmpty()) //TODO: UNIFY ALL COMMAND FIELDS
            {
                retVal = QMetaObject::invokeMethod(obj, methodName.toUtf8());
            }
            else
            {
                retVal = QMetaObject::invokeMethod(obj, methodName.toUtf8(), Q_ARG(QJsonObject, arguments));
            }
            //Resolve message after command is executed and only at first recursion level in order to have updated status
            // message will be resolved after command is executed to update status
            QString baseMessage = Database::instance()->getTextTranslated(messageID);
            QString thisLevelMessage = resolveString(baseMessage);
            message += (message.isEmpty() ? "" : "; ") + thisLevelMessage;
        }
    }

    if(recursionLevel == 1)
    {
        if(recursiveClosePanel && Panel::instance()->isVisible())
        {
            Panel::instance()->hideMenu();
            if(!message.isEmpty())
                message += "; " + Database::instance()->speechText("closed_window");
        }
        VoiceOver::instance()->say(message);
        message = "";
        recursiveClosePanel = false;
    }

    recursionLevel --;
    return retVal;
}

/*!
 *  \brief Metadata::invokeWithReturn
 *  \par command
 *
 *  This static method uses meta-object features of QT MOC
 *  enginge in order to invoke a QMap<QString, QString>
 *  parameter with key/value pair, differently from invokeVoid
 *  this method returns a QString
 *
 *  The keys are:
 *  > "objName" is the name of the object
 *  > "methodName" is the name of the method
 *  > "value", ... "par4" are four optional arguments
 *  > "arg" is an optional QString argument
 *  > "close_panel" containts directve that can hide panel after command
 */
QString Metadata::invokeWithReturn(QString commandID)
{
    if(commandID.isEmpty())
        return "";
    QString retVal = "";

    if(isDebug)
        qDebug() << "searching command:"  << commandID;

    // Checking wrong command format
    QSqlRecord record = Database::instance()->getCommand(commandID);

    if(!record.value("method_name").isValid())
        return "";
    QString className = record.value("class_name").toString();
    QString methodName = record.value("method_name").toString();
    QString messageID = record.value("message_done").toString();
    QString stringArguments = resolveString(record.value("arguments").toString());

    if(isDebug)
        qDebug() << "Command:"  << className << ":" << methodName + "(" + stringArguments + ")";
    bool allowedCommand = record.value("offline").toBool();
    recursiveClosePanel |= record.value("close_panel").toBool();

    if(recursiveClosePanel)
        Panel::instance()->hideMenu();

    if(className.isEmpty() || methodName.isEmpty())
        return "";

    QObject * obj = objectsMap.value(className);
    if(obj == nullptr)
        return "";

    if(!messageID.isEmpty())
        qDebug() << "Not valid command:, please delete it from DB" << messageID;

    QJsonObject arguments = Database::instance()->extractJson(stringArguments);
    if(arguments.isEmpty()) //TODO: UNIFY ALL COMMAND FIELDS
    {
        QMetaObject::invokeMethod(obj, methodName.toUtf8(), Q_RETURN_ARG(QString, retVal));
    }
    else
    {
        QMetaObject::invokeMethod(obj, methodName.toUtf8(),Q_RETURN_ARG(QString, retVal), Q_ARG(QJsonObject, arguments));
    }

    return retVal;
}

/*!
 *  \brief void::addCallableInstance
 *  \par instance
 *
 *  This static method collect all relevant objects
 *  and associate them with their class Name
 */
void Metadata::addCallableInstance(QObject * instance, bool mapToBaseClassName)
{
    if(instance)
    {
        if(mapToBaseClassName)
            objectsMap[instance->metaObject()->superClass()->className()] = instance;
        else
            objectsMap[instance->metaObject()->className()] = instance;
    }
}
/*!
 *  \brief void::addCallableInstance
 *  \par instance
 *
 *  This static method collect all relevant objects
 *  and associate them with their class Name
 */
void Metadata::removeCallableInstance(QObject * instance, bool mappedToBaseClassName)
{
    if(instance)
    {
        if(mappedToBaseClassName)
            objectsMap.remove(instance->metaObject()->superClass()->className());
        else
            objectsMap.remove(instance->metaObject()->className());
    }
}

