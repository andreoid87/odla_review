#include "menustandard.h"

MenuStandard::MenuStandard(QWidget *parent, QSqlRecord record) : Menu(parent, record)
{
    
}

/*!
 *  \brief MenuStandard::navigator
 *
 *  Invoke method mapped to buttonName
 */
void MenuStandard::navigator(QJsonObject command)
{
    bool isNumber = command["value"].isDouble();
    int numpadPos = command["value"].toInt();
    QString keyName = command["value"].toString();

    if(isNumber)
    {
        auto item = itemAtNumpadPos(numpadPos);
        if(item)
        {
           selectElement(item, true);
           item->command("ENTER");
        }
    }
    else if(standardNavigation(keyName))
        return;
    else if(Button::currentButton())
        Button::currentButton()->command(keyName);
}

/*!
 *  \brief MenuStandard::enterByKey
 *  \par dbKey
 *
 *  Give "ENTER" command of the button mapped in DB by key
 */
bool MenuStandard::buttonEnter(QString dbKey)
{
    for(auto &button : _buttonList)
    {
        if(button->buttonID() == dbKey)
        {
            button->command("ENTER");
            return true;
        }
    }
    return false;
}

/*!
 *  \brief MenuInsertion::loadItems
 *
 *  Load all compatible button under this menu in DB
 */
void MenuStandard::loadButtons()
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

        _buttonList.append(item);
    }

    sortItems();

    for(auto &item : _buttonList)
        _numPadLayout->addWidget(item, item->row(), item->column());
}
