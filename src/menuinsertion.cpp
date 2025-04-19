#include "menuinsertion.h"

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
    if(!Button::currentButton() || Button::currentButton()->table() != "BUTTON_INSERTION")
        return;
    bool isNumber = command["value"].isDouble();
    QString keyName = command["value"].toString();
    
    auto textBox = static_cast<ButtonInsertion*>(Button::currentButton());

    if(isNumber)
        textBox->appendDigit(QString::number(command["value"].toInt()));

    else if(keyName == "UNDO")
        textBox->clearTitle();

    else if(keyName == "UP")
        selectUpperItem();

    else if(keyName == "DOWN")
        selectLowerItem();

    else
        Button::currentButton()->command(keyName);
}

QString MenuInsertion::buttonValue(QJsonObject buttonNumberWrapper)
{
    int buttonNumber = buttonNumberWrapper["value"].toInt();

    if(_buttonList.size() > buttonNumber)
    {
        QString defaultValue = buttonNumberWrapper["defaultValue"].toString();
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
    QString filters = QString("parentMenu='%1'").arg(menuID());
    filters += QString(" AND type='INSERTION'");
    auto buttonListRecord = _db->allTableRecords("button", filters);
    for(auto &record : buttonListRecord)
    {
        if(record.value(_db->currentSoftware() + "_position").isNull())
            continue;
        auto item = new ButtonInsertion(this, record);        
        item->hideNumber();
        _buttonList.append(item);
    }
    sortItems();

    for(auto &item : _buttonList)
        _numPadLayout->addWidget(item, item->absPos(), 1);
}
