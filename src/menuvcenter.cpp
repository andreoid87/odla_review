#include "menuvcenter.h"

MenuVCenter::MenuVCenter(QWidget *parent, QSqlRecord record) : Menu(parent, record)
{
    
}

/*!
 *  \brief MenuVCenter::command
 *
 *  Invoke method mapped to buttonName
 */
void MenuVCenter::navigator(QJsonObject command)
{
    bool isNumber =_record.value(command["value"].toString()).isValid();
    int numpadPos = _record.value(command["value"].toString()).toInt();

    if(isNumber)
    {
        auto item = itemAtNumpadPos(numpadPos);
        if(item)
        {
           selectElement(item);
           item->command("ENTER");
        }
    }
    else if(command["value"] == "UP")
        selectUpperItem();
    else if(command["value"] == "DOWN")
        selectLowerItem();
    else if(Button::currentButton())
        Button::currentButton()->command(command["value"].toString());
}

/*!
 *  \brief MenuVCenter::loadItems
 *
 *  Load all compatible button under this menu in DB
 */
void MenuVCenter::loadButtons()
{
    QString filters = QString("parentMenu='%1'").arg(menuID());
    filters += QString(" AND NOT type='INSERTION'");
    filters += QString(" AND NOT type='TOGGLE_EX'");
    auto buttonList = _db->allTableRecords("button", filters);

    for(auto &record :buttonList)
    {
        if(record.value(_db->currentSoftware() + "_position").isNull())
            continue;
        
        Button* item;

        if(record.value("type") == "NUMERIC")
            item = new ButtonNumber(this, record);

        else if(record.value("type") == "TOGGLE")
            item = new ButtonToggle(this, record);

        else if(record.value("type") == "STANDARD")
            item = new ButtonCommand(this, record);
        else
            continue;

        item->hideNumber();
        _buttonList.append(item);
    }

    sortItems();

    for(auto &item : _buttonList)
        _numPadLayout->addWidget(item, item->absPos(), 1);
}
