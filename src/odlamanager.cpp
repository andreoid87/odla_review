//#include "odlamanager.h"
//#include "menudialog.h"
//#include "odladevice.h"
//#include <QDebug>

//const int ODLAManager::KEY_START_REPEAT_TIMEOUT = 600; // ms
//const int ODLAManager::KEY_REPEAT_TIMEOUT = 200; // ms

//ODLAManager * ODLAManager::_instance;

///*!
// * \brief ODLAManager::ODLAManager
// * \param parent
// */
//ODLAManager::ODLAManager(QObject *parent) : QObject(parent)
//{
////    _hotplug = new ODLAHotplug(this);
////    _device = nullptr;
//    _controller = nullptr;
//    _fnPressed = false;
//    _selectPressed = false;

////    _device = ODLADevice::instance();
////    connect(_device, &ODLADevice::keyPressed, this, &ODLAManager::keyPressed);
////    connect(_device, &ODLADevice::keyReleased, this, &ODLAManager::keyReleased);
////    connect(_device, &ODLADevice::error, this, &ODLAManager::deviceError);
//    connect(this, &ODLAManager::staffKeyReleased, this, &ODLAManager::stopStaffKeyRepeat);
//    connect(this, &ODLAManager::upKeyReleased, this, &ODLAManager::stopArrowKeyRepeat);
//    connect(this, &ODLAManager::downKeyReleased, this, &ODLAManager::stopArrowKeyRepeat);
//    connect(this, &ODLAManager::leftKeyReleased, this, &ODLAManager::stopArrowKeyRepeat);
//    connect(this, &ODLAManager::rightKeyReleased, this, &ODLAManager::stopArrowKeyRepeat);
//    qRegisterMetaType<uint8_t>("uint8_t");
//    //connect(_hotplug, &ODLAHotplug::deviceConnectionChanged, this, &ODLAManager::deviceConnectionChanged);
//}

///*!
// * \brief ODLAManager::keyPressed
// * \param key
// */
//void ODLAManager::keyPressed(uint8_t key)
//{
//    if(!_controller)
//        return;

