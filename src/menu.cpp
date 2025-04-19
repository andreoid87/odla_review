#include "menu.h"
#include "panel.h"
#include "metadata.h"
#include <QPainter>

#ifdef Q_OS_MACX
const int Menu::_menuTitleFontSize = 20;
#else
const int Menu::_menuTitleFontSize = 12;
#endif

Menu* Menu::_currentMenu = nullptr;

Menu::Menu(QWidget *parent, QSqlRecord record) : QWidget(parent)
{

    _record = record;
    _db = Database::instance();
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Set Menu Title Label
    _titleLabel.setParent(this);
    _titleLabel.setAttribute(Qt::WA_TranslucentBackground);
    _titleLabel.setStyleSheet("color:white;");
    _titleLabel.setFont(_db->getFont(_menuTitleFontSize, QFont::Bold));
    _titleLabel.setWordWrap(true);
    _titleLabel.setAlignment(Qt::AlignCenter);
    _titleLabel.setFixedHeight(60);
    menuTitleUpdate();

    QSizePolicy arrowPolicy = _leftArrow.sizePolicy();
    arrowPolicy.setRetainSizeWhenHidden(true);
    arrowPolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    _leftArrow.setSizePolicy(arrowPolicy);

    //Define page indicator
    _leftArrow.setParent(this);
    QPixmap tmpIcon;
    tmpIcon.loadFromData(_db->getValue("LEFT_ARROW_ICON", "IMAGE").toByteArray());
    _leftArrow.setPixmap(tmpIcon.scaled(ARROW_SIZE,ARROW_SIZE, Qt::KeepAspectRatio));
    _leftArrow.setStyleSheet("background:transparent");
    _leftArrow.setAlignment(Qt::AlignCenter);
    _leftArrow.setFixedWidth(ARROW_SIZE);

    _rightArrow.setParent(this);
    tmpIcon.loadFromData(_db->getValue("RIGHT_ARROW_ICON", "IMAGE").toByteArray());
    _rightArrow.setPixmap(tmpIcon.scaled(ARROW_SIZE,ARROW_SIZE, Qt::KeepAspectRatio));
    _rightArrow.setStyleSheet("background:transparent");
    _rightArrow.setAlignment(Qt::AlignCenter);
    _rightArrow.setSizePolicy(arrowPolicy);
    _rightArrow.setFixedWidth(ARROW_SIZE);

    _mainLayout = new QVBoxLayout(this);
    _numPadAndArrowsLayout = new QHBoxLayout();
    _numPadLayout = new QGridLayout();

    _mainLayout->setContentsMargins(0,0,0,10);

    _mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _numPadAndArrowsLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _numPadLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _numPadLayout->setSpacing(10);

    _header.setParent(this);
    _header.setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    _header.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    QPixmap logo;
    logo.loadFromData(_db->getValue("ODLA_LOGO", "IMAGE").toByteArray());
    _header.setPixmap(logo);
    _header.setStyleSheet("background-color:#d54e3f;border-radius: 20px;");
    _header.setFixedHeight(60);
    _header.setContentsMargins(30,0,0,0);

    _mainLayout->addWidget(&_header);
    _mainLayout->addWidget(&_titleLabel);
    _numPadAndArrowsLayout->addWidget(&_leftArrow);
    _numPadAndArrowsLayout->addLayout(_numPadLayout);
    _numPadAndArrowsLayout->addWidget(&_rightArrow);

    _mainLayout->addLayout(_numPadAndArrowsLayout);
    _mainLayout->insertSpacing(-1,30);
    setLayout(_mainLayout);
    connect(static_cast<Panel*>(parent), &Panel::languageChanged,this, &Menu::menuTitleUpdate);
    _currentPage = -1;
    setMenuVisible(false);
}

/*!
 *  \brief Menu::paintEvent
 *  \param event (unused)
 *
 *  Intercepts paint Event (i.e. before show for first time) in order to draw menu background
 */
