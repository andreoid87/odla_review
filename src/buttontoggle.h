#ifndef BUTTONTOGGLE_H
#define BUTTONTOGGLE_H

#include "button.h"

/*!
 *  \brief ButtonToggle class
 *
 *  Subclass for menu buttons that contains an ON-OFF switch not mutex
 *  used to set some boolean option
 *
 */
class ButtonToggle : public Button
{
    Q_OBJECT

public:
    ButtonToggle(QWidget *parent, QSqlRecord record);
    QString table() const           override {return "BUTTON_TOGGLE";}

public slots:
    void updateButton()             override;
    void updateLanguageStrings()    override;
    void sayTitleAndValue()         override;
    QVariant loadValue()         override {return _db->getValue(buttonID(), "BOOLEAN_VALUE");}
    QString writtenTitle()          override;
    QString speechTitle()           override;

private:

};

#endif // BUTTONTOGGLE_H
