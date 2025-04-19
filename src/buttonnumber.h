#ifndef BUTTONNUMBER_H
#define BUTTONNUMBER_H

#include "button.h"

/*!
 *  \brief ButtonNumber class
 *
 *  Subclass for menu buttons that contains an editable number
 *  used to set some numeric option
 *
 *  \warning it is more generic thab number since method is defined outside (i.e. call it not only number ...)
 */
class ButtonNumber : public Button
{
    Q_OBJECT

public:
    ButtonNumber(QWidget *parent, QSqlRecord record);
    QString table() const  override {return "BUTTON_NUM";}

public slots:
    void updateButton()             override;
    void updateLanguageStrings()    override;
    void sayTitleAndValue()         override;
    QVariant loadValue()         override {return QVariant();} // FORSE POTREMMO USARE QUESTA?
    QString writtenTitle()          override {return _writtenTitleBase;}
    QString speechTitle()           override {return _speechTitleBase;}

private:
    double numValue() {return _record.value("numValue").toDouble();}

};

#endif // BUTTONNUMBER_H
