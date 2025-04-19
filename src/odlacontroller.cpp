#include "odlacontroller.h"
#include "musescore.h"
#include "database.h"
#include <QInputDialog>
#include <QMessageBox>
#include "voiceover.h"

ODLAControllerV2::ODLAControllerV2(QObject *parent) : QObject(parent)
{
    _scoreApp = nullptr;
    _activeModifier = ModifiersV2::NONE;
    _chordEditStarted = false;
    _metronomeActive = false;

    _currentOptionPage = 0;
    _textInputEditing = false;

    _defaultExtendedFeatureSet = Database::instance()->getDefaultExtendedFeatureSet();
    _selectedFeatureSet = ExtendedFeatures::None;

    _measuresToAdd = 0;
    _gotoDestination = 0;
    _intervalsSign = IntervalSigns::UNVALID;
    _tempoMode = Tempo::NONE;

    _tupletValue = -1;

    _timeSigNumCompleted = false;
    _timeSigDenCompleted = false;
    _timeSignatureEditing = false;

    _selectionMeasureFromCompleted = false;
    _selectionMeasureToCompleted = false;
    _selectionMeasureRangeEditing = false;

    _numerator = 4;
    _denominator = 4;

    _selectMeasureFrom = 1;
    _selectMeasureTo = 1;

    _keySigAccidental = Accidentals::NONE;
    _keySigCount = 0;

    _currentAugmentationDots = Augmentations::NONE;
    _currentNoteDuration = Database::instance()->getDefaultNoteValue();

    _flatKeyMultiplicity = 1;
    _naturalKeyToRemove = false;
    _sharpKeyMultiplicity = 1;

    // init clef key maps
    _clefKeyMap.insert(1, Clefs::TREBLE);
    _clefKeyMap.insert(2, Clefs::TREBLE_8va_ALTA);
    _clefKeyMap.insert(3, Clefs::TREBLE_15ma_ALTA);
    _clefKeyMap.insert(4, Clefs::TREBLE_8va_BASSA);
    _clefKeyMap.insert(5, Clefs::TREBLE_15ma_BASSA);
    _clefKeyMap.insert(6, Clefs::SOPRANO);
    _clefKeyMap.insert(7, Clefs::MEZZO_SOPRANO);
    _clefKeyMap.insert(8, Clefs::ALTO);
    _clefKeyMap.insert(9, Clefs::TENOR);
    _clefKeyMap.insert(10, Clefs::BARITONE);
    _clefKeyMap.insert(11, Clefs::BASS);
    _clefKeyMap.insert(12, Clefs::BASS_8va_ALTA);
    _clefKeyMap.insert(13, Clefs::BASS_15ma_ALTA);
    _clefKeyMap.insert(14, Clefs::BASS_8va_BASSA);
    _clefKeyMap.insert(15, Clefs::BASS_15ma_BASSA);

    // init duration key maps
    _durationKeyMap.insert(1, Durations::N64TH);
    _durationKeyMap.insert(2, Durations::N32ND);
    _durationKeyMap.insert(3, Durations::N16TH);
    _durationKeyMap.insert(4, Durations::EIGTH);
    _durationKeyMap.insert(5, Durations::QUARTER);
    _durationKeyMap.insert(6, Durations::HALF);
    _durationKeyMap.insert(7, Durations::WHOLE);
    _durationKeyMap.insert(8, Durations::DOUBLE_WHOLE);

    _lastKeyPressed = static_cast<OdlaVK>(255);
    _lastKeyReleased= static_cast<OdlaVK>(255);
    _prevKeyPressed = static_cast<OdlaVK>(255);
    _prevKeyReleased= static_cast<OdlaVK>(255);

    // reset
    connect(this, &ODLAControllerV2::inputSequenceAborted, this, &ODLAControllerV2::resetInternalStateVariables);
    connect(this, &ODLAControllerV2::inputSequenceCompleted, this, &ODLAControllerV2::resetInternalStateVariables);

    _scoreApp = Musescore::instance();
}

/*!
 * \brief ODLAControllerV2::scoreApp
 * \return
 */
Musescore* ODLAControllerV2::scoreApp() const
{
    return _scoreApp;
}

///*!
// * \brief ODLAController::initScoreApp
// * \param connected
// */
//void ODLAControllerV2::initScoreApp(bool connected)
//{
//    qDebug() << "initScoreApp, connected?" << connected;
//    if(connected)
//        _scoreApp->init();
//}

/*!
 * \brief ODLAControllerV2::activeModifier
 * \return
 */
ModifiersV2 ODLAControllerV2::activeModifier() const
{
    return _activeModifier;
}

/*!
 * \brief ODLAControllerV2::defaultExtendedFeatureSet
 * \return
 */
ExtendedFeatures ODLAControllerV2::defaultExtendedFeatureSet() const
{
    return _defaultExtendedFeatureSet;
}

/*!
 * \brief ODLAControllerV2::setDefaultExtendedFeatureSet
 * \param defaultExtendedFeatureSet
 */
void ODLAControllerV2::setDefaultExtendedFeatureSet(const ExtendedFeatures& defaultExtendedFeatureSet)
{
    _defaultExtendedFeatureSet = defaultExtendedFeatureSet;
}

/*!
 * \brief ODLAControllerV2::setCurrentSelectedKeyIdx
 * \param newKeyIdx
 */
void ODLAControllerV2::setCurrentSelectedKeyIdx(int newKeyIdx)
{
    _currentOptionSelected = newKeyIdx;
}

/*!
 * \brief ODLAControllerV2::setTempo
 */
// this soluction is temporary, bad practice
void ODLAControllerV2::setTempo()
{
    _scoreApp->insertTempo(_tempoMode, _tempoNumber);
    emit inputSequenceCompleted(ModifiersV2::TEMPO);
}

/*!
 * \brief ODLAControllerV2::setLastKeyPressed
 * \param pressed
 */
void ODLAControllerV2::setLastKeyPressed(uint8_t pressed)
{
    _prevKeyPressed = _lastKeyPressed;
    _lastKeyPressed = static_cast<OdlaVK>(pressed);
}

/*!
 * \brief ODLAControllerV2::setLastKeyReleased
 * \param released
 */
