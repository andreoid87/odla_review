#include <QStyle>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>
#include "panel.h"
#include "database.h"
#include "voiceover.h"
#include "metadata.h"
#include <QImageReader>

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
    // Ci si aspetta da qui che _app sia inizializzato?

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

    QString iconPath = QString(":/%1").arg(ICON_NAME);
    QIcon trayIcon(iconPath);
    _tray.setIcon(trayIcon);
    _tray.setContextMenu(_trayMenu);
    _tray.show();
}

void Panel::setAvailableApplications()
{
    _quitAction = new QAction(this);
    connect(_quitAction, &QAction::triggered, this, &Panel::onQuitRequest);

    const QStringList availableApps = _db->getAvailableApps();
    for (const QString& appName : availableApps)
    {
        auto appAction = new QAction(this);
        appAction->setCheckable(true);
        connect(appAction, &QAction::triggered, this, &Panel::onAppChanged);
        _appActionMap[appName] = appAction;
    }
}

/*!
 *  \brief Panel::onKey
 *  \par modifiers
 *  \par key
 *
 *  Called when a valid keystroke is sent
 */
void Panel::onKeyEvent(QStringList keyStroke, bool pressed)
{
    if(isDebug)
        qDebug() << "pressed:"  << keyStroke;

    if(Button::currentButton() && Button::currentButton()->isDisclaimer() && pressed)
    {
        hideMenu();
        return;
    }

    QString column = QString(pressed ? "on_press" : "on_release") + (isVisible() ? "_with_panel" : "_no_panel");
    QString commandID = _db->getKeystrokeCommandID(keyStroke, column);

    if(!_loadingMenu)
        Metadata::invokeVoid(commandID, _softwareConnected);
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
 * \brief Panel::setLanguage
 * \par lang
 *
 * Set language choosen beteween: "system", "en" and "it"
 *
 */
void Panel::setLanguage(QJsonObject command)
{
    QString lang = command["value"].toString();
    _db->setValue("LANGUAGE","MULTI_CHOICE_VALUE", lang);
    _voiceOver->setLanguage();
    updateLabels();
    emit languageChanged(); // so, all menu call notify it
}

/*!
 * \brief Panel::gotoWebsite
 *
 * Open system default browser and visite www.odlamusic.com
 */
void Panel::gotoWebsite()
{
    QDesktopServices::openUrl(QUrl("http://www.odlamusic.com", QUrl::TolerantMode));
    //_voiceOver->say(DBText("ON_ODLA_WEBSITE_OPENED"));
}

QString Panel::getInsertionButtonValue(QJsonObject buttonIDWrapper)
{
    for(auto &menu : _menuMap)
        if(menu->type() == "INSERTION")
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

    if(pos.at(0) == "TOP") v = Qt::AlignTop;
    else if(pos.at(0) == "BOTTOM") v = Qt::AlignBottom;

    if(pos.at(1) == "LEFT") h = Qt::AlignLeft;
    else if(pos.at(1) == "RIGHT") h = Qt::AlignRight;

    auto screenRect = QGuiApplication::screens().at(0)->availableGeometry();
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, v | h, size(), screenRect));

    _db->setValue("PANEL_POSITION","MULTI_CHOICE_VALUE",  positionWrap["value"].toString());

    auto panelMenu = static_cast<MenuToggleEx*>(_menuMap["PANEL_POSITION"]);
    if(panelMenu)
        panelMenu->updateButtons();
}

/*!
 *  \brief Panel::setSoftware
 *  \par softwareWrapper
 *
 *  Change current software
 */
void Panel::setSoftware(QJsonObject appNameWrapper)
{
    QString appID = appNameWrapper["value"].toString();
    qDebug() << "setting software: " << appID;
    _app = App::instance(this, appID);
    connect(_app, &App::appConnected, this, &Panel::checkSoftwareStatus);
    _app->connectToApp();
    for(auto appAction : _appActionMap)
        appAction->setChecked(false);
    _appActionMap.value(appID)->setChecked(true);
    _reloadMenu.start();
    _db->setValue("SOFTWARE","MULTI_CHOICE_VALUE",  _currentAppID = appID);
}

