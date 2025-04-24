#include "menuinsertion.h"
#include "src/buttoninsertion.h"

MenuInsertion::MenuInsertion(QWidget *parent, QSqlRecord record) : Menu(parent, record)
{
    
    _numPadLayout->setContentsMargins(20,20,20,20);
    _numPadLayout->setSpacing(15);
}

/*!
 *  \brief MenuInsertion::command
 *
 *  Invoke method mapped to buttonName
 */
void MenuInsertion::navigator(QJsonObject command)
{
    // We have to chech if we currently have an instantiated button
    if(!Button::currentButton())
        return;

    auto insertionButton = dynamic_cast<ButtonInsertion*>(Button::currentButton());

    // And also if it's a ButtonInsertion Class instance
    if (insertionButton == nullptr)
        return;

    bool isNumber = command["value"].isDouble();
    QString keyName = command["value"].toString();

    if(isNumber)
        insertionButton->appendDigit(QString::number(command["value"].toInt()));

    else if(keyName == "undo")
        insertionButton->clearTitle();

    else if(keyName == "up")
        selectUpperItem();

    else if(keyName == "down")
        selectLowerItem();
    else
        Button::currentButton()->command(keyName);
}

QString MenuInsertion::buttonValue(QJsonObject buttonNumberWrapper)
{
    int buttonNumber = buttonNumberWrapper["value"].toInt();

    if(_buttonList.size() > buttonNumber)
    {
        QString defaultValue = buttonNumberWrapper["default_value"].toString();
        QString value = _buttonList.at(buttonNumber)->value();
        return value.isEmpty() ? defaultValue : value;
    }
    return "";
}

/*!
 *  \brief MenuInsertion::loadItems
 *
 *  Load all compatible button under this menu in DB
 */
void MenuInsertion::loadButtons()
{
    auto buttonList = _db->getMenuButtons(menuID());
    for(auto &record :buttonList)
    {
        QString type = record.value("type").toString();
        if(type != "insertion")
            continue;
        auto item = new ButtonInsertion(this, record);
        item->hideNumber();
        _buttonList.append(item);
    }
    sortItems();

    for(auto &item : _buttonList)
        _numPadLayout->addWidget(item, item->absPos(), 1);
}
