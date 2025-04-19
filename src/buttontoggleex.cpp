#include "buttontoggleex.h"
#include "database.h"

ButtonToggleEx::ButtonToggleEx(QWidget *parent, QSqlRecord record) : Button(parent, record)
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
    QTimer::singleShot(0, this, &ButtonToggleEx::updateButton);
    QTimer::singleShot(0, this, &ButtonToggleEx::updateLanguageStrings);
}

/*!
 *  \brief ButtonToggleEx::updateButton
 *
 *  Update button (also graphically) status and save it in DB
 */
void ButtonToggleEx::updateButton()
{

    QString currentValue = _db->getValue(_record.value("parentMenu").toString(), "MULTI_CHOICE_VALUE").toString();
    //This is too ugly, change ASAP with button ID logic
    QString thisButtonValue = _db->getCommand(_record.value("ENTER").toString()).value("value").toString();
    bool value = (currentValue == thisButtonValue);
    setChecked(value);
    QPixmap switchIcon;
    QString iconName = QString("TOGGLE_ICON_") + (value ? "ON" : "OFF");
    auto bitArray = Database::instance()->getValue(iconName, "IMAGE").toByteArray();
    switchIcon.loadFromData(bitArray);
    _backgroundLabel->setPixmap(switchIcon);
//    if(value)
//        VoiceOver::instance()->say(DBText("SET_VALUE").arg(speechTitle()));
    QToolButton::update();
}

/*!
 *  \brief ButtonToggleEx::updateLanguageStrings
 *
 *  Method to be called whenever language is changed in order to replace translating names
 */
void ButtonToggleEx::updateLanguageStrings()
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
