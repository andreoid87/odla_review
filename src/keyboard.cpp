#include "keyboard.h"
#include <cstring>
#include <QTimer>
#include <QSerialPortInfo>
#include "metadata.h"
#include "database.h"
#define CONNECTCAST(OBJECT,TYPE,FUNC) static_cast<void(OBJECT::*)(TYPE)>(&OBJECT::FUNC)

extern bool isDebug;
Keyboard * Keyboard::_instance;
const QList<QPair<int, int>> Keyboard::_vidPidToScan =  QList<QPair<int, int>>()
                                                       << QPair<int, int>(ODLA_USB_VID, ODLA_USB_PID)
                                                       << QPair<int, int>(LEONARDO_USB_VID, LEONARDO_USB_PID);

Keyboard::Keyboard(QObject *parent) : QThread(parent)
{
    
    _odlaHID = NULL;
    _repeatKey = -1;
    hid_init();

    QTimer *connectionTimer = new QTimer();
    connectionTimer->setInterval(1000);
    connect(connectionTimer, &QTimer::timeout, this, &Keyboard::checkConnection);
    connectionTimer->start();

    QTimer *hotPlugTimer = new QTimer();
    hotPlugTimer->setInterval(1000);
    connect(hotPlugTimer, &QTimer::timeout, this, &Keyboard::checkHotPlug);
    hotPlugTimer->start();

    _reSendTimer = new QTimer();
    _reSendTimer->setSingleShot(false);
    connect(_reSendTimer, &QTimer::timeout, this, &Keyboard::reSendLastKeystroke);
    connect(this, &Keyboard::repeatStart, _reSendTimer, QOverload<int>::of(&QTimer::start));
    connect(this, &Keyboard::repeatStop, _reSendTimer, &QTimer::stop);

    _holdTimer = new QTimer();
    _holdTimer->setSingleShot(true);
    connect(_holdTimer, &QTimer::timeout, this, &Keyboard::onHoldTimerExpired);

    connect(&_odlaSerial, &QSerialPort::readyRead, this, &Keyboard::readCallback);
    connect(&_queueTimer, &QTimer::timeout, this, &Keyboard::ksSendTask);
    _queueTimer.setSingleShot(true);

    connect(&serialQueueTimer, &QTimer::timeout, this, &Keyboard::processQueue);
    serialQueueTimer.setSingleShot(false);

    _nextRepeatTime = FIRST_PRESS_TIME;
    _currentModifiers = 0;
    _lastKeyCode = -1;
    //_repeatMap = Database::instance()->getRepeatKeys(); // TODO: MOVE TO PANEL
}

/*!
 * \brief Keyboard::instance
 *
 * Singleton pattern
 * \warning to be changed
 */
Keyboard *Keyboard::instance(QObject *parent)
{
    if(!_instance)
        _instance = new Keyboard(parent);
    Metadata::addCallableInstance(_instance);
    return _instance;
}

Keyboard::~Keyboard()
{
    close();
}

/*!
 * \brief Keyboard::checkConnection
 *
 * Sends a message to keyboard to prevent it enter in blink mode
 * i.e. whatchdog periodically checks for disconnection
 */
void Keyboard::checkConnection()
{
    if(_odlaHID != NULL && !_odlaSerial.isOpen())
    {   // ODLA works in RAW HID Mode
        uint8_t data[2] = {0x00, ODLA_HID_LED_ON_REPORT};
        if(hid_write(_odlaHID, data, 2) == -1)
            emit keyboardConnectionChanged(false);
    }
    else if(_odlaHID == NULL && _odlaSerial.isOpen())
    {   // ODLA works in serial Mode
        USBData_t outPkt;
        outPkt.header.command = WAKE_UP_COMMAND;
        serialWrite(outPkt);
    }
}

/*!
 * \brief Keyboard::LedOff
 *
 * May be called to turn leds off (not used for the moment)

void Keyboard::LedOff()
{
    if(_currentDevice == NULL)
        return;

    uint8_t data[2] = {0x00, ODLA_HID_LED_OFF_REPORT};
    if(hid_write(_currentDevice, data, 2) == -1)
        emit keyboardConnectionChanged(false);
}
 */

/*!
 * \brief Keyboard::readCallback
 *
 * Since the HID library does not offer callback for received messages from USB HID buffer
 * We need to continuously read from it in order to check new messages
 */
void Keyboard::readCallback()
{
    uint8_t hidReport = 0xFF;
    uint8_t keyCode = 0xFF;

    if(_odlaHID != NULL && !_odlaSerial.isOpen())
    {
        uint8_t buffer[64]{0};
        if(hid_read(_odlaHID, buffer, 64) >= 2)
        {
            // parse data
            hidReport = buffer[0];
            keyCode = buffer[1];
        }
    }
    else if(_odlaHID == NULL && _odlaSerial.isOpen())
    {
        while (_odlaSerial.bytesAvailable() >= 2) // Leggiamo sempre due byte alla volta se disponibili
        {
            QByteArray buffer = _odlaSerial.read(2); // Leggiamo due byte esatti
            hidReport = buffer[0];
            keyCode = buffer[1];

            //qDebug() << (hidReport == ODLA_HID_PRESSED_REPORT ? "press" : "release") << keyCode;
            hidReport == ODLA_HID_PRESSED_REPORT ? pressedKey(keyCode) : releasedKey(keyCode);
        }
    }
}

