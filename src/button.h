#ifndef BUTTON_H
#define BUTTON_H

#include <QToolButton>
#include <QLabel>
#include <QtSql>
#include <QVBoxLayout>
#include "database.h"

#define BUTTON_MARGIN 20
#define BUTTON_SIDE 90
#define HEADER_HEIGHT 60
#define MENU_TITLE_HEIGHT 60
#define ARROW_MARGIN 7
#define ARROW_SIZE 20
#define NUMPAD_Y_OFFSET 70
#define NUMPAD_X_OFFSET 30
#define WINDOW_WIDTH 400
#define WINDOW_HEIGTH 500
#define BORDER_RADIUS 20

/*!
 * \brief Button class
 *
 * Virtual base class for menu buttons
 */
class Button : public QToolButton
{
    Q_OBJECT

public:
    Button(QWidget *parent, QSqlRecord record);
    void select(bool selection); //set button state to "hovered"
    void setNewPosition(int newPos); //Change button position in the menu
    int absPos() const  {return  _db->getButtonPosition(buttonID());}
    int relPos() const {return absPos() % 9;}
    int page() const {return absPos() / 9;}
    int row() const {return relPos() / 3;}
    int column() const {return relPos() % 3;}
    QString buttonID() const {return  _record.value("button_id").toString();}
    QString titleID() const {return  _record.value("title_id").toString();}
    void hideNumber() {_numberLabel.hide();}
    virtual QString table() const = 0;
    bool isDisclaimer()             {return buttonID().contains("DISCLAIMER");}

    //! \warning move these methods to a global environment (or not?)
    static Button * currentButton() {return _selectedButton;}

public slots:
    void command(QString column);
    QString posString() {return QString::number(absPos() + 1);}
    virtual QString writtenTitle() = 0;
    virtual QString speechTitle() = 0;
    virtual void updateButton() = 0;
    virtual void sayTitleAndValue() = 0;
    virtual QVariant loadValue() = 0;
    QString value() {return _value;}

protected:
    QLabel _titleLabel;
    QLabel _numberLabel;
    QString _writtenTitleBase;
    QString _speechTitleBase;
    QString _value;
    QPixmap _pixmap;
    Database *_db;

    static const QString _defaultStylesheet;
    static const int _titleFontSize;
    static const int _numberFontSize;
    static Button * _selectedButton;
    QLabel* _backgroundLabel;
    QSqlRecord _record;
    QString loadWrittenTitle();
    QString loadSpeechTitle();

    bool hasIcon()                  {return ! _record.value("icon").isNull();}
    bool hasSpecialStylesheet()     {return ! _record.value("specialStylesheet").isNull();}
    void setButtonIcon(); //Load pixmap from database and show it in _titlelabel

protected slots:
    virtual void updateLanguageStrings() = 0;
};

#endif // BUTTON_H
