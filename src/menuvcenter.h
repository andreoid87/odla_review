#ifndef MENUVCENTER_H
#define MENUVCENTER_H

#include "menu.h"
#include <QObject>

/*!
 * \brief MenuVCenter class
 *
 * Menu class that loads special buttons vertical centered such as accidental or +/-
 */
class MenuVCenter : public Menu
{
    Q_OBJECT
public:
    MenuVCenter(QWidget *parent, QSqlRecord record);
    QString type() override {return "V_CENTER";}
    void loadButtons() override;

public slots:
    void navigator(QJsonObject buttonName) override;
    bool buttonEnter(QString buttonID) override {Q_UNUSED(buttonID);return false;}

};

#endif // MENUVCENTER_H
