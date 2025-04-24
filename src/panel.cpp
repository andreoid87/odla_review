#include <QStyle>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>
#include "panel.h"
#include "database.h"
#include "src/menuinsertion.h"
#include "src/menustandard.h"
#include "src/menuvcenter.h"
#include "voiceover.h"
#include "metadata.h"

extern bool isDebug;
Panel* Panel::_instance;

Panel *Panel::instance(QWidget *parent)
{
    if(!_instance)
        _instance = new Panel(parent);
    Metadata::addCallableInstance(_instance);
    return _instance;
}

Panel::Panel(QWidget * parent) : QDialog(parent)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    _mainWidget = new QStackedWidget(this);
    setAttribute(Qt::WA_TranslucentBackground);

    _mainWidget->setStyleSheet("background: #242424;border-radius: 20px;");

    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_mainWidget);

    _db = Database::instance(this);

    _voiceOver = VoiceOver::instance(this);
    _keyBoard = Keyboard::instance(this);

    _keyboardConnected = false;
    _softwareConnected = false;
    _loadingMenu = false;

    setAvailableApplications();

    connect(_keyBoard, &Keyboard::keyEvent, this, &Panel::onKeyEvent);
    connect(_keyBoard, &Keyboard::keyboardConnectionChanged, this, &Panel::onKeyboardConnectionChanged);
    connect(&_reloadMenu, &QTimer::timeout, this, &Panel::loadMenu);
    _reloadMenu.setSingleShot(true); //setting timer because we need to call loadButton only when all button task end

    // Create tray Icon and insert QAction in it
    _trayMenu = new QMenu();

    // Add Action in reverse order
    auto it = _appActionMap.end();
    while (it != _appActionMap.begin()) {
        --it;  // Decrementa prima di accedere al valore
        _trayMenu->addAction(it.value());
    }
    _trayMenu->addAction(_quitAction);
    loadSettings();
    _tray.setIcon(QIcon(QString(":/%1").arg(ICON_NAME)));
    _tray.setContextMenu(_trayMenu);
    _tray.show();
}

void Panel::setAvailableApplications()
{
    _quitAction = new QAction(this);
    connect(_quitAction, &QAction::triggered, this, &Panel::onQuitRequest);

    _availableApps = _db->getAvailableApps();

    for (const QString& appID : _availableApps.keys())
    {
        auto appAction = new QAction(this);
        appAction->setCheckable(true);
        connect(appAction, &QAction::triggered, this, &Panel::onAppChanged);
        _appActionMap[appID] = appAction;
    }
}

/*!
 *  \brief Panel::onKey
 *  \par modifiers
 *  \par key
 *
 *  Called when a valid keystroke is sent
 */
void Panel::onKeyEvent(QList<int> keyStroke, Keyboard::keyEvent_t event)
{
    if(Button::currentButton() && Button::currentButton()->isDisclaimer() && event == Keyboard::PRESS)
    {
        hideMenu();
        return;
    }

    QString eventStr;
    switch (event)
    {
        case Keyboard::PRESS:
            eventStr = "press";
            break;
        case Keyboard::RELEASE:
            eventStr = "release";
            break;
        case Keyboard::HOLD_SHORT:
            eventStr = "hold_short";
            break;
        case Keyboard::HOLD_LONG:
            eventStr = "hold_long";
            break;
        default:
            return;
    }

    QSqlRecord commandRecord = _db->getKeystrokeRecord(keyStroke, eventStr, isVisible());
    if(!_loadingMenu)
        Metadata::invokeVoid(commandRecord);
}

/*!
 * \brief Panel::enterByKey
 * \par key
 *
 * Send command to a specific button choosen by key placed in any menu
 * ID button is passed throug QMAP at key "value"
 */
void Panel::customButtonEnter(QJsonObject command)
{
    for(auto &menu : _menuMap)
        if(menu->buttonEnter(command["value"].toString()))
            return;
}

/*!
 * \brief Panel::updateLanguage
 * \par lang
 *
 * Set language choosen beteween: "system", "en" and "it"
 *
 */
void Panel::setLanguage(QJsonObject command)
{
    _db->updateLanguage();
    _voiceOver->updateLanguage(command["value"].toString());
    updateActionLabels();
    emit languageChanged(); // so, all menu can notify it
}