//    _controller->setLastKeyPressed(key);
//    switch (key)
//    {
//        case OdlaVK::FUNCTION:
//        {
//            _fnPressed = true;
//            break;
//        }
//        case OdlaVK::UP:
//        case OdlaVK::DOWN:
//        case OdlaVK::LEFT:
//        case OdlaVK::RIGHT:
//        {
//            startArrowKeyRepeat(OdlaVK(key));
//            break;
//        }
//        case OdlaVK::SELECT:
//        {
//            _selectPressed = true;
//        } break;
//        case OdlaVK::CLEF: { emit clefKeyPressed(_fnPressed); } break;
//        case OdlaVK::TIMESIG: { emit timeSigKeyPressed(_fnPressed); } break;
//        case OdlaVK::KEYSIG: { emit keySigKeyPressed(_fnPressed); } break;
//        case OdlaVK::BARS: { emit barsKeyPressed(_fnPressed); } break;
//        case OdlaVK::OPTIONS: { emit optionsKeyPressed(_fnPressed); } break;
//        case OdlaVK::COPY: { emit copyKeyPressed(_fnPressed); } break;
//        case OdlaVK::PASTE: { emit pasteKeyPressed(_fnPressed); } break;
//        case OdlaVK::STAFF00: { emit startStaffKeyRepeat(-5); } break;
//        case OdlaVK::STAFF01: { emit startStaffKeyRepeat(-4); } break;
//        case OdlaVK::STAFF02: { emit startStaffKeyRepeat(-3); } break;
//        case OdlaVK::STAFF03: { emit startStaffKeyRepeat(-2); } break;
//        case OdlaVK::STAFF04: { emit startStaffKeyRepeat(-1); } break;
//        case OdlaVK::STAFF05: { emit startStaffKeyRepeat(0); } break;
//        case OdlaVK::STAFF06: { emit startStaffKeyRepeat(1); } break;
//        case OdlaVK::STAFF07: { emit startStaffKeyRepeat(2); } break;
//        case OdlaVK::STAFF08: { emit startStaffKeyRepeat(3); } break;
//        case OdlaVK::STAFF09: { emit startStaffKeyRepeat(4); } break;
//        case OdlaVK::STAFF10: { emit startStaffKeyRepeat(5); } break;
//        case OdlaVK::STAFF11: { emit startStaffKeyRepeat(6); } break;
//        case OdlaVK::STAFF12: { emit startStaffKeyRepeat(7); } break;
//        case OdlaVK::STAFF13: { emit startStaffKeyRepeat(8); } break;
//        case OdlaVK::STAFF14: { emit startStaffKeyRepeat(9); } break;
//        case OdlaVK::STAFF15: { emit startStaffKeyRepeat(10); } break;
//        case OdlaVK::STAFF16: { emit startStaffKeyRepeat(11); } break;
//        case OdlaVK::STAFF17: { emit startStaffKeyRepeat(12); } break;
//        case OdlaVK::STAFF18: { emit startStaffKeyRepeat(13); } break;
//        case OdlaVK::GOTO: { emit gotoKeyPressed(_fnPressed); } break;
//        case OdlaVK::PLAY: { emit playKeyPressed(_fnPressed); } break;
//        case OdlaVK::PAUSE: { emit pauseKeyPressed(_fnPressed); } break;
//        case OdlaVK::METRONOME: { emit metronomeKeyPressed(_fnPressed); } break;
//        case OdlaVK::PLUS: { emit plusKeyPressed(_fnPressed); } break;
//        case OdlaVK::MINUS: { emit minusKeyPressed(_fnPressed); } break;
//        case OdlaVK::SHARP: { emit sharpKeyPressed(_fnPressed); } break;
//        case OdlaVK::NATURAL: { emit naturalKeyPressed(_fnPressed); } break;
//        case OdlaVK::FLAT: { emit flatKeyPressed(_fnPressed); } break;
//        case OdlaVK::VOICES: { emit voicesKeyPressed(_fnPressed); } break;
//        case OdlaVK::INTERVAL: { emit intervalKeyPressed(_fnPressed); } break;
//        case OdlaVK::CHORD: { emit chordKeyPressed(_fnPressed); } break;
//        case OdlaVK::TUPLET: { emit tupletKeyPressed(_fnPressed); } break;
//        case OdlaVK::SLUR: { emit slurKeyPressed(_fnPressed); } break;
//        case OdlaVK::NUM0: { emit numKeyPressed(0, _fnPressed); } break;
//        case OdlaVK::NUM1: { emit numKeyPressed(1, _fnPressed); } break;
//        case OdlaVK::NUM2: { emit numKeyPressed(2, _fnPressed); } break;
//        case OdlaVK::NUM3: { emit numKeyPressed(3, _fnPressed); } break;
//        case OdlaVK::NUM4: { emit numKeyPressed(4, _fnPressed); } break;
//        case OdlaVK::NUM5: { emit numKeyPressed(5, _fnPressed); } break;
//        case OdlaVK::NUM6: { emit numKeyPressed(6, _fnPressed); } break;
//        case OdlaVK::NUM7: { emit numKeyPressed(7, _fnPressed); } break;
//        case OdlaVK::NUM8: { emit numKeyPressed(8, _fnPressed); } break;
//        case OdlaVK::NUM9: { emit numKeyPressed(9, _fnPressed); } break;
//        case OdlaVK::DOT: { emit dotKeyPressed(_fnPressed); } break;
//        case OdlaVK::UNDO: { emit undoKeyPressed(_fnPressed); } break;
//        case OdlaVK::REDO: { emit redoKeyPressed(_fnPressed); } break;
//        case OdlaVK::ENTER: { emit enterKeyPressed(_fnPressed); } break;
//        case OdlaVK::CANCEL: { emit cancelKeyPressed(_fnPressed); } break;
//        case OdlaVK::QUESTION: { emit questionKeyPressed(_fnPressed); } break;
//        default:
//            MenuDialog::instance()->onKey(_fnPressed, _selectPressed, key);
//    }
//}

///*!
// * \brief ODLAManager::keyReleased
// * \param key
// */
//void ODLAManager::keyReleased(uint8_t key)
//{
//    if(!_controller)
//        return;

