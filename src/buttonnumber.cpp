#include "buttonnumber.h"
#include "database.h"
#include "voiceover.h"
#include <QDebug>

ButtonNumber::ButtonNumber(QWidget *parent, QSqlRecord record) : Button(parent, record)
{
    
    setFixedSize(BUTTON_SIDE,BUTTON_SIDE);
    _titleLabel.setFixedSize(BUTTON_SIDE,BUTTON_SIDE);
    _titleLabel.setAlignment(Qt::AlignBottom| Qt::AlignHCenter);
    QTimer::singleShot(0, this, &ButtonNumber::updateLanguageStrings);
    QTimer::singleShot(0, this, &ButtonNumber::updateButton);
}

/*!
 *  \brief ButtonNumber::updateButton
 *
 *  Update button (also graphically) status and save it in DB
 */
void ButtonNumber::updateButton()
{
    _value = Database::instance()->getSetting(buttonID(), "numeric_value").toString();
    _writtenTitleBase = loadWrittenTitle() + QString("\n-  %1  +").arg(_value);
    _titleLabel.setText(_writtenTitleBase);
    _speechTitleBase = loadSpeechTitle() + ": " + _value;
    QToolButton::update();
}

/*!
 *  \brief ButtonNumber::sayTitleAndValue
 *
 *  Tell vocal guide to say speechTitle
 */
void ButtonNumber::sayTitleAndValue()
{
    VoiceOver::instance()->say(_speechTitleBase);
}

/*!
 *  \brief ButtonNumber::updateLanguageStrings
 *
 *  Method to be called whenever language is changed in order to replace translating names
 */
void ButtonNumber::updateLanguageStrings()
{
    if(hasIcon())
        setButtonIcon();
    else
    {
        _writtenTitleBase = loadWrittenTitle() + QString("\n-  %1  +").arg(_value);
        _titleLabel.setText(_writtenTitleBase);
    }
    _speechTitleBase = loadSpeechTitle() + QString(": %1").arg(_value);
}
