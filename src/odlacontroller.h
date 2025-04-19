#ifndef ODLACONTROLLER_V2_H
#define ODLACONTROLLER_V2_H

#include <QObject>
#include <QMap>
#include "musescore.h"
#include "odla_common.h"

/*!
 * \brief The Modifiers enum
 */
enum class ModifiersV2 : int
{
    NONE,
    MENU,
    FILE,
    EDIT,
    CLEF,
    TIMESIG,
    KEYSIG,
    BAR,
    OPTIONS,
    PREFERRED_OPTION, // Fn + Options
    SELECT,
    GOTO,
    VOICE,
    INTERVALS,
    TUPLET,
    CHORD,
    SLUR,
    SELECT_MEASURES,
    TEMPO,
    TEXT,
    ADD_MEASURES
};

/*!
 * \brief The MessageErrorTypes enum
 */
enum class MessageTypes : int
{
    INTERNAL,
    NOTIFY_USER_ERROR_MSG,
    LOG_ERROR_MSG
};

/*!
 * \brief The ODLAController2 class
 */
class ODLAControllerV2 : public QObject
{
    Q_OBJECT
public:
    explicit ODLAControllerV2(QObject *parent = nullptr);
    Musescore* scoreApp() const;
    ModifiersV2 activeModifier() const;
    ExtendedFeatures defaultExtendedFeatureSet() const;
    void setDefaultExtendedFeatureSet(const ExtendedFeatures& defaultExtendedFeatureSet);
    void setCurrentSelectedKeyIdx(int n);
    void setTempo();
    void setLastKeyPressed(uint8_t pressed);
    void setLastKeyReleased(uint8_t released);
    //MenuDialog _diag;
    void silentHide();

signals:
    void statusMessageReady(QString msg);
    void inputSequenceActivated(ModifiersV2 key);
    void inputSequenceCompleted(ModifiersV2 key);
    void inputSequenceProgress(ModifiersV2 key, int opt = -1); // -1 means don't care
    void inputMoveSelection(direction_t dir);
    void enterEvent(ModifiersV2 modifier, bool fn);
    void inputSequenceAborted(ModifiersV2 key);
    void inputDigitReady(int digit);
    void inputError(MessageTypes type, QString message);
    void quitRequested();

public slots:
    // Init MuseScore when connected to Keyboard
    void onKeyboardConnectionChanged(bool connected);

    void onOptionPageChanged(int page);
    void onRatioInputChanged(int numerator, int denominator);
    void onTextInputChanged(QString text);

    // modifier keys
    void onMenuKeyPressed(bool fn);
    void onClefKeyPressed(bool fn);
    void onTimeSignatureKeyPressed(bool fn);
    void onKeySignatureKeyPressed(bool fn);
    void onBarKeyPressed(bool fn);
    void onOptionsKeyPressed(bool fn);

    // copy / paste

    // select
    void onSelectKeyPressed();
    void onSelectKeyReleased();
    void onCopyKeyPressed(bool fn);
    void onPasteKeyPressed(bool fn);

    // staff
    void onStaffKeyPressed(int line, bool fn);

    // plus / minus
    void onPlusKeyPressed(bool fn);
    void onMinusKeyPressed(bool fn);

    // accidentals
    void onFlatKeyPressed(bool fn);
    void onNaturalKeyPressed(bool fn);
    void onSharpKeyPressed(bool fn);

    // goto / playback
    void onGotoKeyPressed(bool fn);
    void onPlayKeyPressed(bool fn);
    void onPauseKeyPressed(bool fn);
    void onMetronomeKeyPressed(bool fn);

    // voice / intervals / chord / tuplet / slur
    void onVoiceKeyPressed(bool fn);
    void onIntervalKeyPressed(bool fn);
    void onChordKeyPressed(bool fn);
    void onChordKeyReleased(bool fn);
    void onTupletKeyPressed(bool fn);
    void onSlurKeyPressed(bool fn);
    void onSlurKeyReleased(bool fn);

    // numpad / dot
    void onNumberKeyPressed(int digit, bool fn);
    void onDotKeyPressed(bool fn);

    // status / Info
    void onQuestionKeyPressed(bool fn);
    void onHelpKeyPressed(bool fn);

    // undo / redo
    void onUndoKeyPressed(bool fn);
    void onRedoKeyPressed(bool fn);

    // enter / cancel
    void onEnterKeyPressed(bool fn);
    void onCancelKeyPressed(bool fn);

    // arrows
    void onArrowUpKeyPressed(bool fn);
    void onArrowDownKeyPressed(bool fn);
    void onArrowLeftKeyPressed(bool fn);
    void onArrowRightKeyPressed(bool fn);

    //transpose
    void onTransposeToolSelected();

protected slots:
    void resetInternalStateVariables();

protected:
    Musescore* _scoreApp;
    QMap<int, Clefs> _clefKeyMap;
    QMap<int, Durations> _durationKeyMap;

    ModifiersV2 _activeModifier;

    // This map is used to set the option key function:
    // Key: name of the features, e.g. "Dynamics"
    // Value: name of method, e.g."set_Option_Dynamics"
    ExtendedFeatures _defaultExtendedFeatureSet;
    ExtendedFeatures _selectedFeatureSet;

    bool _chordEditStarted;

    int _measuresToAdd;
    int _gotoDestination;
    bool _metronomeActive;

    IntervalSigns _intervalsSign;

    bool _timeSigNumCompleted;
    bool _timeSigDenCompleted;
    bool _timeSignatureEditing;


    bool _selectionMeasureFromCompleted;
    bool _selectionMeasureToCompleted;
    bool _selectionMeasureRangeEditing;

    int _numerator;
    int _denominator;

    int _selectMeasureFrom;
    int _selectMeasureTo;

    Tempo _tempoMode;
    int _tempoNumber;

    bool _textInputEditing;

    Accidentals _keySigAccidental;
    int _keySigCount;

    int _tupletValue;

    Augmentations _currentAugmentationDots;
    Durations _currentNoteDuration;

    int _flatKeyMultiplicity;
    bool _naturalKeyToRemove;
    int _sharpKeyMultiplicity;

    int _currentOptionPage;
    int _currentOptionSelected;
    int _currentWindowType;

    void evaluateMenuOption(int opt);
    void evaluateFileSubMenuOption(int opt);
    void evaluateEditSubMenuOption(int opt);
    //void evaluateToolsSubMenuOption(int opt);
    void evaluateClefOption(int opt);
    void evaluateTimeSigOption(int opt);
    void evaluateKeySigOption(int opt);
    void evaluateBarOption(int opt);
    void evaluateExtentedOption(int opt);
    void evaluateTempoShortCut(int opt);
    void evaluateGotoOption(int opt);
    void evaluateAddMeasureOption(int );
    void evaluateVoiceOption(int opt);
    void evaluateIntervalOption(int opt);
    void evaluateTupletOption(int opt);    
    void evaluateSelectionMenu(int opt);
    bool applyOptionalFeature(ExtendedFeatures feature, int opt, bool* special = nullptr);

private:
    OdlaVK  _prevKeyPressed;
    OdlaVK  _prevKeyReleased;
    OdlaVK  _lastKeyPressed;
    OdlaVK  _lastKeyReleased;
};


#endif // ODLACONTROLLER_V2_H
