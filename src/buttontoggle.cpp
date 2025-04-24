#include "buttontoggle.h"
#include "database.h"
#include "voiceover.h"
#include "menustandard.h"

ButtonToggle::ButtonToggle(QWidget *parent, QSqlRecord record) : Button(parent, record)
{
    
    setCheckable(true);
    // Set the Label with number in top left corner
    _backgroundLabel= new QLabel(this);
    _backgroundLabel->setGeometry(5,5,85,85);
    _backgroundLabel->setAttribute(Qt::WA_TranslucentBackground);
    _backgroundLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    _backgroundLabel->lower();
    setFixedSize(BUTTON_SIDE,BUTTON_SIDE);
    _titleLabel.setFixedSize(BUTTON_SIDE,BUTTON_SIDE);
    _titleLabel.setAlignment(Qt::AlignBottom| Qt::AlignHCenter);
    QTimer::singleShot(0, this, &ButtonToggle::updateButton);
    QTimer::singleShot(0, this, &ButtonToggle::updateLanguageStrings);
    connect(this, &ButtonToggle::buttonClicked, dynamic_cast<MenuStandard*>(parent), &MenuStandard::onButtonClicked);
}

/*!
 *  \brief ButtonToggle::updateButton
 *
 *  Update button (also graphically) status and save it in DB
 */
void ButtonToggle::updateButton()
{
    bool value = loadValue().toBool();
    setChecked(value);
    QPixmap switchIcon;

    QString iconName = QString("toggle_icon_") + (value ? "on" : "off");
    auto bitArray = Database::instance()->getSetting(iconName, "image").toByteArray();
    switchIcon.loadFromData(bitArray);
    _backgroundLabel->setPixmap(switchIcon);
    _speechTitleBase = loadSpeechTitle();
    QToolButton::update();
}

/*!
 *  \brief ButtonToggle::updateLanguageStrings
 *
 *  Method to be called whenever language is changed in order to replace translating names
 */
void ButtonToggle::updateLanguageStrings()
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

/*!
 *  \brief ButtonToggle::sayTitleAndValue
 *
 *  Tell vocal guide to say speechTitle
 */
void ButtonToggle::sayTitleAndValue()
{
    VoiceOver::instance()->say(_speechTitleBase);
}

QVariant ButtonToggle::loadValue()
{
    return _db->getButtonState(buttonID());
}

QString ButtonToggle::writtenTitle()
{
    return _writtenTitleBase;
}

QString ButtonToggle::speechTitle()
{
    return _db->speechText(loadValue().toBool() ? "on_enabled" : "on_disabled").arg(_speechTitleBase);
}

void ButtonToggle::command(QString key)
{
    if(key=="enter")
    {
        _db->setActiveToggleButtons(buttonID(), !isChecked());
        emit buttonClicked(this);
    }
    Button::command(key);
}