void ODLAControllerV2::setLastKeyReleased(uint8_t released)
{
    _prevKeyReleased = _lastKeyReleased;
    _lastKeyReleased  = static_cast<OdlaVK>(released);
}

void ODLAControllerV2::onKeyboardConnectionChanged(bool connected)
{
    _scoreApp->onKeyboardConnectionChanged(connected);
}

/*!
 * \brief ODLAControllerV2::onOptionPageChanged
 * \param page
 */
void ODLAControllerV2::onOptionPageChanged(int page)
{
    _currentOptionPage = page;
}

/*!
 * \brief ODLAControllerV2::onRatioInputChanged
 * \param numerator
 * \param denominator
 */
void ODLAControllerV2::onRatioInputChanged(int upper_number, int lower_number)
{
    if (_activeModifier == ModifiersV2::TIMESIG)
    {
        _numerator = upper_number;
        _denominator = lower_number;
    }
    else if(_activeModifier == ModifiersV2::SELECT_MEASURES)
    {
        _selectMeasureFrom = upper_number;
        _selectMeasureTo = lower_number;
    }
}

/*!
 * \brief ODLAControllerV2::onTextInputChanged
 * \param text
 */
void ODLAControllerV2::onTextInputChanged(QString text)
{
    switch (_activeModifier)
    {
        case ModifiersV2::GOTO:
        {
            bool ok = false;
            int num = text.toInt(&ok);
            if (ok)
                _gotoDestination = num;
        } break;

        case ModifiersV2::ADD_MEASURES:
        {
            bool ok = false;
            int num = text.toInt(&ok);
            if (ok)
                _measuresToAdd = num;
        } break;

        case ModifiersV2::OPTIONS:
        {
            if (_selectedFeatureSet == ExtendedFeatures::Tempo)
            {
                bool ok = false;
                int num = text.toInt(&ok);
                if (ok)
                    _tempoNumber = num;
            }
        } break;

        case ModifiersV2::TEMPO:
        {
            bool ok = false;
            int num = text.toInt(&ok);
            if (ok)
                _tempoNumber = num;

        } break;

        default:
            break;
    }
}

/*!
 * \brief ODLAControllerV2::onMenuKeyPressed
 */
void ODLAControllerV2::onMenuKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::MENU;
    emit inputSequenceActivated(ModifiersV2::MENU);
}

/*!
 * \brief ODLAControllerV2::onClefKeyPressed
 */
void ODLAControllerV2::onClefKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::CLEF;
    emit inputSequenceActivated(ModifiersV2::CLEF);
}

/*!
 * \brief ODLAControllerV2::onTimeSignatureKeyPressed
 */
void ODLAControllerV2::onTimeSignatureKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::TIMESIG;

    // set default values
    _timeSigNumCompleted = false;
    _numerator = 4;
    _denominator = 4;

    emit inputSequenceActivated(ModifiersV2::TIMESIG);
}

/*!
 * \brief ODLAControllerV2::onKeySignatureKeyPressed
 */
void ODLAControllerV2::onKeySignatureKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::KEYSIG;

    _keySigAccidental = Accidentals::NONE;
    _keySigCount = 0;

    emit inputSequenceActivated(ModifiersV2::KEYSIG);
}

/*!
 * \brief ODLAControllerV2::onBarKeyPressed
 */
void ODLAControllerV2::onBarKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::BAR;
    emit inputSequenceActivated(ModifiersV2::BAR);
}

/*!
 * \brief ODLAControllerV2::onOptionsKeyPressed
 */
void ODLAControllerV2::onOptionsKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::OPTIONS;
    _selectedFeatureSet = ExtendedFeatures::None;

    if (fn && (_defaultExtendedFeatureSet != ExtendedFeatures::None))
    {
        _selectedFeatureSet = _defaultExtendedFeatureSet;
        emit inputSequenceActivated(ModifiersV2::PREFERRED_OPTION);
    }
    else
    {
       emit inputSequenceActivated(ModifiersV2::OPTIONS);
    }
}

/*!
 * \brief ODLAControllerV2::onCopyKeyPressed
 */
void ODLAControllerV2::onCopyKeyPressed(bool fn)
{
    if (_scoreApp)
        _scoreApp->copySelection();
}

/*!
 * \brief ODLAControllerV2::onPasteKeyPressed
 */
void ODLAControllerV2::onPasteKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (fn)
            _scoreApp->repeatSelection();
        else
            _scoreApp->pasteSelection();
    }
}

/*!
 * \brief ODLAControllerV2::onSelectKeyPressed
 */
void ODLAControllerV2::onSelectKeyPressed()
{
    _activeModifier = ModifiersV2::SELECT;
}

/*!
 * \brief ODLAControllerV2::onSelectKeyReleased
 */
void ODLAControllerV2::onSelectKeyReleased()
{
    if (_activeModifier == ModifiersV2::SELECT)
        _activeModifier =  ModifiersV2::NONE;
}

/*!
 * \brief ODLAControllerV2::onStaffKeyPressed
 * \param line
 */
void ODLAControllerV2::onStaffKeyPressed(int line, bool fn)
{
    if (_scoreApp)
        _scoreApp->addNoteToStaffLine(line,
                                      (_activeModifier == ModifiersV2::CHORD) /*&& _chordEditStarted*/,
                                      (_activeModifier == ModifiersV2::SLUR));

    _chordEditStarted = false;
}

/*!
 * \brief ODLAControllerV2::onPlusKeyPressed
 */
void ODLAControllerV2::onPlusKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::INTERVALS)
        {
            // used to select the type of interval ('+' means Above)
            _intervalsSign = IntervalSigns::ABOVE;
            emit inputSequenceProgress(ModifiersV2::INTERVALS, int(_intervalsSign));
        }
        else if (_activeModifier == ModifiersV2::NONE)
        {
            if (fn)
                _scoreApp->pitchUpOctave();
            else
                _scoreApp->pitchUpDiatonic();

            emit inputSequenceCompleted(ModifiersV2::NONE);
        }
    }
}

/*!
 * \brief ODLAControllerV2::onMinusKeyPressed
 */