//    _controller->setLastKeyReleased(key);
//    switch (key)
//    {
//        case OdlaVK::FUNCTION:
//        {
//            _fnPressed = false;
//            break;
//        }
//        case OdlaVK::UP:
//        case OdlaVK::DOWN:
//        case OdlaVK::LEFT:
//        case OdlaVK::RIGHT:
//        {
//            stopArrowKeyRepeat();
//            break;
//        }
//        case OdlaVK::MENU: { emit menuKeyReleased(_fnPressed); } break;
//        case OdlaVK::SELECT: { emit selectKeyReleased(_fnPressed); } break;
//        case OdlaVK::CLEF: { emit clefKeyReleased(_fnPressed); } break;
//        case OdlaVK::TIMESIG: { emit timeSigKeyReleased(_fnPressed); } break;
//        case OdlaVK::KEYSIG: { emit keySigKeyReleased(_fnPressed); } break;
//        case OdlaVK::BARS: { emit barsKeyReleased(_fnPressed); } break;
//        case OdlaVK::OPTIONS: { emit optionsKeyReleased(_fnPressed); } break;
//        case OdlaVK::COPY: { emit copyKeyReleased(_fnPressed); } break;
//        case OdlaVK::PASTE: { emit pasteKeyReleased(_fnPressed); } break;
//        case OdlaVK::STAFF00: { emit staffKeyReleased(-5, _fnPressed); } break;
//        case OdlaVK::STAFF01: { emit staffKeyReleased(-4, _fnPressed); } break;
//        case OdlaVK::STAFF02: { emit staffKeyReleased(-3, _fnPressed); } break;
//        case OdlaVK::STAFF03: { emit staffKeyReleased(-2, _fnPressed); } break;
//        case OdlaVK::STAFF04: { emit staffKeyReleased(-1, _fnPressed); } break;
//        case OdlaVK::STAFF05: { emit staffKeyReleased(0, _fnPressed); } break;
//        case OdlaVK::STAFF06: { emit staffKeyReleased(1, _fnPressed); } break;
//        case OdlaVK::STAFF07: { emit staffKeyReleased(2, _fnPressed); } break;
//        case OdlaVK::STAFF08: { emit staffKeyReleased(3, _fnPressed); } break;
//        case OdlaVK::STAFF09: { emit staffKeyReleased(4, _fnPressed); } break;
//        case OdlaVK::STAFF10: { emit staffKeyReleased(5, _fnPressed); } break;
//        case OdlaVK::STAFF11: { emit staffKeyReleased(6, _fnPressed); } break;
//        case OdlaVK::STAFF12: { emit staffKeyReleased(7, _fnPressed); } break;
//        case OdlaVK::STAFF13: { emit staffKeyReleased(8, _fnPressed); } break;
//        case OdlaVK::STAFF14: { emit staffKeyReleased(9, _fnPressed); } break;
//        case OdlaVK::STAFF15: { emit staffKeyReleased(10, _fnPressed); } break;
//        case OdlaVK::STAFF16: { emit staffKeyReleased(11, _fnPressed); } break;
//        case OdlaVK::STAFF17: { emit staffKeyReleased(12, _fnPressed); } break;
//        case OdlaVK::STAFF18: { emit staffKeyReleased(13, _fnPressed); } break;
//        case OdlaVK::GOTO: { emit gotoKeyReleased(_fnPressed); } break;
//        case OdlaVK::PLAY: { emit playKeyReleased(_fnPressed); } break;
//        case OdlaVK::PAUSE: { emit pauseKeyReleased(_fnPressed); } break;
//        case OdlaVK::METRONOME: { emit metronomeKeyReleased(_fnPressed); } break;
//        case OdlaVK::PLUS: { emit plusKeyReleased(_fnPressed); } break;
//        case OdlaVK::MINUS: { emit minusKeyReleased(_fnPressed); } break;
//        case OdlaVK::SHARP: { emit sharpKeyReleased(_fnPressed); } break;
//        case OdlaVK::NATURAL: { emit naturalKeyReleased(_fnPressed); } break;
//        case OdlaVK::FLAT: { emit flatKeyReleased(_fnPressed); } break;
//        case OdlaVK::VOICES: { emit voicesKeyReleased(_fnPressed); } break;
//        case OdlaVK::INTERVAL: { emit intervalKeyReleased(_fnPressed); } break;
//        case OdlaVK::CHORD: { emit chordKeyReleased(_fnPressed); } break;
//        case OdlaVK::TUPLET: { emit tupletKeyReleased(_fnPressed); } break;
//        case OdlaVK::SLUR: { emit slurKeyReleased(_fnPressed); } break;
//        case OdlaVK::NUM0: { emit numKeyReleased(0, _fnPressed); } break;
//        case OdlaVK::NUM1: { emit numKeyReleased(1, _fnPressed); } break;
//        case OdlaVK::NUM2: { emit numKeyReleased(2, _fnPressed); } break;
//        case OdlaVK::NUM3: { emit numKeyReleased(3, _fnPressed); } break;
//        case OdlaVK::NUM4: { emit numKeyReleased(4, _fnPressed); } break;
//        case OdlaVK::NUM5: { emit numKeyReleased(5, _fnPressed); } break;
//        case OdlaVK::NUM6: { emit numKeyReleased(6, _fnPressed); } break;
//        case OdlaVK::NUM7: { emit numKeyReleased(7, _fnPressed); } break;
//        case OdlaVK::NUM8: { emit numKeyReleased(8, _fnPressed); } break;
//        case OdlaVK::NUM9: { emit numKeyReleased(9, _fnPressed); } break;
//        case OdlaVK::DOT: { emit dotKeyReleased(_fnPressed); } break;
//        case OdlaVK::UNDO: { emit undoKeyReleased(_fnPressed); } break;
//        case OdlaVK::REDO: { emit redoKeyReleased(_fnPressed); } break;
//        case OdlaVK::ENTER: { emit enterKeyReleased(_fnPressed); } break;
//        case OdlaVK::CANCEL: { emit cancelKeyReleased(_fnPressed); } break;
//        case OdlaVK::QUESTION: { emit questionKeyReleased(_fnPressed); } break;
//        case OdlaVK::HELP: { emit helpKeyReleased(_fnPressed); } break;
//    }
//}