/*!
 * \brief Keyboard::run
 *
 * The infinite loop in the thread that reads from HID USB buffer
 */
void Keyboard::run()
{
    while(1)
    {
        readCallback();
        msleep(10);
    }
}

/*!
 * \brief Keyboard::pressedKey
 * \par keyCode
 *
 * Triggered when an ODLA key is pressed
 */
void Keyboard::pressedKey(uint8_t keyCode)
{
    if(_lastPressTimer.restart() < DEBOUNCE_THRESHOLD && _lastKeyCode == keyCode)
        return;

    _currentPressedKeys.append(keyCode);

    // TODO: Questa deve andare nel medoto dell'intero keystroke
    // if(_repeatMap.value(keyCode))
    //     emit repeatStart(FIRST_PRESS_TIME);
    emit keyEvent(/*_currentPressedKeys*/ QList<int>() << keyCode, PRESS);

    _lastKeyCode = keyCode;
    _lastPressTimer.start();
    _shortPress = true; // by default we should retain short press since timer slot edit this value
    _holdTimer->start(HOLD_TIME);
    _shortPress = true;
}

/*!
 * \brief Keyboard::releasedKey
 * \par keyCode
 *
 * Triggered when an ODLA key is released
 */
void Keyboard::releasedKey(uint8_t keyCode)
{
    if(_shortPress && _currentPressedKeys.size() > 0)
        emit keyEvent(/*_currentPressedKeys*/ QList<int>() << keyCode, HOLD_SHORT);
    emit keyEvent(/*_currentPressedKeys*/ QList<int>() << keyCode, RELEASE);
    _currentPressedKeys.removeAll(keyCode);
    qDebug() << _currentPressedKeys;

    _holdTimer->stop();
    // if(_repeatMap.value(keyCode))
    //     emit repeatStop();
}

/*!
 * \brief Keyboard::onHoldTimerExpired
 * \par keyCode
 *
 * Triggered when a key was pressed for a long time
 */
void Keyboard::onHoldTimerExpired()
{
    if(_currentPressedKeys.size() > 0)
        emit keyEvent(_currentPressedKeys, HOLD_LONG);
    _holdTimer->stop();
    _shortPress = false;
}

/*!
 * \brief Keyboard::keyRepetition
 *
 * task for repetition key event
 */
void Keyboard::reSendLastKeystroke()
{
    // emit keyEvent(_currentPressedKeys, PRESS);
    // emit repeatStart(REPEAT_PRESS_TIME);
}

/*!
 * \brief Keyboard::open
 *
 * This function try to open ODLA in HID mode BEFORE
 * if it fails, then try to open it in serial mode
 */
void Keyboard::open(QString devicePath)
{

    if(_odlaHID != NULL)
        close();

    if(_odlaSerial.isOpen())
        _odlaSerial.close();

    _odlaHID = hid_open_path(devicePath.toStdString().c_str());

    // Here ODLA works in HID mode
    if(_odlaHID != NULL)
    {
        qDebug() << "Device works in HDI Mode";
        if(hid_set_nonblocking(_odlaHID, 1) == 0)
        {
            emit keyboardConnectionChanged(true);
            start();
        }
        else
            _odlaHID = NULL;
    }
    // Here ODLA works in Serial mode
    if(_odlaHID == NULL)
    {
        QString path;
        const auto serialPortInfos = QSerialPortInfo::availablePorts();

        for (const QSerialPortInfo &serialPortInfo : serialPortInfos)
        {
            int VID = serialPortInfo.vendorIdentifier();
            int PID = serialPortInfo.productIdentifier();
            if(VID == ODLA_USB_VID && PID == ODLA_USB_PID)
                path = serialPortInfo.portName();
        }
        if(!path.isEmpty()) //TODO: use argument of this function to identify
        {
            qDebug() << "Device works in Serial Mode, serial port:" << path;
            _odlaSerial.setPortName(path);
            _odlaSerial.setBaudRate(QSerialPort::Baud115200);
            _odlaSerial.setDataBits(QSerialPort::Data8);
            _odlaSerial.setParity(QSerialPort::NoParity);
            _odlaSerial.setStopBits(QSerialPort::OneStop);
            _odlaSerial.setFlowControl(QSerialPort::NoFlowControl);
            if(_odlaSerial.open(QSerialPort::ReadWrite))
            {
                _odlaSerial.setDataTerminalReady(true);
                _odlaSerial.setRequestToSend(true);
                checkConnection();
                emit keyboardConnectionChanged(true);
            }
            else
            {
                close();
                qDebug() << "Unable to open the device" << _odlaSerial.errorString();
            }
        }
    }

}