void ODLAControllerV2::onMinusKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::INTERVALS)
        {
            // used to select the type of interval ('-' means Below)
            _intervalsSign = IntervalSigns::BELOW;
            emit inputSequenceProgress(ModifiersV2::INTERVALS, int(_intervalsSign));
        }
        else if (_activeModifier == ModifiersV2::NONE)
        {
            if(fn)
                _scoreApp->pitchDownOctave();
            else
                _scoreApp->pitchDownDiatonic();

            emit inputSequenceCompleted(ModifiersV2::NONE);
        }
    }
}

/*!
 * \brief ODLAControllerV2::onFlatKeyPressed
 */
void ODLAControllerV2::onFlatKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::KEYSIG)
        {
            if (_keySigAccidental == Accidentals::NONE)
            {
                _keySigAccidental = Accidentals::FLAT;

                // go on to get the accidental count
                emit inputSequenceProgress(ModifiersV2::KEYSIG, static_cast<int>(Accidentals::FLAT));
            }
        }
        else if (_activeModifier == ModifiersV2::NONE ||
                 _activeModifier == ModifiersV2::CHORD ||
                 _activeModifier == ModifiersV2::SLUR)
        {
            bool brackets = fn;

            if(_prevKeyPressed != OdlaVK::FLAT)
                _flatKeyMultiplicity = 1;

            switch (_flatKeyMultiplicity)
            {
                case 1:
                    _scoreApp->setAccidental(Accidentals::FLAT, brackets);
                    break;
                case 2:
                    _scoreApp->setAccidental(Accidentals::DOUBLE_FLAT, brackets);
                    break;
                default:
                    _scoreApp->setAccidental(Accidentals::NONE);
            }
            _flatKeyMultiplicity = (_flatKeyMultiplicity + 1) % 3;
        }
    }
}

/*!
 * \brief ODLAControllerV2::onNaturalKeyPressed
 */
void ODLAControllerV2::onNaturalKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::KEYSIG)
        {
            if (_keySigAccidental == Accidentals::NONE)
            {
                _keySigAccidental = Accidentals::NATURAL;

                // insert keysig
                _scoreApp->insertKeySignature(Accidentals::NATURAL, 0);

                emit inputSequenceCompleted(ModifiersV2::KEYSIG);
            }
        }
        else if (_activeModifier == ModifiersV2::NONE ||
                 _activeModifier == ModifiersV2::CHORD ||
                 _activeModifier == ModifiersV2::SLUR)
        {
            bool brackets = fn;

            if(_prevKeyPressed != OdlaVK::NATURAL)
                _naturalKeyToRemove = false;

            if(_naturalKeyToRemove)
                _scoreApp->setAccidental(Accidentals::NONE);
            else
                _scoreApp->setAccidental(Accidentals::NATURAL, brackets);

            _naturalKeyToRemove = !_naturalKeyToRemove;
        }
    }
}

/*!
 * \brief ODLAControllerV2::onSharpKeyPressed
 */
void ODLAControllerV2::onSharpKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::KEYSIG)
        {
            if (_keySigAccidental == Accidentals::NONE)
            {
                _keySigAccidental = Accidentals::SHARP;

                // go on to get the accidental count
                emit inputSequenceProgress(ModifiersV2::KEYSIG, static_cast<int>(Accidentals::SHARP));
            }
        }
        else if (_activeModifier == ModifiersV2::NONE ||
                 _activeModifier == ModifiersV2::CHORD ||
                 _activeModifier == ModifiersV2::SLUR)
        {
            bool brackets = fn;

            if(_prevKeyPressed != OdlaVK::SHARP)
                _sharpKeyMultiplicity = 1;

            switch (_sharpKeyMultiplicity) {
                case 1:
                    _scoreApp->setAccidental(Accidentals::SHARP, brackets);
                    break;
                case 2:
                    _scoreApp->setAccidental(Accidentals::DOUBLE_SHARP, brackets);
                    break;
                default:
                    _scoreApp->setAccidental(Accidentals::NONE);
            }

            _sharpKeyMultiplicity = (_sharpKeyMultiplicity % 3) + 1;
        }
    }
}

/*!
 * \brief ODLAControllerV2::onGotoKeyPressed
 */
void ODLAControllerV2::onGotoKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::GOTO;
    _gotoDestination = 0;
    if(_scoreApp && fn)
    {
        _scoreApp->gotoEnd();
        emit inputSequenceCompleted(ModifiersV2::GOTO);
    }
    else
        emit inputSequenceActivated(ModifiersV2::GOTO);
}

/*!
 * \brief ODLAControllerV2::onPlayKeyPressed
 */
void ODLAControllerV2::onPlayKeyPressed(bool fn)
{
    if(fn)
        _scoreApp->showMixer();
    else if (_scoreApp && _activeModifier == ModifiersV2::NONE)
    {
        if(_scoreApp->isPlaying())
            _scoreApp->stopPlayback();
        else
            _scoreApp->startPlayback();
    }
}

/*!
 * \brief ODLAControllerV2::onPauseKeyPressed
 */
void ODLAControllerV2::onPauseKeyPressed(bool fn)
{
    if (_scoreApp && _activeModifier == ModifiersV2::NONE)
    {
        if(_scoreApp->isPlaying())
            _scoreApp->pausePlayback();
        else
            _scoreApp->resumePlayback();
    }
}

/*!
 * \brief ODLAControllerV2::onMetronomeKeyPressed
 */
void ODLAControllerV2::onMetronomeKeyPressed(bool fn)
{
    if(fn)
    {
        _activeModifier = ModifiersV2::TEMPO;
        _selectedFeatureSet = ExtendedFeatures::Tempo;
        emit inputSequenceActivated(ModifiersV2::TEMPO);
    }
    if (_scoreApp && _activeModifier == ModifiersV2::NONE)
    {
        _metronomeActive = !_metronomeActive;
        _scoreApp->toggleMetronome(_metronomeActive);
    }
}

/*!
 * \brief ODLAControllerV2::onVoiceKeyPressed
 */
void ODLAControllerV2::onVoiceKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::VOICE;
    emit inputSequenceActivated(ModifiersV2::VOICE);
}

/*!
 * \brief ODLAControllerV2::onIntervalKeyPressed
 */
