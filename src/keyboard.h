#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QObject>
#include <QMap>
#include <QElapsedTimer>
#include <QSerialPort>
#include <QThread>
#include <QTimer>
#include <QQueue>
#include "hidapi.h"
#include "database.h"

typedef QList<uint8_t> ks_t;

#define FIRST_PRESS_TIME 500
#define REPEAT_PRESS_TIME 180
#define DEBOUNCE_THRESHOLD 180

#define ODLA_USB_NAME                       L"ODLA Keyboard"
#define ODLA_USB_VID                        0x04D8
#define ODLA_USB_PID                        0xEBF7

#define LEONARDO_USB_NAME                       L"ODLA Keyboard"
#define LEONARDO_USB_VID                        0x2341
#define LEONARDO_USB_PID                        0x8036

#define ODLA_HID_PRESSED_REPORT             0x10
#define ODLA_HID_RELEASED_REPORT            0x12
#define ODLA_HID_LED_RGB_REPORT             0x20
#define ODLA_HID_LED_ON_REPORT              0x22
#define ODLA_HID_LED_OFF_REPORT             0x23

enum command_t : uint8_t
{
    WAKE_UP_COMMAND = 0,
    MIDI_COMMAND = 1,
    QWERTY_COMMAND = 2
};

struct USB_HEADER
{
    command_t command;
};

struct WAKEUP_COMMAND_t
{
    struct USB_HEADER header; //just to reserve space
};

struct MIDI_t
{
    uint8_t channel;
    uint8_t pitch;
    uint8_t velocity;
    uint8_t press;
};

struct MIDI_COMMAND_t
{
    struct USB_HEADER header; //just to reserve space
    uint8_t howMany;
    struct MIDI_t midiEvent[15];
};

struct QWERTY_t
{
    uint8_t key;
    uint8_t press;
};

struct QWERTY_COMMAND_t
{
    struct USB_HEADER header; //just to reserve space
    uint8_t howMany;
    struct QWERTY_t qwertyEvent[15];
};

union USBData_t
{
    struct USB_HEADER header;
    struct WAKEUP_COMMAND_t ledComm;
    struct MIDI_COMMAND_t midiComm;
    struct QWERTY_COMMAND_t quertyComm;
    uint8_t data[64]; //add one for HID_REPORT issue
};

/*!
 * \brief Keyboard class
 *
 * This class implements a thread that continuously read from USB
 * for press/release events from ODLA Keyboard, uses RAW HID USB protocol
 * i.e. the lower level of USB stack protocol used for mouse, keyboard devices
 *
 */
class Keyboard : public QThread
{
    Q_OBJECT
public:
    static Keyboard* instance(QObject *parent = nullptr);
    ~Keyboard();

public slots:
    void open(QString devicePath);
    void close();
    void checkConnection();
    void sendQwerty(QJsonObject command);
    void sendMidi(QJsonObject command);
    //void LedOff();

private:
    explicit Keyboard(QObject *parent = nullptr);
    void readCallback();
    void run() override;
    void pressedKey(uint8_t keyCode);
    void releasedKey(uint8_t keyCode);
    bool serialWrite(USBData_t pkt);

    static const QList<QPair<int, int>> _vidPidToScan;
    QElapsedTimer _lastPressTimer;
    static Keyboard * _instance;
    hid_device *_odlaHID;
    QSerialPort _odlaSerial;
    QTimer _queueTimer;
    QTimer *_reSendTimer;
    QQueue<ks_t> _ksQueue;

    QStringList _currentPressedKeys;
    int _currentModifiers;
    int _repeatKey;
    int _nextRepeatTime;
    int _lastKeyCode;
    bool _repeat;

signals:
    void keyEvent(QStringList, bool);
    void keyboardConnectionChanged(bool);

    void repeatStart(int);
    void repeatStop();

private slots:
    void checkHotPlug();
    void ksSendTask();
    void reSendLastKey();
    void emitKeyPress(bool press);
};

#endif // KEYBOARD_H
