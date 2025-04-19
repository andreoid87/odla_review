#include "button.h"
#include "menu.h"
#include "database.h"
#include "metadata.h"
#include "qmenu.h"
#include <QApplication>
#include <QtDebug>
#include <QLabel>


const QString Button::_defaultStylesheet = QString( "Button{border-radius: 15px;background-color: #2f2f2f;}"
                                                        "Button:hover{background-color: #d54e3f;}");
Button* Button::_selectedButton = nullptr;

#ifdef Q_OS_MACX
const int Button::_titleFontSize = 12;
const int Button::_numberFontSize = 24;
#else
const int Button::_titleFontSize = 7;
const int Button::_numberFontSize = 14;
#endif

Button::Button(QWidget *parent, QSqlRecord record) : QToolButton(parent)
{
    
    _record = record;
    _db = Database::instance(nullptr);
    if(hasSpecialStylesheet())
        setStyleSheet(_record.value("specialStylesheet").toString());
    else
        setStyleSheet(_defaultStylesheet);

    // Set the Label with number in top left corner
    _numberLabel.setParent(this);
    _numberLabel.setText(QString::number(relPos() + 1));
    _numberLabel.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    _numberLabel.setFont(_db->getFont(_numberFontSize, QFont::Bold));
    _numberLabel.setStyleSheet("background:transparent; color:white; padding:0px; margin:4px; border:none;");

    // Set the Label with title at bottom center
    _titleLabel.setParent(this);
    _titleLabel.setWordWrap(true);
    _titleLabel.setFont(_db->getFont(_titleFontSize, QFont::Bold));
    _titleLabel.setStyleSheet("background:transparent; color:white; padding:gray; margin:0px; border:none;");
    _titleLabel.setMargin(5);

    setVisible(false);
    select(false);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    connect(static_cast<Menu*>(parent), &Menu::menuTitleUpdated,this, &Button::updateLanguageStrings);

    // This commands prevent unwanted moving of buttons menu when page changes
    QSizePolicy fixSize = sizePolicy();
    fixSize.setRetainSizeWhenHidden(true);
    setSizePolicy(fixSize);
}

/*!
 *  \brief Button::select
 *
 *  Set button selected changing its state to "hovered"
 *  i.e. emulate mouse hover on it
 */
void Button::select(bool selection)
{
    QApplication::postEvent(this, new QEvent(selection ? QEvent::Enter : QEvent::Leave));
    _selectedButton  = selection ? this : nullptr;
    Metadata::addCallableInstance(_selectedButton, true);
    QToolButton::update();
}


/*!
 *  \brief Button::setNewPosition
 *  \par newPos
 *
 *  Change the reference of position of the button in the menu
 */
void Button::setNewPosition(int newPos)
{
    if(newPos == absPos())  return;
    _db->setButtonPosition(buttonID(), newPos);
    _numberLabel.setText(QString::number(relPos() + 1));
    QToolButton::update();
}

/*!
 *  \brief Button::setButtonIcon
 *
 *  Load pixmap from database and show it in _titlelabel
 */
void Button::setButtonIcon()
{
    _pixmap.loadFromData(_record.value("icon").toByteArray());
    _titleLabel.setPixmap(_pixmap.scaled(_titleLabel.size() * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    _titleLabel.setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    _titleLabel.setContentsMargins(0,0,0,0);
    _titleLabel.setMargin(0);
}

/*!
 *  \brief Button::command
 *  \par column
 *
 *  Execute (through MOC meta-call) the method written in the db
 *  under a column named like ODLA key name
 *
 *  i.e. if "ENTER" is pressed it search the method name written under "ENTER" column
 *  and invoke it after eventually resolve some reference
 */
void Button::command(QString column)
{
    bool isNumber = false;
    column.toInt(&isNumber);
    QString commandID = _record.value(column).toString();
    Metadata::invokeVoid(commandID, true); // if called by button assume that command is always allowed
    updateButton();
}

QString Button::loadWrittenTitle()
{
    return Metadata::resolveString(_db->writtenText(titleID()).toUpper());
}

QString Button::loadSpeechTitle()
{
    return Metadata::resolveString(_db->speechText(titleID()));
}
