//#ifndef ODLAMANAGER_H
//#define ODLAMANAGER_H

//#include <QObject>
//#include "odlacontroller.h"
//#include "menudialog.h"

///*!
// * \brief The ODLAManager class
// */
//class ODLAManager : public QObject
//{
//    Q_OBJECT
//public:
//    static ODLAManager* instance() {return (_instance ? _instance : _instance = new ODLAManager());}
//    ODLAControllerV2 *getController() const;
//    void setOldController(ODLAControllerV2 *value);
//    void setNewDialog(MenuDialog *value);
//    void deviceConnectionChanged(bool connected, QString path);
//    void deviceError(QString message);

//public slots:
//    void keyPressed(uint8_t key);
//    void keyReleased(uint8_t key);

//signals:
//    void menuKeyPressed(bool fn);
//    void clefKeyPressed(bool fn);
//    void timeSigKeyPressed(bool fn);
//    void keySigKeyPressed(bool fn);
//    void barsKeyPressed(bool fn);
//    void optionsKeyPressed(bool fn);
//    void copyKeyPressed(bool fn);
//    void pasteKeyPressed(bool fn);
//    void selectKeyPressed(bool fn);
//    void functionKeyPressed(bool fn);
//    void staffKeyPressed(int k, bool fn);
//    void gotoKeyPressed(bool fn);
//    void playKeyPressed(bool fn);
//    void pauseKeyPressed(bool fn);
//    void metronomeKeyPressed(bool fn);
//    void plusKeyPressed(bool fn);
//    void minusKeyPressed(bool fn);
//    void sharpKeyPressed(bool fn);
//    void naturalKeyPressed(bool fn);
//    void flatKeyPressed(bool fn);
//    void voicesKeyPressed(bool fn);
//    void intervalKeyPressed(bool fn);
//    void chordKeyPressed(bool fn);
//    void tupletKeyPressed(bool fn);
//    void slurKeyPressed(bool fn);
//    void numKeyPressed(int k, bool fn);
//    void dotKeyPressed(bool fn);
//    void undoKeyPressed(bool fn);
//    void redoKeyPressed(bool fn);
//    void enterKeyPressed(bool fn);
//    void cancelKeyPressed(bool fn);
//    void upKeyPressed(bool fn);
//    void downKeyPressed(bool fn);
//    void leftKeyPressed(bool fn);
//    void rightKeyPressed(bool fn);
//    void questionKeyPressed(bool fn);
//    void helpKeyPressed(bool fn);

//    void menuKeyReleased(bool fn);
//    void clefKeyReleased(bool fn);
//    void timeSigKeyReleased(bool fn);
//    void keySigKeyReleased(bool fn);
//    void barsKeyReleased(bool fn);
//    void optionsKeyReleased(bool fn);
//    void copyKeyReleased(bool fn);
//    void pasteKeyReleased(bool fn);
//    void selectKeyReleased(bool fn);
//    void functionKeyReleased(bool fn);
//    void staffKeyReleased(int k, bool fn);
//    void gotoKeyReleased(bool fn);
//    void playKeyReleased(bool fn);
//    void pauseKeyReleased(bool fn);
//    void metronomeKeyReleased(bool fn);
//    void plusKeyReleased(bool fn);
//    void minusKeyReleased(bool fn);
//    void sharpKeyReleased(bool fn);
//    void naturalKeyReleased(bool fn);
//    void flatKeyReleased(bool fn);
//    void voicesKeyReleased(bool fn);
//    void intervalKeyReleased(bool fn);
//    void chordKeyReleased(bool fn);
//    void tupletKeyReleased(bool fn);
//    void slurKeyReleased(bool fn);
//    void numKeyReleased(int k, bool fn);
//    void dotKeyReleased(bool fn);
//    void undoKeyReleased(bool fn);
//    void redoKeyReleased(bool fn);
//    void enterKeyReleased(bool fn);
//    void cancelKeyReleased(bool fn);
//    void upKeyReleased(bool fn);
//    void downKeyReleased(bool fn);
//    void leftKeyReleased(bool fn);
//    void rightKeyReleased(bool fn);
//    void questionKeyReleased(bool fn);
//    void helpKeyReleased(bool fn);

//protected:
//    explicit ODLAManager(QObject *parent = nullptr);
//    static ODLAManager *_instance;
//    QThread *_deviceThread;
//    ODLAControllerV2 *_controller;
//    MenuDialog *_menuDialog;
//    QTimer _staffRepeatTimer;
//    QMetaObject::Connection _staffTimerRepeat;

//    QTimer _arrowRepeatTimer;
//    QMetaObject::Connection _arrowTimerRepeat;

//    bool _fnPressed;
//    bool _selectPressed;


//    void startArrowKeyRepeat(OdlaVK arrow);
//    void stopArrowKeyRepeat();

//    static const int KEY_START_REPEAT_TIMEOUT;
//    static const int KEY_REPEAT_TIMEOUT;

//    void startStaffKeyRepeat(int k);
//    void stopStaffKeyRepeat();
//};

//#endif // ODLAMANAGER_H
