#ifndef MENUTOGGLEEX_H
#define MENUTOGGLEEX_H

#include "menu.h"
#include <QObject>

/*!
 * \brief MenuToggleEx class
 *
 * Menu class that loads only button toggle mutex
 */
class MenuToggleEx : public Menu
{
    Q_OBJECT
public:
    explicit MenuToggleEx(QWidget *parent, QSqlRecord record);
    QString type() override {return "TOGGLE_EX";}
    void loadButtons() override;
    void updateButtons();

public slots:
    void navigator(QJsonObject buttonName) override;
    bool buttonEnter(QString buttonID) override;

protected:
    void selectRelPosItem(int relPos);
    void setCurrentEnabled(bool enabled);
};

#endif // MENUTOGGLEEX_H
