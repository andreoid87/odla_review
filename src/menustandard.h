#ifndef MENUSTANDARD_H
#define MENUSTANDARD_H

#include "menu.h"
#include <QObject>

/*!
 * \brief MenuStandard class
 *
 * Menu class for standard menu
 */
class MenuStandard : public Menu
{
    Q_OBJECT
public:

    explicit MenuStandard(QWidget *parent, QSqlRecord record);
    QString type() override {return "standard";}
    void loadButtons() override;

public slots:
    void navigator(QJsonObject buttonName) override;
    bool buttonEnter(QString dbKey) override;
    void onButtonClicked(Button* button);

};

#endif // MENUSTANDARD_H