/*!
 * \brief Keyboard::close
 */
void Keyboard::close()
{
    hid_close(_odlaHID);
    _odlaSerial.close();
    _odlaHID = NULL;
    emit keyboardConnectionChanged(false);
    terminate();
}

/*!
 * \brief Keyboard::checkHotPlug
 *
 * Detects if keyboard is connected/disconnected
 */
void Keyboard::checkHotPlug()
{
    hid_device_info *newDeviceInfo = nullptr;
    for(auto &item : _vidPidToScan)
    {
        newDeviceInfo = hid_enumerate(item.first, item.second);
        if(newDeviceInfo)
        {
            if(!_odlaHID && !_odlaSerial.isOpen())
                open(newDeviceInfo->path);
            break;
        }
    }
    if((_odlaHID || _odlaSerial.isOpen()) && !newDeviceInfo)
        close();
    hid_free_enumeration(newDeviceInfo);
}

/*!
 * \brief Keyboard::ksSendTask
 *
 * Send keyboard key stroke
 */
void Keyboard::ksSendTask()
{
    ks_t ks = _ksQueue.dequeue();
    USBData_t outPkt;
    outPkt.header.command = QWERTY_COMMAND;
    outPkt.quertyComm.howMany = ks.size() * 2; //events incldue press and release
    for(int i = 0; i < ks.size(); i++)
        outPkt.quertyComm.qwertyEvent[i] = {ks.at(i), true};

    for(int i = 0; i < ks.size(); i++)
        outPkt.quertyComm.qwertyEvent[ks.size() + i] = {ks.at(i), false};

    serialWrite(outPkt);

    if(!_ksQueue.isEmpty())
        _queueTimer.start(50);
}

/*!
 * \brief Keyboard::write
 *
 * Send data to ODLA in serial Mode
 */
void Keyboard::serialWrite(USBData_t pkt)
{
    packetQueue.enqueue(pkt);
    if (!serialQueueTimer.isActive())
        serialQueueTimer.start(20);
}

/*
 * \brief Keyboard::processQueue
 *
 * Send data to ODLA in serial Mode
 */
void Keyboard::processQueue()
{
    if (!_odlaSerial.isOpen() || packetQueue.isEmpty())
    {
        serialQueueTimer.stop(); // Se la porta è chiusa o la coda è vuota, ferma il timer
        return;
    }

    USBData_t pkt = packetQueue.dequeue();
    if(_odlaSerial.write((char*)pkt.data, 64) == -1)
        emit keyboardConnectionChanged(false);

    if (packetQueue.isEmpty())
        serialQueueTimer.stop();
}
/*!
 * \brief Keyboard::sendKeyboardEvent
 *
 *  Load QWERTY from DB and send just one key event
 *  press or release
 */
void Keyboard::sendKeyboardEvent(QJsonObject command)
{
    auto key = command.value("key");
    auto press = command.value("press");
    ks_t keyStroke;
    keyStroke.append(Database::instance()->getQwertyCode(key.toString()));
    USBData_t outPkt;
    outPkt.header.command = QWERTY_COMMAND;
    outPkt.quertyComm.howMany = 1;
    outPkt.quertyComm.qwertyEvent[0] = {keyStroke.at(0), press.toBool()};
    serialWrite(outPkt);
}


/*!
 * \brief Keyboard::sendKeystroke
 *
 *  Load QWERTY from DB and relative vocal guide message
 */
void Keyboard::sendKeystroke(QJsonObject command)
{
    auto key1 = command.value("key1");
    auto key2 = command.value("key2");
    auto key3 = command.value("key3");
    auto key4 = command.value("key4");

    ks_t keyStroke;
    if(!key1.isUndefined())
        keyStroke.append(Database::instance()->getQwertyCode(key1.toString()));
    if(!key2.isUndefined())
        keyStroke.append(Database::instance()->getQwertyCode(key2.toString()));
    if(!key3.isUndefined())
        keyStroke.append(Database::instance()->getQwertyCode(key3.toString()));
    if(!key4.isUndefined())
        keyStroke.append(Database::instance()->getQwertyCode(key4.toString()));
    _ksQueue.enqueue(keyStroke);

    ksSendTask();
}
/*!
 * \brief Keyboard::sendQwerty
 *
 *  Load QWERTY from DB and relative vocal guide message
 */
void Keyboard::sendMidi(QJsonObject command)
{
    if(command.value("pitch").isUndefined() || command.value("press").isUndefined())
        return;
    int pitch = command.value("pitch").toInt();
    bool press = command.value("press").toBool();
    USBData_t outPkt;
    outPkt.header.command = MIDI_COMMAND;
    outPkt.midiComm.howMany = 1;
    outPkt.midiComm.midiEvent[0].pitch = pitch;
    outPkt.midiComm.midiEvent[0].press = press;
    outPkt.midiComm.midiEvent[0].channel = 1;
    outPkt.midiComm.midiEvent[0].velocity = 64;
    serialWrite(outPkt);
}