///*!
// * \brief ODLAManager::deviceFound
// * \param path
// */
//void ODLAManager::deviceConnectionChanged(bool connected, QString path)
//{
////    if(connected)
////    {
////        //_deviceThread = new QThread(this);
////        _device->open(path);
////        _device->start();
////    }
////    else if(_device)
////    {
////        _device->quit();
////        _device->close();
////    }
//    qDebug() << path << (connected ? "connected" : "disconnected");
//    _controller->onKeyboardConnectionChanged(connected);
//}


///*!
// * \brief ODLAManager::deviceError
// */
//void ODLAManager::deviceError(QString message)
//{
//    qDebug() << message;
//}

///*!
// * \brief ODLAManager::startStaffKeyRepeat
// * \param k
// */
//void ODLAManager::startStaffKeyRepeat(int k)
//{
//    // single pression
//    emit staffKeyPressed(k, _fnPressed);

//    // remove old timer
//    stopStaffKeyRepeat();

//    // activate repeat
//    _staffTimerRepeat = connect(&_staffRepeatTimer, &QTimer::timeout, this, [this, k] () {
//        _staffRepeatTimer.setInterval(KEY_REPEAT_TIMEOUT);
//        emit staffKeyPressed(k, _fnPressed);
//    });

//    _staffRepeatTimer.start(KEY_START_REPEAT_TIMEOUT);
//}

///*!
// * \brief ODLAManager::stopStaffKeyRepeat
// * \param k
// */
//void ODLAManager::stopStaffKeyRepeat()
//{
//    _staffRepeatTimer.stop();
//    // remove connection
//    disconnect(_staffTimerRepeat);
//}

///*!
// * \brief ODLAManager::startArrowKeyRepeat
// */
//void ODLAManager::startArrowKeyRepeat(OdlaVK arrow)
//{
//    // single pression
//    switch (arrow)
//    {
//        case OdlaVK::UP: emit upKeyPressed(_fnPressed); break;
//        case OdlaVK::DOWN: emit downKeyPressed(_fnPressed); break;
//        case OdlaVK::LEFT: emit leftKeyPressed(_fnPressed); break;
//        case OdlaVK::RIGHT: emit rightKeyPressed(_fnPressed); break;
//        default: break;
//    }