void ODLAControllerV2::onIntervalKeyPressed(bool fn)
{
    if(fn)
    {
        _scoreApp->transpose();
    }
    else
    {
        _activeModifier = ModifiersV2::INTERVALS;
        _intervalsSign = IntervalSigns::UNVALID;
        emit inputSequenceActivated(ModifiersV2::INTERVALS);
    }
}

/*!
 * \brief ODLAControllerV2::onChordKeyPressed
 * \param pressed
 */
void ODLAControllerV2::onChordKeyPressed(bool fn)
{
    emit inputSequenceCompleted(ModifiersV2::NONE);
    _activeModifier = ModifiersV2::CHORD;
    _chordEditStarted = true;
}

/*!
 * \brief ODLAControllerV2::onChordKeyReleased
 */
void ODLAControllerV2::onChordKeyReleased(bool fn)
{
    _chordEditStarted = false;
    if (_activeModifier == ModifiersV2::CHORD)
    {
        _activeModifier =  ModifiersV2::NONE;

        // move cursor forward if chord key is released
        //onArrowRightKeyPressed();
    }
}

/*!
 * \brief ODLAControllerV2::onTupletKeyPressed
 */
void ODLAControllerV2::onTupletKeyPressed(bool fn)
{
    _activeModifier = ModifiersV2::TUPLET;
    _tupletValue = -1;
    //_tupletDuration = Durations::QA;
    emit inputSequenceActivated(ModifiersV2::TUPLET);
}

/*!
 * \brief ODLAControllerV2::onSlurKeyPressed
 * \param pressed
 */
void ODLAControllerV2::onSlurKeyPressed(bool fn)
{    
    // Fn + Slur key add tie
    if (fn && _scoreApp)
        _scoreApp->addTie();
    else
        _scoreApp->insertLine(Lines::SLUR);


    emit inputSequenceCompleted(ModifiersV2::NONE);
    _activeModifier = ModifiersV2::SLUR;
}

/*!
 * \brief ODLAControllerV2::onSlurKeyReleased
 */
void ODLAControllerV2::onSlurKeyReleased(bool fn)
{
    if (_activeModifier == ModifiersV2::SLUR)
        _activeModifier =  ModifiersV2::NONE;
}

/*!
 * \brief ODLAControllerV2::onNumberKeyPressed
 * \param digit
 */
void ODLAControllerV2::onNumberKeyPressed(int digit, bool fn)
{
    if (!_scoreApp)
        return;
    emit inputDigitReady(digit);

    switch (_activeModifier)
    {
        // change note values, i.e. duration
        case ModifiersV2::NONE:
        {
            if (digit == 0)
                _scoreApp->addRest(Durations::NONE);
            else
            {
                if (digit > 0 && digit <= _durationKeyMap.size())
                {
                    _currentAugmentationDots = Augmentations::NONE;
                    Durations d = _durationKeyMap.value(digit, Durations::NONE);
                    if (d != Durations::NONE)
                    {
                        _scoreApp->setNoteValue(d);

                        // set current duration
                        _currentNoteDuration = d;
                    }
                }
            }
        }
            break;

        case ModifiersV2::MENU:
           evaluateMenuOption(digit);

           break;
        case ModifiersV2::FILE:
           evaluateFileSubMenuOption(digit);
           break;

        case ModifiersV2::EDIT:
           evaluateEditSubMenuOption(digit);
           break;

        case ModifiersV2::GOTO:
            evaluateGotoOption(digit);
            break;

        case ModifiersV2::ADD_MEASURES:
            evaluateAddMeasureOption(digit);
            break;

        case ModifiersV2::CLEF:
            evaluateClefOption(digit);
            break;

        case ModifiersV2::VOICE:
            evaluateVoiceOption(digit);
            break;

        case ModifiersV2::BAR:
            evaluateBarOption(digit);
            break;

        case ModifiersV2::INTERVALS:
            evaluateIntervalOption(digit);
            break;

        case ModifiersV2::TUPLET:
            evaluateTupletOption(digit);
            break;

        case ModifiersV2::TIMESIG:
            if (!_timeSignatureEditing)
            {
                evaluateTimeSigOption(digit);
            }
            else if (!_timeSigNumCompleted)
            {
                emit inputSequenceProgress(ModifiersV2::TIMESIG, -2); // edit numerator
            }
            else if (!_timeSigDenCompleted)
            {
                emit inputSequenceProgress(ModifiersV2::TIMESIG, -3); // edit denominator
            }
            break;

        case ModifiersV2::KEYSIG:
            evaluateKeySigOption(digit);
            break;

        case ModifiersV2::OPTIONS:
            evaluateExtentedOption(digit);
            break;

        case ModifiersV2::TEMPO:
            evaluateTempoShortCut(digit);
            break;

        case ModifiersV2::SELECT_MEASURES:
            if (!_selectionMeasureRangeEditing)
            {
                evaluateSelectionMenu(digit);
            }
            else if (!_selectionMeasureFromCompleted)
            {
                emit inputSequenceProgress(ModifiersV2::SELECT_MEASURES, -2); // edit measure from
            }
            else if (!_selectionMeasureToCompleted)
            {
                emit inputSequenceProgress(ModifiersV2::SELECT_MEASURES, -3); // edit measure to
            }
            break;

        default:
            break;
    }
}

/*!
 * \brief ODLAControllerV2::onDotKeyPressed
 */
void ODLAControllerV2::onDotKeyPressed(bool fn)
{
    if (_scoreApp && _activeModifier == ModifiersV2::NONE)
    {
        switch (_currentAugmentationDots)
        {
            case Augmentations::NONE:
                _currentAugmentationDots = Augmentations::DOTTED;
            break;

            case Augmentations::DOTTED:
                _currentAugmentationDots = Augmentations::DOUBLE_DOTTED;
            break;

            case Augmentations::DOUBLE_DOTTED:
                _currentAugmentationDots = Augmentations::TRIPLE_DOTTED;
            break;

            case Augmentations::TRIPLE_DOTTED:
                _currentAugmentationDots = Augmentations::NONE;
            break;
        }

    }
    _scoreApp->setAugmentationDots(_currentNoteDuration, _currentAugmentationDots);
}

/*!
 * \brief ODLAControllerV2::onQuestionKeyPressed
 */