void Panel::checkSoftwareStatus(bool connected)
{
    _softwareConnected = connected;

    if(isDebug)
        qDebug() << "App connected?" << connected;

    if(!_softwareConnected && Menu::currentMenu() != _menuMap["SOFTWARE"])
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

    auto recordsList = _db->allTableRecords("menu");
    qDebug() << "menu size"<< recordsList.size();

    QProgressDialog b (_db->writtenText("LOADING_MENU"), "", 0, recordsList.size(), this);
    b.setCancelButton(nullptr);
    b.show();
    int progress = 0;

    for(auto &record : recordsList)
    {
        b.setValue(++progress);
        QCoreApplication::processEvents();
        if(!record.value(_currentAppID).toBool())
            continue;

        if(record.value("type").toString() == "STANDARD")
            initMenu(new MenuStandard(this, record));

        if(record.value("type").toString() == "TOGGLE_EX")
            initMenu(new MenuToggleEx(this, record));

        if(record.value("type").toString() == "INSERTION")
            initMenu(new MenuInsertion(this, record));

        if(record.value("type").toString() == "V_CENTER")
            initMenu(new MenuVCenter(this, record));
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
    settingWrapper["value"] = _db->getValue("LANGUAGE","MULTI_CHOICE_VALUE").toString();
    setLanguage(settingWrapper);

    settingWrapper["value"] = _db->getValue("PANEL_POSITION","MULTI_CHOICE_VALUE").toString();
    setPanelPosition(settingWrapper);

    settingWrapper["value"] = _db->getValue("PREFERRED_MENU_SELECTION","MULTI_CHOICE_VALUE").toString();
    setPreferredMenu(settingWrapper);

    //    This will be called at keyboard connection
    //    settingWrapper["value"] = _db->getValue("SOFTWARE","MULTI_CHOICE_VALUE").toString();
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
    _db->setValue("PREFERRED_MENU_SELECTION","MULTI_CHOICE_VALUE",  menuID);
    _menuMap["PREFERRED_MENU"] = _menuMap[menuID];
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
                           _db->writtenText("CONFIRM"),
                           _db->writtenText("ON_ODLA_QUIT_REQUEST"),
                           QMessageBox::Yes | QMessageBox::No,
                           this);
    messageBox.setButtonText(QMessageBox::Yes, _db->writtenText("YES"));
    messageBox.setButtonText(QMessageBox::No, _db->writtenText("NO"));

    if (messageBox.exec() == QMessageBox::Yes)
        QCoreApplication::quit();
}

/*!
 *  \brief Panel::updateLabels
 *
 *  Method to be called whenever language is changed in order to update all labels
 */
void Panel::updateLabels()
{
    for (const QString& appID : _appActionMap.keys())
        _appActionMap.value(appID)->setText(_db->writtenText("USE_APP") + _db->writtenText(appID));
    _quitAction->setText(_db->writtenText("ON_QUIT"));
}


/*!
 * \brief Musescore::onKeyboardConnectionChanged
 *
 *  Method called when it is detected connection or disconnection of keyboard
 */
void Panel::onKeyboardConnectionChanged(bool keyboardConnected)
{
    VoiceOver::instance()->say(_db->speechText(keyboardConnected ? "ON_KEYBOARD_CONNECTED" :"ON_KEYBOARD_DISCONNECTED"));
    _keyboardConnected = keyboardConnected;

    if(!keyboardConnected)
        return;

    QJsonObject softwareNameWrapper;
    softwareNameWrapper["value"] = _db->getValue("SOFTWARE","MULTI_CHOICE_VALUE").toString();
    setSoftware(softwareNameWrapper);
    hideMenu();
}
