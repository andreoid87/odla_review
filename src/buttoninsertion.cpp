#include "buttoninsertion.h"

#ifdef Q_OS_MACX
const int ButtonInsertion::_bigTitleFontSize = 64;
#else
const int ButtonInsertion::_bigTitleFontSize = 40;
#endif

ButtonInsertion::ButtonInsertion(QWidget *parent, QSqlRecord record) : Button(parent, record)
{
    
    _titleLabel.setFont(Database::instance()->getFont(_bigTitleFontSize, QFont::Bold));
    _numberLabel.setVisible(false);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setLayout(&_layout);
    _layout.addWidget(&_titleLabel);
    _titleLabel.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    _titleLabel.setAlignment(Qt::AlignCenter);
    QTimer::singleShot(0, this, &ButtonInsertion::updateLanguageStrings);
}

/*!
 *  \brief ButtonInsertion::appendDigit
 *  \par digit
 *
 *  Append digit to label and update value for command to be sent
 */
void ButtonInsertion::appendDigit(QString digit)
{
    bool ok = false;
    digit.toInt(&ok);
    if(ok)
    {
        _writtenTitleBase =_value += digit;
        if(!_value.isEmpty())
        _speechTitleBase = loadSpeechTitle() + _value;
        _titleLabel.setText(writtenTitle());
        //VoiceOver::instance()->say(speechTitle());
    }
}

/*!
 *  \brief ButtonInsertion::clearTitle
 *  \par digit
 *
 *  Clear label and value for command to be sent
 */
void ButtonInsertion::clearTitle()
{
    _writtenTitleBase = _value = "";
    _speechTitleBase = "";
    _titleLabel.setText("");
}

/*!
 *  \brief ButtonInsertion::updateLanguageStrings
 *
 *  Method to be called whenever language is changed in order to replace translating names
 */
void ButtonInsertion::updateLanguageStrings()
{
    _writtenTitleBase = _value = "";
    _titleLabel.setText(_writtenTitleBase);
    if(!_value.isEmpty())
        _speechTitleBase = loadSpeechTitle() + _value;
    else
        _speechTitleBase = "";
}

/*!
 *  \brief ButtonInsertion::showEvent
 *  \par visible
 *
 *  Intercepts showevent, since every time will be shown this button
 *  the old value will bel deleted
 */
void ButtonInsertion::showEvent(QShowEvent *e)
{
    clearTitle();
    QWidget::showEvent(e);
}