void Menu::paintEvent(QPaintEvent *event)
{
    if(hasBackground())
    {
        QPainter painter(this);
        QPixmap pixmap;
        pixmap.loadFromData(_record.value("background").toByteArray());
        painter.drawPixmap((rect().bottomRight() - pixmap.rect().bottomRight()), pixmap);
    }
    else
        QWidget::paintEvent(event);
}

/*!
 *  \brief buttonCompare
 *  \param v1
 *  \param v2
 *
 *  Function used to compare two buttons by their absolute position
 *  in menu, it is used to sort them. (rename as button_compare?)
 */
bool buttonCompare(const Button* v1, const Button* v2)
{
    if(!v1 || !v2) return false;
    return v1->absPos() < v2->absPos();
}

/*!
 *  \brief Menu::sortItems
 *
 *  Sort menu data structure by using comparison function and
 *  change position if gaps or duplicates position in menu ar detected
 */
void Menu::sortItems()
{
    std::sort(_buttonList.begin(), _buttonList.end(), buttonCompare);
    for(int i = 0; i < _buttonList.size(); i++)
        _buttonList[i]->setNewPosition(i);
}

/*!
 *  \brief Menu::buttonAtDeltaPosFromCurrentSelected
 *  \param dx
 *  \param dy
 *
 *  Search and return if exist a button that intersects the area
 *  delimited from of dx pixel horizontally and dy pixel vertically
 *  from the selected one
 */
Button *Menu::buttonAtDeltaPosFromCurrentSelected(int dx, int dy)
{
    if(Button::currentButton() == nullptr) return nullptr;
    QRect currentRect = Button::currentButton()->geometry();
    currentRect.translate(dx,dy);
    for(auto &item : _buttonList)
        if(item->isVisible() && item != Button::currentButton() && currentRect.intersects(item->geometry()))
            return item;
    return nullptr;
}

/*!
 *  \brief Menu::valueItem
 *  \param absPosString (absPos as String)
 *
 *  \return the QString value of the button placed at absPosString
 *  Input and output is QString since it's a slot used by meta-object calling
 */
QString Menu::valueItem(QJsonObject command)
{
    bool ok = command["value"].isBool();
    if(!ok)
        return "";
    int absPos = command["value"].toInt();
    auto item = itemAtAbsPos(absPos);
    if(item == nullptr)
        return "";
    return item->value();
}

/*!
 *  \brief Menu::ordinalValueItem
 *  \param absPosString (absPos as String)
 *
 * \return ordinal number value of item placed at absPos
 *  Input and output is QString since it's a slot used by meta-object calling
 *
 */
QString Menu::ordinalValueItem(QString absPosString)
{
    QJsonObject wrapAbsPos;
    wrapAbsPos["value"] = absPosString;
    return VoiceOver::instance()->cardinalToOrdinal(valueItem(wrapAbsPos));
}

/*!
 *  \brief Menu::itemAtAbsPos
 *  \param absPos
 *
 * \return button at absoulute position in menu if exists, else nullptr
 */
Button *Menu::itemAtAbsPos(int absPos)
{
    for(auto &item : _buttonList)
        if(item->absPos() == absPos)
            return item;
    return nullptr;
}

/*!
 *  \brief Menu::itemAtAbsPos
 *  \param numPadPos
 *
 * \return button at position mapped by numbers written
 * in ODLA keyboard silk-screen printing
 */
Button *Menu::itemAtNumpadPos(int numPadPos)
{
    for(auto &item : _buttonList)
        if(item->relPos() + 1 == numPadPos && item->isVisible())
            return item;
    return nullptr;
}

/*!
 *  \brief Menu::itemByKey
 *  \param key
 *
 * \return button by db key (to be continued.... @giorgio)
 */
Button *Menu::itemByKey(QString key)
{
    for(auto &item : _buttonList)
        if(item->buttonID() == key)
            return item;
    return nullptr;
}