void ODLAControllerV2::onQuestionKeyPressed(bool fn)
{
    if (_scoreApp && _activeModifier == ModifiersV2::NONE)
        _scoreApp->getStatus();
}

/*!
 * \brief ODLAControllerV2::onHelpKeyPressed
 */
void ODLAControllerV2::onHelpKeyPressed(bool fn)
{
    if(fn) return;
<<<<<<< HEAD:src/odlacontroller.cpp
    VoiceOver::instance()->toggleActivation();
=======
    VocalSynt::toggleActivation();
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/odlacontroller.cpp
}

/*!
 * \brief ODLAControllerV2::onUndoKeyPressed
 */
void ODLAControllerV2::onUndoKeyPressed(bool fn)
{
    if (_scoreApp && _activeModifier == ModifiersV2::NONE)
        _scoreApp->undo();
}

/*!
 * \brief ODLAControllerV2::onRedoKeyPressed
 */
void ODLAControllerV2::onRedoKeyPressed(bool fn)
{
    if (_scoreApp && _activeModifier == ModifiersV2::NONE)
        _scoreApp->redo();
}

/*!
 * \brief ODLAControllerV2::onEnterKeyPressed
 */
void ODLAControllerV2::onEnterKeyPressed(bool fn)
{    
    switch(_activeModifier)
    {
        case ModifiersV2::TIMESIG:

            if (_timeSignatureEditing && !_timeSigNumCompleted && _currentOptionSelected == 0) // todo: fix currentOptionSelected
            {
                // enter key pressed to end numerator input
                emit inputSequenceProgress(ModifiersV2::TIMESIG, -3);
                _timeSigNumCompleted = true;
                _timeSignatureEditing = true;
            }
            else if (_timeSignatureEditing && _timeSigNumCompleted && !_timeSigDenCompleted && _currentOptionSelected == 0)
            {
               // enter key pressed to end denominator input
               _timeSigDenCompleted = true;
               _timeSignatureEditing = false;

               QList<int> denValues;
               denValues << 2 << 4 << 8 << 16 << 32 << 64;
               if (denValues.indexOf(_denominator) != -1)
               {
                    _scoreApp->insertTimeSignature(_numerator, _denominator);
               }
               emit inputSequenceCompleted(ModifiersV2::TIMESIG);
            }
            else
                emit enterEvent(ModifiersV2::TIMESIG, fn);
            break;

        case ModifiersV2::GOTO:
            if(_gotoDestination)
                _scoreApp->gotoMeasure(_gotoDestination);
            else
                _scoreApp->gotoBeginning();
            emit inputSequenceCompleted(ModifiersV2::GOTO);
            break;


        case ModifiersV2::ADD_MEASURES:
            _scoreApp->insertMeasures(_measuresToAdd);
            emit inputSequenceCompleted(ModifiersV2::ADD_MEASURES);
            break;

        case ModifiersV2::OPTIONS:
            switch (_selectedFeatureSet)
            {

                case ExtendedFeatures::Transpose:
                    _scoreApp->transpose();
                    emit inputSequenceCompleted(ModifiersV2::OPTIONS);
                    break;

                default:
                    emit enterEvent(ModifiersV2::OPTIONS, fn);
                    break;
            }
            break;

        case ModifiersV2::SELECT:
            if(fn)
            {
                emit inputSequenceActivated(ModifiersV2::SELECT_MEASURES);
                _activeModifier = ModifiersV2::SELECT_MEASURES;
            }
            break;

       case ModifiersV2::SELECT_MEASURES:
            if (_selectionMeasureRangeEditing && !_selectionMeasureFromCompleted && _currentOptionSelected == 0)
            {
                // enter key pressed to end numerator input
                emit inputSequenceProgress(ModifiersV2::SELECT_MEASURES, -3);
                _selectionMeasureFromCompleted = true;
                _selectionMeasureRangeEditing = true;
            }
            else if (_selectionMeasureRangeEditing && _selectionMeasureFromCompleted && !_selectionMeasureToCompleted &&  _currentOptionSelected == 0)
            {
               // enter key pressed to end denominator input
               _selectionMeasureRangeEditing = false;
               _selectionMeasureToCompleted = true;

               _scoreApp->selectRange(_selectMeasureFrom, _selectMeasureTo);
               emit inputSequenceCompleted(ModifiersV2::SELECT_MEASURES);
            }
            else
               emit enterEvent(ModifiersV2::SELECT_MEASURES, fn);
            break;

    case ModifiersV2::NONE:
        return;


        default:
            emit enterEvent(_activeModifier, fn);
            break;

    }
}
/*!
 * \brief ODLAControllerV2::silentHide
 */
void ODLAControllerV2::silentHide()
{
    _activeModifier = ModifiersV2::NONE;
}

/*!
 * \brief ODLAControllerV2::onCancelKeyPressed
 */
void ODLAControllerV2::onCancelKeyPressed(bool fn)
{
    if (_activeModifier == ModifiersV2::NONE)
    {
        if(fn)
            _scoreApp->deleteSelection();

        else
            _scoreApp->emptySelection();
    }

    _activeModifier = ModifiersV2::NONE;

    emit inputSequenceCompleted(ModifiersV2::NONE);
}

/*!
 * \brief ODLAControllerV2::onArrowUpKeyPressed
 */
void ODLAControllerV2::onArrowUpKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::SELECT)
        {
            _scoreApp->selectStaffAbove();
        }
        else if (_activeModifier == ModifiersV2::NONE)
        {
            if (fn)
                _scoreApp->moveToPrevTrack();
            else
                _scoreApp->moveToUpperNote();
        }
        else
            if (fn)
                emit inputMoveSelection(direction_t::FN_UP);
            else
                emit inputMoveSelection(direction_t::PREV_ROW);
    }
}

/*!
 * \brief ODLAControllerV2::onArrowDownKeyPressed
 */
void ODLAControllerV2::onArrowDownKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::SELECT)
        {
            _scoreApp->selectStaffBelow();
        }
        else if (_activeModifier == ModifiersV2::NONE)
        {
            if (fn)
                _scoreApp->moveToNextTrack();
            else
                _scoreApp->moveToLowerNote();
        }
        else
            if (fn)
                emit inputMoveSelection(direction_t::FN_DOWN);
            else
                emit inputMoveSelection(direction_t::NEXT_ROW);
    }
}

