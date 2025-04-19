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
 * \class ODLAControllerV2
 * \brief Manages all keyboard-driven input sequences and routes them to the notation engine.
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
    void silentHide();

signals:
    void statusMessageReady(QString msg);
    void inputSequenceActivated(ModifiersV2 key);
    void inputSequenceCompleted(ModifiersV2 key);
    void inputSequenceProgress(ModifiersV2 key, int opt = -1); // -1 means don't care
    void inputMoveSelection(direction_t dir);
    void enterEvent(ModifiersV2 modifier, bool fn);

    // modifier keys
    // TODO: refactor on*KeyPressed/on*KeyReleased into a macro or generic handler
    #define DECLARE_KEY_SLOT(name) \
      /// \brief Handler for the name key press event. \
      void on##name##KeyPressed(bool fn); \
      void on##name##KeyReleased(bool fn);

    // Key-slot declarations
    DECLARE_KEY_SLOT(Menu)
    DECLARE_KEY_SLOT(Clef)
    DECLARE_KEY_SLOT(TimeSignature)
    DECLARE_KEY_SLOT(KeySignature)
    DECLARE_KEY_SLOT(Bar)
    DECLARE_KEY_SLOT(Options)
    DECLARE_KEY_SLOT(Select)
    DECLARE_KEY_SLOT(Copy)
    DECLARE_KEY_SLOT(Paste)
    DECLARE_KEY_SLOT(Staff)
    DECLARE_KEY_SLOT(Plus)
    DECLARE_KEY_SLOT(Minus)
    DECLARE_KEY_SLOT(Flat)
    DECLARE_KEY_SLOT(Natural)
    DECLARE_KEY_SLOT(Sharp)
    DECLARE_KEY_SLOT(Goto)
    DECLARE_KEY_SLOT(Play)
    DECLARE_KEY_SLOT(Pause)
    DECLARE_KEY_SLOT(Metronome)
    DECLARE_KEY_SLOT(Voice)
    DECLARE_KEY_SLOT(Interval)
    DECLARE_KEY_SLOT(Chord)
    DECLARE_KEY_SLOT(Tuplet)
    DECLARE_KEY_SLOT(Slur)
    DECLARE_KEY_SLOT(Number)
    DECLARE_KEY_SLOT(Dot)
    DECLARE_KEY_SLOT(Question)
    DECLARE_KEY_SLOT(Help)
    DECLARE_KEY_SLOT(Undo)
    DECLARE_KEY_SLOT(Redo)
    DECLARE_KEY_SLOT(Enter)
    DECLARE_KEY_SLOT(Cancel)
    DECLARE_KEY_SLOT(ArrowUp)
    DECLARE_KEY_SLOT(ArrowDown)
    DECLARE_KEY_SLOT(ArrowLeft)
    DECLARE_KEY_SLOT(ArrowRight)

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
