#ifndef BUTTONINSERTION_H
#define BUTTONINSERTION_H

#include "button.h"

/*!
 *  \brief ButtonInsertion class
 *
 *  Subclass with editable number title button used to send some
 *  commands with variable number
 */
class ButtonInsertion : public Button
{
    Q_OBJECT

public:
    ButtonInsertion(QWidget *parent, QSqlRecord record);

public slots:
    void appendDigit(QString digit); //Append digit to label and update value for command to be sent
    void clearTitle();
    void updateButton()             override {}
    void sayTitleAndValue()         override {}
    QVariant loadValue()         override {return QVariant();} // FORSE POTREMMO USARE QUESTA?
    void updateLanguageStrings()    override;
    QString writtenTitle()          override {return _writtenTitleBase;}
    QString speechTitle()           override {return _speechTitleBase;}

private:
    static const QString _defaultStylesheet;
    static const int _bigTitleFontSize;
    static const int _numberFontSize;
    QVBoxLayout _layout;

    void showEvent(QShowEvent *e)   override;
};

#endif // BUTTONINSERTION_H