/*!
 * \brief ODLAControllerV2::onArrowLeftKeyPressed
 */
void ODLAControllerV2::onArrowLeftKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::SELECT)
        {
            if (fn)
                _scoreApp->selectPrevMeasure();
            else
                _scoreApp->selectPrevChord();
        }
        else if (_activeModifier == ModifiersV2::NONE)
        {
            if (fn)
                _scoreApp->moveToPrevMeasure();
            else
                _scoreApp->moveToPrevChord();
        }
        else
            if (fn)
                emit inputMoveSelection(direction_t::PREV_PAGE);
            else
                emit inputMoveSelection(direction_t::PREV_ELEMENT);
    }
}

/*!
 * \brief ODLAControllerV2::onArrowRightKeyPressed
 */
void ODLAControllerV2::onArrowRightKeyPressed(bool fn)
{
    if (_scoreApp)
    {
        if (_activeModifier == ModifiersV2::SELECT)
        {
            if (fn)
                _scoreApp->selectNextMeasure();
            else
                _scoreApp->selectNextChord();
        }
        else if (_activeModifier == ModifiersV2::NONE)
        {
            if (fn)
                _scoreApp->moveToNextMeasure();
            else
                _scoreApp->moveToNextChord();
        }
        else
            if (fn)
                emit inputMoveSelection(direction_t::NEXT_PAGE);
            else
                emit inputMoveSelection(direction_t::NEXT_ELEMENT);
    }
}

/*!
 * \brief ODLAControllerV2::onTransposeToolSelected
 * \param opt
 */
void ODLAControllerV2::onTransposeToolSelected()
{
    _scoreApp->transpose();
    emit inputSequenceCompleted(ModifiersV2::OPTIONS);
}

/*!
 * \brief ODLAControllerV2::evaluateMenuOption
 * \param opt
 */
void ODLAControllerV2::evaluateMenuOption(int opt)
{
    qDebug() << "evaluateMenuOption" << opt;
    switch (opt)
    {
    case 1: // file
        _activeModifier = ModifiersV2::FILE;
        emit inputSequenceProgress(ModifiersV2::FILE, opt);
        break;
    case 2: // edit
        _activeModifier = ModifiersV2::EDIT;
        emit inputSequenceProgress(ModifiersV2::EDIT, opt);
        break;
    case 3: // quit
        _scoreApp->quit();
        emit inputSequenceCompleted(ModifiersV2::MENU);
        break;

    default:
            DebugAndLog::debugConsole(tr("Wrong choice: ") + opt);
        return;
    }
}

/*!
 * \brief ODLAControllerV2::evaluateFileSubMenuOption
 * \param opt
 */
void ODLAControllerV2::evaluateFileSubMenuOption(int opt)
{
    switch (opt)
    {
        case 1: _scoreApp->newScore(); break;
        case 2: _scoreApp->openScore(); break;
        case 3: _scoreApp->saveScore(); break;
        case 4: _scoreApp->saveScoreAs(); break;
        case 5: _scoreApp->exportScore(); break;
        case 6: _scoreApp->closeScore(); break;
        case 7: _scoreApp->printScore(); break;
        case 8: _scoreApp->showPageFormatOption(); break;
        case 9: _scoreApp->extractParts(); break;
        default:
            DebugAndLog::debugConsole(tr("Wrong choice: ") + opt);
            return;
    }
    emit inputSequenceCompleted(ModifiersV2::MENU);
}

/*!
 * \brief ODLAControllerV2::evaluateEditSubMenuOption
 * \param opt
 */
void ODLAControllerV2::evaluateEditSubMenuOption(int opt)
{
    int absOpt = opt + _currentOptionPage * 9;
    switch (absOpt)
    {
        case 1: _scoreApp->showInstrumentsSelection(); break;
        case 2: _scoreApp->setViewMode(2 /*line mode*/); break;
        case 3: _scoreApp->setViewMode(1 /*page mode*/); break;
        //case 4: _scoreApp->toggleConcertPitch(); break;
        case 4: _scoreApp->setLoop(); break;
        case 5: _scoreApp->showMixer(); break;
        case 6: _scoreApp->showPlayPanel(); break;
        //case 8: _scoreApp->advancedSelection(); break;
        case 7:
            _activeModifier = ModifiersV2::ADD_MEASURES;
            emit inputSequenceActivated(ModifiersV2::ADD_MEASURES);
            return;
        case 8: _scoreApp->deleteSelection(); break;
        default:
            DebugAndLog::debugConsole(tr("Wrong choice: ") + opt);
            return;
    }
    emit inputSequenceCompleted(ModifiersV2::MENU);

}

/*!
 * \brief ODLAControllerV2::evaluateClefOption
 * \param opt
 */
void ODLAControllerV2::evaluateClefOption(int opt)
{
    Clefs c = _clefKeyMap.value(opt + (_currentOptionPage * 9));
    _scoreApp->insertClef(c);
    emit inputSequenceCompleted(ModifiersV2::CLEF);

}

/*!
 * \brief ODLAControllerV2::evaluateTimeSigOption
 * \param opt
 */
void ODLAControllerV2::evaluateTimeSigOption(int opt)
{
    TimeSig timeSigOpt = static_cast<TimeSig>(opt + (_currentOptionPage * 9));

    bool validOpt = true;

    int num = 4;
    int den = 4;

    switch (timeSigOpt)
    {
    case TimeSig::CUSTOM:
        _timeSigNumCompleted = false;
        _timeSigDenCompleted = false;
        _timeSignatureEditing = true;

        emit inputSequenceProgress(ModifiersV2::TIMESIG, -2); // -2 means show custom edit panel
        break;

    case TimeSig::N2D4:
        num = 2;
        den = 4;
        break;

    case TimeSig::N3D4:
        num = 3;
        den = 4;
        break;

    case TimeSig::N4D4:
        num = 4;
        den = 4;
        break;

    // invert sign to differentiate from case N4D4
    case TimeSig::COMMON_TIME:
        num = -4;
        den = -4;
        break;

    case TimeSig::N5D4:
        num = 5;
        den = 4;
        break;

    case TimeSig::N6D4:
        num = 6;
        den = 4;
        break;

    case TimeSig::N3D8:
        num = 3;
        den = 8;
        break;

    case TimeSig::N6D8:
        num = 6;
        den = 8;
        break;

    case TimeSig::N7D8:
        num = 7;
        den = 8;
        break;

    case TimeSig::N9D8:
        num = 9;
        den = 8;
        break;

    case TimeSig::N12D8:
        num = 12;
        den = 8;
        break;

    case TimeSig::N2D2:
        num = 2;
        den = 2;
        break;

    // invert sign to differentiate from case N2D2
    case TimeSig::ALLA_BREVE:
        num = -2;
        den = -2;
        break;

    case TimeSig::N3D2:
        num = 3;
        den = 2;
        break;

    case TimeSig::N4D2:
        num = 4;
        den = 2;
        break;

    default:
        validOpt = false;
    }

    // predefined option selected
    if (!_timeSignatureEditing && validOpt)
    {
        if (_scoreApp)
        {
             _scoreApp->insertTimeSignature(num, den);
        }
        emit inputSequenceCompleted(ModifiersV2::TIMESIG);
    }
}