/*!
 *  \brief Menu::menuTitleUpdated
 *
 *  Update labels when language changes and propagate signal to all buttons
 */
void Menu::menuTitleUpdate()
{
    _writtenTitle = loadWrittenTitle();
    _titleLabel.setText(_writtenTitle);
    _speechTitle = loadSpeechTitle();
    emit menuTitleUpdated();
}

/*!
 *  \brief Menu::showPage
 *  \par page
 *  \par visible
 *
 *  Set "page" number to visibile/hidden
 */
void Menu::showPage(int page, bool visible)
{
    foreach(auto item, _buttonList)
        if(item->page() == page)
            item->setVisible(visible);
    if(!visible)
        _currentPage = -1;
}

/*!
 *  \brief Menu::setMenuVisible
 *  \par visible
 *
 *  Set this menu visibile/hidden
 */
void Menu::setMenuVisible(bool visible)
{
    QWidget::setVisible(visible);
    if(visible)
    {
        _currentMenu = this;
        showNewPage(0);
        selectElement(0, true);
    }
    else
    {
        if(Button::currentButton())
            Button::currentButton()->select(false);
        _currentMenu = nullptr;
    }
    Metadata::addCallableInstance(_currentMenu, true);
}

/*!
 *  \brief Menu::showNewPage
 *  \par newPage
 *
 *  Makes every element of page newPage visible
 */
bool Menu::showNewPage(int newPage)
{
    if( newPage < 0 || newPage > elements() || newPage == _currentPage)
        return false;
    // Hide last page
    showPage(_currentPage, false);
    //Show new one
    showPage(newPage, true);
    // Update page button
    _leftArrow.setVisible(newPage == 0 || elements() == 0 ? false : true);
    _rightArrow.setVisible(newPage >= pages() || elements() == 0 ? false: true);
    _currentPage = newPage;
    return true;
}

/*!
 *  \brief Menu::selectElement
 *  \par absPosString
 *
 *  Overload function: select element as absolute position as string
 */
void Menu::selectElement(QString absPosString)
{
    bool ok = false;
    int absPos = absPosString.toInt(&ok);
    selectElement(absPos);
}

/*!
 *  \brief Menu::selectElement
 *  \par item
 *  \par mute
 *
 *  Select button if exists,
 *  if mute is true it avoids vocal guide to read the title
 */
void Menu::selectElement(Button * button, bool mute)
{
    if(button == nullptr || button == Button::currentButton()) return;
    // Now that we have sure an element Redraw page if needed
    showNewPage(button->page());
    // unselect previous selected
    if(Button::currentButton())
        Button::currentButton()->select(false);
    // and select the new
    button->select(true);
    return;
}

/*!
 *  \brief Menu::selectElement
 *  \par absPosString
 *
 *  Overload function: select button at abslute position if exists,
 *  if mute is true it avoids vocal guide to read the title
 */
void Menu::selectElement(int absPos, bool mute)
{
    auto newButton = itemAtAbsPos(absPos);
    // if don't found element exit
    if(!newButton) return;
    selectElement(newButton, mute);
}

/*!
 *  \brief Menu::standardNavigation
 *  \par key
 *
 *  It maps key pressed name string and navigation method
 */
bool Menu::standardNavigation(QString key)
{
    if(key == "LEFT")
        selectLeftItem();
    else if(key == "RIGHT")
        selectRightItem();
    else if(key == "UP")
        selectUpperItem();
    else if(key == "DOWN")
        selectLowerItem();
    else if(key == "FNLEFT")
        showPrevPage();
    else if(key == "FNRIGHT")
        showNextPage();
    else if(key == "FNUP")
        selectFirstItem();
    else if(key == "FNDOWN")
        selectLastItem();
    else if(key == "UNDO")
        swapPrevItem();
    else if(key == "REDO")
        swapNextItem();
    else
        return false;
    return true;
}

