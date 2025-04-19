#include "menutoggleex.h"

MenuToggleEx::MenuToggleEx(QWidget *parent, QSqlRecord record) : Menu(parent, record)
{
    
}

/*!
 *  \brief MenuToggleEx::command
 *
 *  Invoke method mapped to buttonName
 */
void MenuToggleEx::navigator(QJsonObject command)
{
    bool isNumber = command["value"].isDouble();
    int numpadPos = command["value"].toInt();
    QString keyName = command["value"].toString();

    if(standardNavigation(keyName))
        return;
    else if(isNumber)
    {
        auto item = itemAtNumpadPos(numpadPos);
        if(item)
        {
            selectElement(item, true);
            setCurrentEnabled(!item->isChecked());
        }
    }
    else if(keyName == "ENTER")
        setCurrentEnabled(true);
    else if(keyName == "PLUS")
        setCurrentEnabled(true);
    else if(keyName == "MINUS")
        setCurrentEnabled(false);
    else if(Button::currentButton())
        Button::currentButton()->command(keyName);
}

/*!
 *  \brief MenuToggleEx::enterByKey
 *  \par dbKey
 *
 *  Give "ENTER" command of the button mapped in DB by key
 */
bool MenuToggleEx::buttonEnter(QString buttonID)
{
    bool found = false;

    for(auto &button : _buttonList)
        if(button->buttonID() == buttonID)
        {
            button->command("ENTER");
            found = true;
        }

    if(found)
        for(auto &button : _buttonList)
            button->updateButton();

    return found;
}

/*!
 *  \brief MenuToggleEx::setCurrentEnabled
 *  \par dbKey
 *
 *  Reload other mutex buttons to swith off all except this
 */
void MenuToggleEx::setCurrentEnabled(bool enabled)
{
    if(!Button::currentButton())
        return;
    if(enabled)
        Button::currentButton()->command("ENTER");
    updateButtons();
}

void MenuToggleEx::updateButtons()
{
    for(auto &button : _buttonList)
        button->updateButton();
}


/*!
 *  \brief MenuToggleEx::loadItems
 *
 *  Load all compatible button under this menu in DB
 */
void MenuToggleEx::loadButtons()
{
    QString filters = QString("parentMenu='%1'").arg(menuID());
    filters += QString(" AND type='TOGGLE_EX'");
    auto buttonListRecord = _db->allTableRecords("button", filters);

    for(auto &record : buttonListRecord)
        if(!record.value(_db->currentSoftware() + "_position").isNull())
            _buttonList.append(new ButtonToggleEx(this, record));

    sortItems();

    for(auto &item : _buttonList)
        _numPadLayout->addWidget(item, item->row(), item->column());
}