/*!
 * \brief ODLAControllerV2::evaluateKeySigOption
 * \param opt
 */
void ODLAControllerV2::evaluateKeySigOption(int opt)
{
    // user must press an accidental key first
    if (_keySigAccidental != Accidentals::NONE)
    {
        if (opt >= 1 && opt <= 7)
        {
            // 2nd number entered is for number of accidentals
            _keySigCount = opt;

            // insert keysig
            _scoreApp->insertKeySignature(_keySigAccidental, _keySigCount);

            _keySigCount = 0;
            _keySigAccidental = Accidentals::NONE;

            emit inputSequenceCompleted(ModifiersV2::KEYSIG);
        }
    }
}

/*!
 * \brief ODLAControllerV2::evaluateBarOption
 * \param opt
 */
void ODLAControllerV2::evaluateBarOption(int opt)
{
    if (opt > 0)
    {
        int absOpt = opt + _currentOptionPage * 9;
        Bars b = static_cast<Bars>(absOpt);
        if (b != Bars::NONE)
        {
            _scoreApp->insertBarline(b);
            emit inputSequenceCompleted(ModifiersV2::BAR);
        }
    }
}

/*!
 * \brief ODLAControllerV2::evaluateExtentedOption
 * \param opt
 */
void ODLAControllerV2::evaluateExtentedOption(int opt)
{
    // step 1: show complete options menu
    if (_selectedFeatureSet == ExtendedFeatures::None)
    {
        if (opt > 0 && opt <= 9)
        {
            _selectedFeatureSet = static_cast<ExtendedFeatures>(opt + _currentOptionPage * 9);
            // comunicate to control panel to show next submenu,
            // for the selected extended feature.
            emit inputSequenceProgress(ModifiersV2::OPTIONS, opt + _currentOptionPage * 9);
        }
    }
    else
    {
        // step 2: show selected feature submenu
        bool isSpecialOption = false;
        bool validOption = applyOptionalFeature(_selectedFeatureSet, opt, &isSpecialOption);

        if (validOption)
        {
            // sequence is completed
            _selectedFeatureSet = ExtendedFeatures::None;
            emit inputSequenceCompleted(ModifiersV2::OPTIONS);
        }
        if (isSpecialOption)
        {
            // special options require numeric input
            emit inputSequenceProgress(ModifiersV2::OPTIONS, 101);
            _textInputEditing = true;
        }
    }
}

void ODLAControllerV2::evaluateTempoShortCut(int opt)
{
    bool isSpecialOption = false;
    bool validOption = applyOptionalFeature(_selectedFeatureSet, opt, &isSpecialOption);

    if (validOption)
    {
        // sequence is completed
        _selectedFeatureSet = ExtendedFeatures::None;
        emit inputSequenceCompleted(ModifiersV2::TEMPO);
    }
    if (isSpecialOption)
    {
        // special options require numeric input
        emit inputSequenceProgress(ModifiersV2::TEMPO, 101);
        _textInputEditing = true;
    }
}

/*!
 * \brief ODLAControllerV2::evaluateAddMeasureOption
 * \param opt
 */
void ODLAControllerV2::evaluateAddMeasureOption(int /*opt*/)
{
    emit inputSequenceProgress(ModifiersV2::ADD_MEASURES, 0);
}

/*!
 * \brief ODLAControllerV2::evaluateGotoOption
 * \param opt
 */
void ODLAControllerV2::evaluateGotoOption(int /*opt*/)
{
    emit inputSequenceProgress(ModifiersV2::GOTO, 0);
}

/*!
 * \brief ODLAControllerV2::evaluateVoiceOption
 * \param opt
 */
void ODLAControllerV2::evaluateVoiceOption(int opt)
{
    if (opt > 0 && opt < 5)
    {
        _scoreApp->setVoice(opt);

        emit inputSequenceCompleted(ModifiersV2::VOICE);
    }
}

/*!
 * \brief ODLAControllerV2::evaluateIntervalOption
 * \param opt
 */
void ODLAControllerV2::evaluateIntervalOption(int opt)
{
    if ((opt > 0) && (opt <= 9) && (_intervalsSign != IntervalSigns::UNVALID))
    {
        Intervals in;
        switch (opt)
        {
            case 1:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_1;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_1;
            break;

            case 2:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_2;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_2;
            break;

            case 3:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_3;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_3;
            break;

            case 4:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_4;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_4;
            break;

            case 5:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_5;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_5;
            break;

            case 6:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_6;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_6;
            break;

            case 7:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_7;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_7;
            break;

            case 8:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_8;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_8;
            break;

            case 9:
                if(_intervalsSign == IntervalSigns::ABOVE)
                    in = Intervals::ABOVE_9;
                if(_intervalsSign == IntervalSigns::BELOW)
                    in = Intervals::BELOW_9;
            break;
        }
        _scoreApp->insertInterval(in);
        // reset interval sign (+ above, - below, 0 unset)
        _intervalsSign = IntervalSigns::UNVALID;
        emit inputSequenceCompleted(ModifiersV2::INTERVALS);
    }
}

/*!
 * \brief ODLAControllerV2::evaluateTupletOption
 * \param opt
 */