//    // remove old timer
//    stopArrowKeyRepeat();

//    // activate repeat
//    _arrowTimerRepeat = connect(&_arrowRepeatTimer, &QTimer::timeout, this, [this, arrow] () {
//        _arrowRepeatTimer.setInterval(KEY_REPEAT_TIMEOUT);
//        switch (arrow)
//        {
//            case OdlaVK::UP: emit upKeyPressed(_fnPressed); break;
//            case OdlaVK::DOWN: emit downKeyPressed(_fnPressed); break;
//            case OdlaVK::LEFT: emit leftKeyPressed(_fnPressed); break;
//            case OdlaVK::RIGHT: emit rightKeyPressed(_fnPressed); break;
//            default: break;
//        }
//    });

//    _arrowRepeatTimer.start(KEY_START_REPEAT_TIMEOUT);
//}

///*!
// * \brief ODLAManager::stopArrowKeyRepeat
// */
//void ODLAManager::stopArrowKeyRepeat()
//{
//    _arrowRepeatTimer.stop();

//    // remove connection
//    disconnect(_arrowTimerRepeat);
//}

//ODLAControllerV2 *ODLAManager::getController() const
//{
//    return _controller;
//}

//void ODLAManager::setOldController(ODLAControllerV2 *value)
//{
//    _controller = value;

//    if (_controller)
//    {
//        connect(this, &ODLAManager::menuKeyPressed, _controller, &ODLAControllerV2::onMenuKeyPressed);
//        connect(this, &ODLAManager::clefKeyPressed, _controller, &ODLAControllerV2::onClefKeyPressed);
//        connect(this, &ODLAManager::timeSigKeyPressed, _controller, &ODLAControllerV2::onTimeSignatureKeyPressed);
//        connect(this, &ODLAManager::keySigKeyPressed, _controller, &ODLAControllerV2::onKeySignatureKeyPressed);
//        connect(this, &ODLAManager::barsKeyPressed, _controller, &ODLAControllerV2::onBarKeyPressed);
//        connect(this, &ODLAManager::optionsKeyPressed, _controller, &ODLAControllerV2::onOptionsKeyPressed);
//        connect(this, &ODLAManager::copyKeyPressed, _controller, &ODLAControllerV2::onCopyKeyPressed);
//        connect(this, &ODLAManager::pasteKeyPressed, _controller, &ODLAControllerV2::onPasteKeyPressed);
//        connect(this, &ODLAManager::selectKeyPressed, _controller, &ODLAControllerV2::onSelectKeyPressed);
//        connect(this, &ODLAManager::selectKeyReleased, _controller, &ODLAControllerV2::onSelectKeyReleased);
//        connect(this, &ODLAManager::staffKeyPressed, _controller, &ODLAControllerV2::onStaffKeyPressed);
//        connect(this, &ODLAManager::gotoKeyPressed, _controller, &ODLAControllerV2::onGotoKeyPressed);
//        connect(this, &ODLAManager::playKeyPressed, _controller, &ODLAControllerV2::onPlayKeyPressed);
//        connect(this, &ODLAManager::pauseKeyPressed, _controller, &ODLAControllerV2::onPauseKeyPressed);
//        connect(this, &ODLAManager::metronomeKeyPressed, _controller, &ODLAControllerV2::onMetronomeKeyPressed);
//        connect(this, &ODLAManager::plusKeyPressed, _controller, &ODLAControllerV2::onPlusKeyPressed);
//        connect(this, &ODLAManager::minusKeyPressed, _controller, &ODLAControllerV2::onMinusKeyPressed);
//        connect(this, &ODLAManager::sharpKeyPressed, _controller, &ODLAControllerV2::onSharpKeyPressed);
//        connect(this, &ODLAManager::naturalKeyPressed, _controller, &ODLAControllerV2::onNaturalKeyPressed);
//        connect(this, &ODLAManager::flatKeyPressed, _controller, &ODLAControllerV2::onFlatKeyPressed);
//        connect(this, &ODLAManager::voicesKeyPressed, _controller, &ODLAControllerV2::onVoiceKeyPressed);
//        connect(this, &ODLAManager::intervalKeyPressed, _controller, &ODLAControllerV2::onIntervalKeyPressed);
//        connect(this, &ODLAManager::chordKeyPressed, _controller, &ODLAControllerV2::onChordKeyPressed);
//        connect(this, &ODLAManager::chordKeyReleased, _controller, &ODLAControllerV2::onChordKeyReleased);
//        connect(this, &ODLAManager::tupletKeyPressed, _controller, &ODLAControllerV2::onTupletKeyPressed);
//        connect(this, &ODLAManager::slurKeyPressed, _controller, &ODLAControllerV2::onSlurKeyPressed);
//        connect(this, &ODLAManager::slurKeyReleased, _controller, &ODLAControllerV2::onSlurKeyReleased);
//        connect(this, &ODLAManager::numKeyPressed, _controller, &ODLAControllerV2::onNumberKeyPressed);
//        connect(this, &ODLAManager::dotKeyPressed, _controller, &ODLAControllerV2::onDotKeyPressed);
//        connect(this, &ODLAManager::undoKeyPressed, _controller, &ODLAControllerV2::onUndoKeyPressed);
//        connect(this, &ODLAManager::redoKeyPressed, _controller, &ODLAControllerV2::onRedoKeyPressed);
//        connect(this, &ODLAManager::enterKeyPressed, _controller, &ODLAControllerV2::onEnterKeyPressed);
//        connect(this, &ODLAManager::cancelKeyPressed, _controller, &ODLAControllerV2::onCancelKeyPressed);
//        connect(this, &ODLAManager::upKeyPressed, _controller, &ODLAControllerV2::onArrowUpKeyPressed);
//        connect(this, &ODLAManager::downKeyPressed, _controller, &ODLAControllerV2::onArrowDownKeyPressed);
//        connect(this, &ODLAManager::leftKeyPressed, _controller, &ODLAControllerV2::onArrowLeftKeyPressed);
//        connect(this, &ODLAManager::rightKeyPressed, _controller, &ODLAControllerV2::onArrowRightKeyPressed);
//        connect(this, &ODLAManager::questionKeyPressed, _controller, &ODLAControllerV2::onQuestionKeyPressed);
//        connect(this, &ODLAManager::helpKeyPressed, _controller, &ODLAControllerV2::onHelpKeyPressed);
//    }
//}

