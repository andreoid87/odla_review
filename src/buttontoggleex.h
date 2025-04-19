#ifndef BUTTONTOGGLEEX_H
#define BUTTONTOGGLEEX_H

#include "button.h"

/*!
 *  \brief ButtonToggleEx class
 *
 *  Subclass for menu buttons that contains an ON-OFF switch
 *  with mutex logic
 *
 *  i.e. if this button is on, other buttons of same menu is off
 *
 *  \warning the mutex logic is not regulated by this object but by MenuToggleEx this
 *  this object change from ButtonToggle only for status saving
 *
 *  \todo join with ButtonToggle?
 */
class ButtonToggleEx : public Button
{
    Q_OBJECT

public:
    ButtonToggleEx(QWidget *parent, QSqlRecord record);
    QString table() const           override {return "BUTTON_TOGGLE_EX";}

public slots:
    void updateButton()             override;
    void updateLanguageStrings()    override;
    void sayTitleAndValue()         override {}
    QVariant loadValue()         override {return QVariant();}
    QString writtenTitle()              override {return _writtenTitleBase;}
    QString speechTitle()               override {return _speechTitleBase;}

};

#endif // BUTTONTOGGLEEX_H
