#ifndef BUTTONCOMMAND_H
#define BUTTONCOMMAND_H

#include "button.h"

/*!
 *  \brief ButtonCommand class
 *
 *  Subclass for menu buttons that simply sends some command
 *  like open a submenu or send something to notation software
 *  It can be used for every method dispached trough MOC engine
 *  (see Button:invokeVoid method)
 */
class ButtonCommand : public Button
{
    Q_OBJECT

public:
    ButtonCommand(QWidget *parent, QSqlRecord record);
    QString table() const               override {return "BUTTON_COMMAND";}
    void setVisible(bool val) override;

public slots:
    void updateButton()                 override {}
    void sayTitleAndValue()             override {}
    QVariant loadValue()             override {return QVariant();}
    void updateLanguageStrings()        override; //translation handler
    QString writtenTitle()              override {return _writtenTitleBase;}
    QString speechTitle()               override {return _speechTitleBase;}
};

#endif // BUTTONCOMMAND_H