/*!
 * \brief Panel::gotoWebsite
 *
 * Open system default browser and visite www.odlamusic.com
 */
void Panel::gotoWebsite()
{
    QDesktopServices::openUrl(QUrl("http://www.odlamusic.com", QUrl::TolerantMode));
    //_voiceOver->say(DBText("on_odla_website_opened"));
}

QString Panel::getInsertionButtonValue(QJsonObject buttonIDWrapper)
{
    for(auto &menu : _menuMap)
        if(menu->type() == "insertion")
            for(auto &button : menu->buttonList())
                if(button->buttonID() == buttonIDWrapper["value"].toString())
                    return button->value();
    return "";
}

/*!
 *  \brief Panel::showMenu
 *  \par menu
 *  Show panel with menu ID mapped in par1 QMAP
 */
void Panel::showMenu(QJsonObject command)
{
    auto newMenu = _menuMap[command["value"].toString()];
    if(newMenu == nullptr)  return;

    _mainWidget->setCurrentWidget(newMenu);
    static_cast<Menu*>(newMenu)->setMenuVisible(true);
    show();
    adjustSize();
    QString msg = Menu::currentMenu()->speechTitle();
    if(Button::currentButton())
        msg += ";" + Button::currentButton()->speechTitle();
}

/*!
 *  \brief Panel::hideMenu
 *  \par mute
 *
 *  Hide current menu and panel.
 *  If mute is true, vocalguide is not called
 */
void Panel::hideMenu()
{
    if(isVisible() && Menu::currentMenu())
    {
        Menu::currentMenu()->setMenuVisible(false);
        hide();
    }
}

/*!
 *  \brief Panel::setPanelPosition
 *  \par positionWrap *
 *  Move panel in a place of screen chosen by:
 *  Vertical alignement: TOP, CENTER, BOTTOM
 *  Horyzontal alignement: LEFT, CENTER, RIGHT
 *
 *  the two strings are separated by "_" char
 */
void Panel::setPanelPosition(QJsonObject positionWrap)
{
    QStringList pos = positionWrap["value"].toString().split("_");
    if(pos.size() != 2) return;
    auto v = Qt::AlignVCenter;
    auto h = Qt::AlignHCenter;

    if(pos.at(0) == "top") v = Qt::AlignTop;
    else if(pos.at(0) == "bottom") v = Qt::AlignBottom;

    if(pos.at(1) == "left") h = Qt::AlignLeft;
    else if(pos.at(1) == "right") h = Qt::AlignRight;

    auto screenRect = QGuiApplication::screens().at(0)->availableGeometry();
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, v | h, size(), screenRect));

    //auto panelMenu = static_cast<MenuStandard*>(_menuMap["panel_position"]);
    // if(panelMenu)
    //     panelMenu->updateButtons();
}

/*!
 *  \brief Panel::setSoftware
 *  \par softwareWrapper
 *
 *  Change current software
 */
void Panel::setSoftware(QJsonObject appID)
{
    _db->updateSoftware();

    QString app = appID.value("value").toString();
    if(_currentAppID == app || app.isEmpty()) return;
    _currentAppID = app;
    _softwareConnected = false;

    _app = App::instance(this, app);
    connect(_app, &App::appConnected, this, &Panel::checkSoftwareStatus);
    _app->connectToApp();
    for(auto appAction : _appActionMap)
        appAction->setChecked(false);
    _appActionMap.value(app)->setChecked(true);
    _reloadMenu.start();

}

/*!
 *  \brief Panel::isAppConnected
 *
 *  Return true if software is connected
 */
bool Panel::isAppConnected()
{
    return _softwareConnected;
}

void Panel::checkSoftwareStatus(bool connected)
{
    if(isDebug)
        qDebug() << "app connected?" << connected;

    _softwareConnected = connected;

    if(!_softwareConnected)
        hideMenu();    
}

/*!
 *  \brief Panel::loadMenu
 *
 *  Load all menus found in DB
 */
