#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include <QtSql>
#include <QLabel>
#include <QVBoxLayout>
#include "database.h"
#include "button.h"

/*!
 * \brief Menu class
 *
 * Virtual base class for menu
 */
class Menu : public QWidget
{
    Q_OBJECT
public:
    explicit Menu(QWidget *parent, QSqlRecord record);
    QString menuID() {return _record.value("menu_id").toString();}
    QString titleID() {return _record.value("title_id").toString();}
    static Menu * currentMenu() {return _currentMenu;}
    int pages() {return (_buttonList.size() - 1) / 9;}
    int elements() {return _buttonList.size() - 1;}
    int currentPage() {return _currentPage;}
    const QList<Button*> buttonList() {return _buttonList;}
    void setMenuVisible(bool visible);
    virtual QString type() = 0;
    virtual void loadButtons() = 0;

protected:
    int _currentPage;
    static Menu * _currentMenu;
    static const int _menuTitleFontSize;
    QSqlRecord _record;
    QLabel _titleLabel;
    QString _writtenTitle;
    QString _speechTitle;
    QLabel _header;
    QList<Button*> _buttonList;
    Database* _db;

    void showPage(int page, bool visible);
    bool swapItems(int item1, int item2);
    void selectElement(Button *item, bool mute = false);
    void selectElement(int absPos, bool mute = false);
    bool standardNavigation(QString key);
    bool showNewPage(int newPage);
    void sortItems();
    bool hasBackground() {return !_record.value("background").isNull();}
    void paintEvent(QPaintEvent *event) override;
    int rel2AbsPos(int relPos)      {return _currentPage * 9 + relPos;}
    Button *itemAtAbsPos(int absPos);
    Button *itemAtNumpadPos(int numPadPos);
    Button *itemByKey(QString key);
    Button *buttonAtDeltaPosFromCurrentSelected(int dx, int dy);
    QString loadWrittenTitle(){return _db->writtenText(titleID()).toUpper();}
    QString loadSpeechTitle() {return _db->speechText(titleID());}

    QVBoxLayout *_mainLayout;
    QHBoxLayout *_numPadAndArrowsLayout;
    QGridLayout *_numPadLayout;
    QLabel _leftArrow, _rightArrow;
    QString replaceKey(QString key);

public slots:
    /*!
     *  \brief loadTitle
     *  \warning make same of Button!
     */
    virtual void navigator(QJsonObject command) = 0;
    virtual bool buttonEnter(QString buttonID) = 0;
    QString valueItem(QJsonObject command);
    QString ordinalValueItem(QString absPosString);
    QString writtenTitle()      {return _writtenTitle;}
    QString speechTitle()       {return _speechTitle;}
    void selectElement(QString absPosString);

    void selectFirstItem();
    void selectLowerItem();
    void selectUpperItem();
    void selectRightItem();
    void selectLeftItem();
    void selectLastItem();
    void showNextPage();
    void showPrevPage();
    void swapNextItem();
    void swapPrevItem();
    virtual QString buttonValue(QJsonObject buttonNumberWrapper) {Q_UNUSED(buttonNumberWrapper); return "";};

protected slots:
    void menuTitleUpdate();

signals:
    void menuTitleUpdated();
};

#endif // MENU_H