//void ODLAManager::setNewDialog(MenuDialog *value)
//{
//    _menuDialog = value;

//    if (_menuDialog)
//    {
//        connect(this, &ODLAManager::numKeyPressed, _menuDialog, &MenuDialog::setPanelTopLeft);
//        connect(this, &ODLAManager::dotKeyPressed, _menuDialog, &MenuDialog::onDotKeyPressed);
//        connect(this, &ODLAManager::undoKeyPressed, _menuDialog, &MenuDialog::onUndoKeyPressed);
//        connect(this, &ODLAManager::redoKeyPressed, _menuDialog, &MenuDialog::onRedoKeyPressed);
//        connect(this, &ODLAManager::enterKeyPressed, _menuDialog, &MenuDialog::onEnterKeyPressed);
//        connect(this, &ODLAManager::cancelKeyPressed, _menuDialog, &MenuDialog::onCancelKeyPressed);
//        connect(this, &ODLAManager::upKeyPressed, _menuDialog, &MenuDialog::onArrowUpKeyPressed);
//        connect(this, &ODLAManager::downKeyPressed, _menuDialog, &MenuDialog::onArrowDownKeyPressed);
//        connect(this, &ODLAManager::leftKeyPressed, _menuDialog, &MenuDialog::onArrowLeftKeyPressed);
//        connect(this, &ODLAManager::rightKeyPressed, _menuDialog, &MenuDialog::onArrowRightKeyPressed);
//        connect(this, &ODLAManager::questionKeyPressed, _menuDialog, &MenuDialog::onQuestionKeyPressed);
//        connect(this, &ODLAManager::helpKeyPressed, _menuDialog, &MenuDialog::onHelpKeyPressed);
//        connect(this, &ODLAManager::plusKeyPressed, _menuDialog, &MenuDialog::onPlusKeyPressed);
//        connect(this, &ODLAManager::minusKeyPressed, _menuDialog, &MenuDialog::onMinusKeyPressed);
//    }
//}