void Panel::loadMenu()
{
    hideMenu();
    _loadingMenu = true;

    for(auto &menu : _menuMap)
        if(menu)
            menu->deleteLater();

    _menuMap.clear();

    // TODO: remove DB filter from this class
    QList<QSqlRecord> menuRecordList = _db->allTableRecords("menu", QString("software_id = 'any' or software_id = '%1'").arg(_currentAppID));
    qDebug() << "menu size"<< menuRecordList.size();

    QProgressDialog b (_db->writtenText("loading_menu"), "", 0, menuRecordList.size(), this);
    b.setCancelButton(nullptr);
    b.show();
    int progress = 0;

    for(QSqlRecord &menuRecord : menuRecordList)
    {
        b.setValue(++progress);
        QCoreApplication::processEvents();
        QString type = menuRecord.value("type").toString();

        if(type == "insertion")
            initMenu(new MenuInsertion(this, menuRecord));

        if(type == "v_center")
            initMenu(new MenuVCenter(this, menuRecord));

        else
            initMenu(new MenuStandard(this, menuRecord));
    }
    b.hide();
    _loadingMenu = false;
}

/*!
 *  \brief Panel::loadSettings
 *
 *  Load all settings from DB, format is given by DB standard commands with 4 QString parameters
 */
void Panel::loadSettings()
{
    QJsonObject settingWrapper;
    settingWrapper["value"] = _db->getActiveToggleExButtons("language").toString();
    setLanguage(settingWrapper);    

    settingWrapper["value"] = _db->getActiveToggleExButtons("panel_position").toString();
    setPanelPosition(settingWrapper);

    settingWrapper["value"] = _db->getActiveToggleExButtons("preferred_menu_selection").toString();
    setPreferredMenu(settingWrapper);

    //    This will be called at keyboard connection
    //    settingWrapper["value"] = _db->getSetting("software","multi_choice_value").toString();
    //    setSoftware(settingWrapper);
}

/*!
 *  \brief Panel::createMenu
 *  \par menu
 *
 *  Init all menus and place them in their layout
 */
void Panel::initMenu(Menu *menu)
{
    _mainWidget->addWidget(menu);
    _menuMap[menu->menuID()] = menu;
    menu->loadButtons();
}

/*!
 *  \brief Panel::setPreferredMenu
 *  \par key
 *
 *  Set menu opened by FN + OPTION keystroke
 */
void Panel::setPreferredMenu(QJsonObject command)
{
    QString menuID = command["value"].toString();
    if(_menuMap[menuID] == nullptr)
        return;

    _menuMap["preferred_menu"] = _menuMap[menuID];
}

/*!
 *  \brief Panel::onAppChanged
 *  \par key
 *
 *  Slot connected to click of QAction for app change
 */
void Panel::onAppChanged()
{
    QAction* sender = qobject_cast<QAction*>(QObject::sender());
    if (sender)
    {
        const QString& key = _appActionMap.key(sender);
        _db->setActiveToggleButtons(key, true);
        QJsonObject wrapper;
        wrapper["value"] = key;
        setSoftware(wrapper);
    }
}

/*!
 *  \brief Panel::onQuitRequest
 *  \par key
 *
 *  Show blocking confirmation messagebox before quit
 */
void Panel::onQuitRequest()
{
    QMessageBox messageBox(QMessageBox::Question,
                           _db->writtenText("confirm"),
                           _db->writtenText("on_odla_quit_request"),
                           QMessageBox::Yes | QMessageBox::No,
                           this);

    if (messageBox.exec() == QMessageBox::Yes)
        QCoreApplication::quit();
}

/*!
 *  \brief Panel::updateActionLabels
 *
 *  Method to be called whenever language is set in order to update all labels
 */
void Panel::updateActionLabels()
{
    qDebug() << "updateActionLabels";
    for (const QString& appID : _appActionMap.keys())
        _appActionMap.value(appID)->setText(_db->writtenText("use_app") + " " +_db->writtenText(_availableApps[appID]));
    _quitAction->setText(_db->writtenText("on_quit"));
}


/*!
 * \brief Musescore::onKeyboardConnectionChanged
 *
 *  Method called when it is detected connection or disconnection of keyboard
 */
void Panel::onKeyboardConnectionChanged(bool keyboardConnected)
{
    VoiceOver::instance()->say(_db->speechText(keyboardConnected ? "on_keyboard_connected" :"on_keyboard_disconnected"));
    _keyboardConnected = keyboardConnected;

    if(!keyboardConnected)
        return;

    QJsonObject softwareNameWrapper;
    softwareNameWrapper["value"] = _db->getActiveToggleExButtons("software").toString();
    setSoftware(softwareNameWrapper);
    hideMenu();
}