void ODLAControllerV2::evaluateTupletOption(int opt)
{
    _tupletValue = opt;
    if (_tupletValue > 1)
    {
        // insert tuplet
        _scoreApp->insertTuplet(_tupletValue, _currentNoteDuration, _currentAugmentationDots);
        _tupletValue = -1;
        //_tupletDuration = Durations::NONE;
        emit inputSequenceCompleted(ModifiersV2::TUPLET);
    }
}

/*!
 * \brief ODLAControllerV2::evaluateFileSubMenuOption
 * \param opt
 */
void ODLAControllerV2::evaluateSelectionMenu(int opt)
{
    assert(_scoreApp);
    switch(opt)
    {
        case 1:
            _scoreApp->selectAllMeasures();
            emit inputSequenceCompleted(ModifiersV2::SELECT_MEASURES);
            break;
        case 2:
            emit inputSequenceProgress(ModifiersV2::SELECT_MEASURES, -2);
            _selectionMeasureRangeEditing = true;
            break;
        default:
            DebugAndLog::debugConsole(tr("Wrong choice: ") + opt);
            return;
    }
}

/*!
 * \brief ODLAControllerV2::resetInternalStateVariables
 */
void ODLAControllerV2::resetInternalStateVariables()
{
    _currentOptionPage = 0;

    _activeModifier = ModifiersV2::NONE;
    //fn = false;
    _chordEditStarted = false;
    _metronomeActive = false;

    _tempoMode = Tempo::NONE;
    _textInputEditing = false;
    _gotoDestination = 0;
    _intervalsSign = IntervalSigns::UNVALID;
    _tupletValue = -1;

    _timeSigNumCompleted = false;
    _timeSigDenCompleted = false;
    _timeSignatureEditing = false;

    _selectionMeasureFromCompleted = false;
    _selectionMeasureToCompleted = false;
    _selectionMeasureRangeEditing = false;

    _numerator = 4;
    _denominator = 4;

    _keySigAccidental = Accidentals::NONE;
    _keySigCount = 0;

    //_currentAugmentationDots = Augmentations::NONE;
    //_dotKeyDuration = ODLASettings::instance()->getDefaultNoteValue();

    _flatKeyMultiplicity = 1;
    _naturalKeyToRemove = false;
    _sharpKeyMultiplicity = 1;
}

/*!
 * \brief ODLAControllerV2::applyOptionalFeature
 * \param feature
 * \param opt
 * \param special
 * \return
 */
bool ODLAControllerV2::applyOptionalFeature(ExtendedFeatures feature, int opt, bool* special)
{
    bool validOpt = false;

    int absOpt = opt + _currentOptionPage * 9;

    switch (feature)
    {
        case ExtendedFeatures::None:
            break;

        case ExtendedFeatures::Dynamics:
            if (absOpt >= 0 && absOpt <= 25)
            {
                _scoreApp->addDynamics(static_cast<Dynamics>(absOpt));
                validOpt = true;
            }
            break;

        case ExtendedFeatures::Tremolo:
            if (absOpt > 0 && absOpt <= 8)
            {
                _scoreApp->addTremolo(static_cast<Tremolo>(absOpt));
                validOpt = true;
            }
            break;

        case ExtendedFeatures::Articulations:
            if (absOpt >= 0 && absOpt <= 22)
            {
                _scoreApp->addArticulation(static_cast<Articulations>(absOpt));
                validOpt = true;
            }
            break;

        case ExtendedFeatures::Ornaments:
            if (absOpt >= 0 && absOpt <= 7)
            {
                _scoreApp->addOrnament(static_cast<Ornaments>(absOpt));
                validOpt = true;
            }
            break;

        case ExtendedFeatures::GraceNotes:
            if (absOpt >= 0 && absOpt <= 8)
            {
                _scoreApp->addGraceNote(static_cast<GraceNotes>(absOpt));
                validOpt = true;
            }
            break;

//        case ExtendedFeatures::BreathsAndPauses:
//            if (absOpt >= 0 && absOpt <= 6)
//            {
//                _scoreApp->addBreathsAndPauses(static_cast<BreathsAndPauses>(absOpt));
//                validOpt = true;
//            }
//            break;

        case ExtendedFeatures::NoteHeads:
            if (absOpt >= 0 && absOpt <= 6)
            {
                _scoreApp->addNoteHead(static_cast<NoteHeads>(absOpt));
                validOpt = true;
            }
            break;

        case ExtendedFeatures::Lines:
            if (absOpt >= 0 && absOpt <= 13)
            {
                _scoreApp->insertLine(static_cast<Lines>(absOpt));
                validOpt = true;
            }
            break;

        case ExtendedFeatures::Fingering:
            if (absOpt >= 0 && absOpt <= 24)
            {
                _scoreApp->addFingering(static_cast<Fingering>(absOpt));
                validOpt = true;
            }
            break;

        case ExtendedFeatures::Arpeggios:
            if (absOpt >= 0 && absOpt <= 16)
            {
                _scoreApp->addArpeggio(static_cast<Arpeggio>(absOpt));
                validOpt = true;
            }
            break;

        case ExtendedFeatures::Tempo:
            if (!_textInputEditing)
            {
                if (absOpt > 0 && absOpt <= 25 && opt != 0)
                {
                    _tempoMode = static_cast<Tempo>(absOpt);

                    if (absOpt > 6)
                    {
                       validOpt = true;
                       // enter key pressed to end denominator input
                       _scoreApp->insertTempo(_tempoMode, 0);
                    }
                    else
                    {
                        // tempo options need numeric input
                        if (special != nullptr)
                            *special = true;
                    }
                }
            }
            else
            {
                // tempo options need numeric input
                if (special != nullptr)
                    *special = true;
            }

            break;
        case ExtendedFeatures::Transpose:
            _scoreApp->transpose();
            break;

        case ExtendedFeatures::Text:
            switch (opt)
            {
                case 1: _scoreApp->addAboveStaffText(); break;
                case 2: _scoreApp->addBelowStaffText(); break;
                case 3: _scoreApp->addChordText(); break;
                case 4: _scoreApp->addLyrics(); break;
            }
            emit inputSequenceCompleted(ModifiersV2::TEXT);
                break;
    }

    return validOpt;
}
