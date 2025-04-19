#include "keyboard.h"
#include "odlamanager.h"
#include <cstring>
#include <QTimer>
#include <QSerialPortInfo>
#include "metadata.h"
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
    connect(_reSendTimer, &QTimer::timeout, this, &Keyboard::reSendLastKey);
    connect(this, &Keyboard::repeatStart, _reSendTimer, QOverload<int>::of(&QTimer::start));
    connect(this, &Keyboard::repeatStop, _reSendTimer, &QTimer::stop);

    connect(&_odlaSerial, &QSerialPort::readyRead, this, &Keyboard::readCallback);
    connect(&_queueTimer, &QTimer::timeout, this, &Keyboard::ksSendTask);
    _queueTimer.setSingleShot(true);

    _nextRepeatTime = FIRST_PRESS_TIME;
    _currentModifiers = 0;
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
        if (!serialWrite(outPkt))
            emit keyboardConnectionChanged(false);
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
        QByteArray buffer = _odlaSerial.readAll();
        // parse data
        if(buffer.size() > 0)
        {
            hidReport = buffer[0];
            keyCode = buffer[1];
        }
    }

    if(hidReport == ODLA_HID_PRESSED_REPORT)
        pressedKey(keyCode);

    else if(hidReport == ODLA_HID_RELEASED_REPORT)
        releasedKey(keyCode);
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

//    if(isDebug)
//        qDebug() << "Pressed key:"  << keyCode;

    bool repeat, modifier;
    QString keyName = Database::instance()->getKeyName(keyCode, &repeat, &modifier);

//    if(modifier)
//        _currentPressedKeys.prepend(keyName);
//    else
        _currentPressedKeys.append(keyName);

    // qDebug() << "keyPressed" << _currentPressedKeys;

    if(repeat)
        emit repeatStart(FIRST_PRESS_TIME);

    emitKeyPress(true);

    _lastKeyCode = keyCode;
    _lastPressTimer.start();
}

/*!
 * \brief Keyboard::releasedKey
 * \par keyCode
 *
 * Triggered when an ODLA key is released
 */
void Keyboard::releasedKey(uint8_t keyCode)
{
    bool repeat, modifier;
    QString keyName = Database::instance()->getKeyName(keyCode, &repeat, &modifier);

    if(isDebug)
        qDebug() << "Released key:"  << keyCode;

    emitKeyPress(false);
    _currentPressedKeys.removeAll(keyName);

    if(repeat)
        emit repeatStop();
}

/*!
 * \brief Keyboard::keyRepetition
 *
 * task for repetition key event
 */
void Keyboard::reSendLastKey()
{
    emitKeyPress(true);
    emit repeatStart(REPEAT_PRESS_TIME);
}

void Keyboard::emitKeyPress(bool press)
{    
    emit keyEvent(_currentPressedKeys, press);
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

    if (!serialWrite(outPkt))
        emit keyboardConnectionChanged(false);

    if(!_ksQueue.isEmpty())
        _queueTimer.start(50);
}

/*!
 * \brief Keyboard::write
 *
 * Send data to ODLA in serial Mode
 */
bool Keyboard::serialWrite(USBData_t pkt)
{
    if(_odlaSerial.isOpen())
        return _odlaSerial.write((char*)pkt.data, 64) != -1;
    else
        return false;
}

/*!
 * \brief Keyboard::sendQwerty
 *
 *  Load QWERTY from DB and relative vocal guide message
 */
void Keyboard::sendQwerty(QJsonObject command)
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

    qDebug() << "Sending midi pitch: " << pitch;
    if (!serialWrite(outPkt))
        emit keyboardConnectionChanged(false);
}
