#include "buttoncommand.h"

ButtonCommand::ButtonCommand(QWidget *parent, QSqlRecord record) : Button(parent, record)
{
    if(isDisclaimer()) //! \warning create subclass for this case only?
    {
        _numberLabel.setVisible(false);
        setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        auto layout = new QVBoxLayout(this);
        layout->addWidget(&_titleLabel);
        _titleLabel.setAlignment(Qt::AlignCenter);
        _titleLabel.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    }
    else
    {
        setFixedSize(BUTTON_SIDE,BUTTON_SIDE);
        _titleLabel.setFixedSize(BUTTON_SIDE,BUTTON_SIDE);
        _titleLabel.setAlignment(Qt::AlignBottom| Qt::AlignHCenter);
    }
    QTimer::singleShot(0, this, &ButtonCommand::updateLanguageStrings);
}

void ButtonCommand::setVisible(bool val)
{
    if(isDisclaimer() && isVisible())
        val = false;
    Button::setVisible(val);
}

/*!
 *  \brief ButtonCommand::updateLanguageStrings
 *
 *  Method to be called whenever language is changed in order to replace translating names
 */
void ButtonCommand::updateLanguageStrings()
{
    if(hasIcon())
        setButtonIcon();
    else
    {
        _writtenTitleBase = loadWrittenTitle();
        _titleLabel.setText(_writtenTitleBase);
    }
    _speechTitleBase = loadSpeechTitle();
}
