#include "menuvcenter.h"
#include "src/buttonnumber.h"
#include "src/buttontoggle.h"
#include "src/buttoncommand.h"

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
    QString key = replaceKey(command.value("value").toString());
    bool isNumber;
    int numpadPos = key.toInt(&isNumber);
    qDebug() << "numpadPos" << numpadPos;
    if(isNumber)
    {
        auto item = itemAtNumpadPos(numpadPos);
        if(item)
        {
            selectElement(item);
            item->command("enter");
        }
    }
    else if(command["value"] == "up")
        selectUpperItem();
    else if(command["value"] == "down")
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
    for(auto &record :_db->getMenuButtons(menuID()))
    {
        QString type = record.value("type").toString();

        Button* item;

        if(type == "numeric")
            item = new ButtonNumber(this, record);

        else if(type.contains("toggle"))
            item = new ButtonToggle(this, record);

        else if(type == "standard")
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
