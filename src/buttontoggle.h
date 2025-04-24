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

public slots:
    void updateButton()             override;
    void updateLanguageStrings()    override;
    void sayTitleAndValue()         override;
    QVariant loadValue()            override;
    QString writtenTitle()          override;
    QString speechTitle()           override;
    void command(QString key)       override;

};

#endif // BUTTONTOGGLE_H
