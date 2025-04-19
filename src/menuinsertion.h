#ifndef MENUINSERTION_H
#define MENUINSERTION_H

#include "menu.h"
#include <QObject>

/*!
 * \brief MenuInsertion class
 *
 * Menu class containing only button insertion
 */
class MenuInsertion : public Menu
{
    Q_OBJECT
public:
    MenuInsertion(QWidget *parent, QSqlRecord record);
    QString type() override {return "INSERTION";}
    void loadButtons() override;

public slots:
    void navigator(QJsonObject buttonName) override;
    bool buttonEnter(QString buttonID) override {Q_UNUSED(buttonID); return false;}
    QString buttonValue(QJsonObject buttonNumberWrapper) override;

};

#endif // MENUINSERTION_H
