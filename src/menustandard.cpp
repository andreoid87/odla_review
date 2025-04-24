#include "menustandard.h"
#include "src/buttonnumber.h"
#include "src/buttontoggle.h"
#include "src/buttoncommand.h"

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
           item->command("enter");
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
 *  Give "enter" command of the button mapped in DB by key
 */
bool MenuStandard::buttonEnter(QString dbKey)
{
    for(auto &button : _buttonList)
    {
        if(button->buttonID() == dbKey)
        {
            button->command("enter");
            return true;
        }
    }
    return false;
}

void MenuStandard::onButtonClicked(Button *button)
{
    Q_UNUSED(button);
    for(Button* item : _buttonList)
        item->updateButton();
}

/*!
 *  \brief MenuInsertion::loadItems
 *
 *  Load all compatible button under this menu in DB
 */
void MenuStandard::loadButtons()
{
    for(auto &record : _db->getMenuButtons(menuID()))
    {
        //qDebug() << "found button" << record.value("button_id").toString();
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

        _buttonList.append(item);
    }

    sortItems();

    for(auto &item : _buttonList)
        _numPadLayout->addWidget(item, item->row(), item->column());
}
