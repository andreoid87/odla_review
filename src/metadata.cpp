#include "metadata.h"
#include "panel.h"

extern bool isDebug;
int Metadata::recursionLevel = 0;
QString Metadata::message = "";
bool Metadata::closePanel = false;
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
    // Recursively scan until we get all message
    //    if(finalString.contains(re))
    //        finalString = resolveString(finalString);
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
bool Metadata::invokeVoid(QString commandID, bool softwareConnected)
{
    if(commandID.isEmpty())
        return false;
    bool retVal = false;

    if(isDebug)
        qDebug() << "searching command:"  << commandID;

    // Checking wrong command format
    QJsonObject command = Database::instance()->getCommand(commandID);

    if(isDebug)
        qDebug() << "Found command:"  << command;

    if(command.value("method_name").isUndefined())
        return false;
    QString className = command.take("class_name").toString();
    QString methodName = command.take("method_name").toString();
    QString messageID = command.take("message_done").toString();
    if(isDebug)
        qDebug() << "message_done:"  << messageID;
    bool allowedCommand = command.take("available_offline").toBool() | softwareConnected;
    closePanel |= command.take("close_panel").toBool();

    // check recursion level in order to avoid to notify only the latest command message
    recursionLevel++;
    if(!className.isEmpty() && !methodName.isEmpty() && allowedCommand)
    {
        QObject * obj = objectsMap.value(className);
        if(obj)
        {
            if(command.isEmpty()) //TODO: UNIFY ALL COMMANDS FIELDS
                retVal = QMetaObject::invokeMethod(obj, methodName.toUtf8());
            else
                retVal = QMetaObject::invokeMethod(obj, methodName.toUtf8(), Q_ARG(QJsonObject, command));

            //Resolve message after command is executed and only at first recursion level in order to have updated status
            // message will be resolved after command is executed to update status
            QString baseMessage = Database::instance()->getTextTranslated(messageID);
            QString thisLevelMessage = resolveString(baseMessage);
            message += (message.isEmpty() ? "" : "; ") + thisLevelMessage;
        }
    }
    if(recursionLevel == 1)
    {
        if(closePanel && Panel::instance()->isVisible())
        {
            Panel::instance()->hideMenu();
            if(!message.isEmpty())
                message += "; " + Database::instance()->speechText("CLOSED_WINDOW");
        }
        VoiceOver::instance()->say(message);
        message = "";
        closePanel = false;
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

    // Checking wrong command format
    QJsonObject command = Database::instance()->getCommand(commandID);
    
       if(isDebug)
           qDebug() << "Found command with return:"  << command;

    QString className = command.take("class_name").toString();
    QString methodName = command.take("method_name").toString();
    QString messageID = command.take("message_done").toString();
    bool closePanel = command.take("close_panel").toBool();
    command.remove("available_offline");

    if(closePanel)
        Panel::instance()->hideMenu();

    if(className.isEmpty() || methodName.isEmpty())
        return "";

    QObject * obj = objectsMap.value(className);
    if(obj == nullptr)
        return "";

    QString retVal;
    if(!messageID.isEmpty())
        qDebug() << "here is a message id, please delete it from DB" << messageID;

    if(command.isEmpty())
        QMetaObject::invokeMethod(obj, methodName.toUtf8(), Q_RETURN_ARG(QString, retVal));
    else
        QMetaObject::invokeMethod(obj, methodName.toUtf8(), Q_RETURN_ARG(QString, retVal), Q_ARG(QJsonObject, command));

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
        //        if(!QString("Database").compare(instance->metaObject()->className()))
        //            qDebug() << objectsMap;
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