/*!
 *  \brief Menu::swapItems
 *  \par pos1
 *  \par pos2
 *
 *  It swaps two buttons placed ad absolute position pos1 and pos2
 */
bool Menu::swapItems(int pos1, int pos2)
{
    auto item1 = itemAtAbsPos(pos1);
    auto item2 = itemAtAbsPos(pos2);
    if(!item1 || !item2) return false;
    item1->setNewPosition(pos2);
    item2->setNewPosition(pos1);

    _numPadLayout->removeWidget(item1);
    _numPadLayout->removeWidget(item2);

    _numPadLayout->addWidget(item1, item1->row(), item1->column());
    _numPadLayout->addWidget(item2, item2->row(), item2->column());
    selectElement(item1);
    QString message = _db->speechText("ON_SWAPPED_BUTTON");
    VoiceOver::instance()->say(message.arg(item1->speechTitle()).arg(item2->speechTitle()));
    return true;
}

/*!
 *  \brief Menu::selectFirstItem
 */
void Menu::selectFirstItem()
{
    selectElement(0);
}

/*!
 *  \brief Menu::selectLowerItem
 */
void Menu::selectLowerItem()
{
    if(Button::currentButton() == nullptr)
        return;
    auto item = buttonAtDeltaPosFromCurrentSelected(0,60);
    if(item)
        selectElement(item);
    else
        selectElement(Button::currentButton()->absPos() + 3);
}

/*!
 *  \brief Menu::selectUpperItem
 */
void Menu::selectUpperItem()
{
    if(Button::currentButton() == nullptr) return;
    auto item = buttonAtDeltaPosFromCurrentSelected(0,-60);
    if(item)
        selectElement(item);
    else
        selectElement(Button::currentButton()->absPos() - 3);
}

/*!
 *  \brief Menu::selectRightItem
 */
void Menu::selectRightItem()
{
    if(Button::currentButton() == nullptr) return;
    auto item = buttonAtDeltaPosFromCurrentSelected(60,0);
    if(item)
        selectElement(item);
    else
        selectElement(Button::currentButton()->absPos() + 1);
}

/*!
 *  \brief Menu::selectLeftItem
 */
void Menu::selectLeftItem()
{
    if(Button::currentButton() == nullptr) return;
    auto item = buttonAtDeltaPosFromCurrentSelected(-60,0);
    if(item)
        selectElement(item);
    else
        selectElement(Button::currentButton()->absPos() - 1);
}

/*!
 *  \brief Menu::selectLastItem
 */
void Menu::selectLastItem()
{
    selectElement(elements());
}

/*!
 *  \brief Menu::showNextPage
 */
void Menu::showNextPage()
{
    if(Button::currentButton() == nullptr) return;
    int newPos = Button::currentButton()->absPos();
    newPos += 9;
    while((newPos % 9) != 0)
        newPos--;
    selectElement(newPos);
}

/*!
 *  \brief Menu::showPrevPage
 */
void Menu::showPrevPage()
{
    if(Button::currentButton() == nullptr) return;
    int newPos = Button::currentButton()->absPos();
    newPos -= 9;
    while((newPos % 9) != 0)
        newPos++;
    selectElement(newPos);
}

/*!
 *  \brief Menu::swapNextItem
 *
 *  Swap current intem with next one
 */
void Menu::swapNextItem()
{
    if(Button::currentButton() == nullptr) return;
    int currentPos = Button::currentButton()->absPos();
    swapItems(currentPos, currentPos + 1);
    showNewPage(Button::currentButton()->page());
}

/*!
 *  \brief Menu::swapPrevItem
 *
 *  Swap current intem with previous one
 */
void Menu::swapPrevItem()
{
    if(Button::currentButton() == nullptr) return;
    int currentPos = Button::currentButton()->absPos();
    swapItems(currentPos, currentPos - 1);
    showNewPage(Button::currentButton()->page());
}
