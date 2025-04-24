#ifndef PANEL_H
#define PANEL_H

#include "database.h"
#include "src/app.h"
#include "voiceover.h"
#include "database.h"
#include "keyboard.h"

#include <QMenu>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QShowEvent>
#include <QDesktopServices>
#include <QStackedWidget>
#include "menu.h"

/*!
 * \brief Panel class
 *
 * Class that contains all main graphical objects and manage their interaction
 */
class Panel : public QDialog
{
    Q_OBJECT

public:
    static Panel* instance(QWidget * parent = nullptr);

private:
    explicit Panel(QWidget *parent = nullptr);
    void loadSettings();
    void initMenu(Menu *menu);
    QAction *_quitAction;
    QMap<QString, QAction *>_appActionMap;
    QAction *_useDorico;
    QAction *_useFinale;
    QStackedWidget *_mainWidget;
    QMap<QString, QString> _availableApps;

    Database * _db;
    VoiceOver * _voiceOver;
    Keyboard * _keyBoard;
    App *_app;
    QMenu* _trayMenu;
    QSystemTrayIcon _tray;
    static Panel *_instance;
    QMap<QString, Menu*> _menuMap;
    bool _keyboardConnected;
    bool _softwareConnected;
    bool _loadingMenu;
    QString _currentAppID;
    QTimer _reloadMenu;

private slots:
    void onQuitRequest();    
    void onKeyboardConnectionChanged(bool keyboardConnected);
    // unable keyboard during loadng of menus and buttons
    void loadMenu();
    void setAvailableApplications();
    void checkSoftwareStatus(bool connected);

public slots:
    void showMenu(QJsonObject command);
    void hideMenu();
    void setPanelPosition(QJsonObject positionWrap);
    void setPreferredMenu(QJsonObject command); //Set menu opened by FN + OPTION keystroke
    void onAppChanged();
    void setSoftware(QJsonObject appID);
    bool isAppConnected();
    void onKeyEvent(QList<int> keyStroke, Keyboard::keyEvent_t event);
    void customButtonEnter(QJsonObject command);
    void updateActionLabels();
    void setLanguage(QJsonObject lang);
    void gotoWebsite();
    QString getInsertionButtonValue(QJsonObject buttonIDWrapper);

signals:
    void languageChanged();
};

#endif // PANEL_H
