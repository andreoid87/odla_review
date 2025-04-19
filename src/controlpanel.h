#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QMainWindow>
#include <QParallelAnimationGroup>
#include <QMap>
#include <QSystemTrayIcon>
#include "odlacontroller.h"

namespace Ui {
class ControlPanel;
}

/**
 * @class ControlPanel
 * @brief Main control panel window for ODLA, handling input, UI states, and interactions.
 *
 * Manages numeric/text input pads, plus/minus and accidental options, and forwards user actions
 * to the ODLAControllerV2. Provides signals for page changes and input completion.
 */
class ControlPanel : public QMainWindow
{
    Q_OBJECT

    enum class activeInputType : int
    {
        None,
        IconNumpad,
        TextNumpad,
        PlusMinusOptions,
        AccidentalOptions,
        NumericInput,
        RatioInput
    };

public:
    /**
     * @brief Constructs the control panel window.
     * @param parent Optional parent widget.
     */
    explicit ControlPanel(QWidget *parent = nullptr);

    /** @brief Destroys the control panel window. */
    ~ControlPanel();

    /**
     * @brief Handles widget change events.
     * @param event The change event.
     */
    virtual void changeEvent(QEvent *event) override;

    /**
     * @brief Handles widget close events.
     * @param event The close event.
     */
    virtual void closeEvent(QCloseEvent *event) override;

    /** @brief Initializes the control panel components. */
    void init();

    /**
     * @brief Sets the ODLA controller.
     * @param odlaController Pointer to the ODLAControllerV2.
     */
    void setODLAController(ODLAControllerV2* odlaController);

    /**
     * @brief Retrieves the About action.
     * @return Pointer to the QAction for About.
     */
    QAction* getAboutAction() const;

    /** @brief Updates the current page display. */
    void setCurrentPage();

public slots:
    void silentHide();

signals:
    void currentPageChanged(int pageIdx);
    void ratioInputChanged(int numerator, int denominator);
    void textInputChanged(QString text);
    void transposeToolSelected();

private slots:
    void onDigitKeyPressed(int digit);
    void clearNumericInputField();
    void toggleCollapseAnimation();
    void prepareNumPadOptions(ODLAStates state, QMap<int, QPixmap> iconMap = QMap<int, QPixmap>(), int customPosition = -1);
    void prepareNumPadOptions(ODLAStates state, QStringList options);
    void preparePlusMinusOptions(ODLAStates state);
    void prepareAccidentalOptions(ODLAStates state);
    void updateCurrentState(ODLAStates state);
    void prepareNumericInput(ODLAStates state, QString title = QString());
    void prepareRatioInput(ODLAStates state, QString title, bool numerator); // true = numerator; false = denominator
    void updateStatusMessage(QString msg);
    void onInputSequenceStarted(ModifiersV2 mod);
    void onInputMoveSelection(direction_t dir);
    void onOkEvent(ModifiersV2 modifier, bool fn);
    void onInputSequenceProgress(ModifiersV2 mod, int opt);
    void onInputSequenceCompleted();
    void onInputError(MessageTypes type, QString msg);
    void on_action_Settings_triggered();
    void on_action_Close_triggered();
    void on_action_About_triggered();

private:
    Ui::ControlPanel *ui;
    ODLAControllerV2 *_odlaController;
    QParallelAnimationGroup _toggleAnimation;
    bool _askForConfirmOnClosing;

    QMap<ODLAStates, QString> _statesMap;
    QMap<ODLAStates, QStringList> _optMap;

    QPoint centerOfScreen;

    /**
     * @struct KeyStyle
     * @brief Holds style parameters for keypad rendering.
     */
    struct KeyStyle {
        QColor disabledColor;
        QColor enabledColor;
        QColor selectedColor;
        QColor highlightColor;
        QColor backColor;
        QFont numberFont;
        QFont labelFont;
        QFont sideLabelFont;
        QFont bigFont;
        float keySize;
        float keySpacing;
        float arrowWidth;
    };

    KeyStyle _keyStyle;

    bool _autoExpand;
    bool _neverCollapse;
    int _collapsedHeight;
    int _expandedHeight;

    int _currentPage;
    int _absCurrentSelectedKeyIdx;
    QStringList _currentOptionLabels;
    activeInputType _currentActiveWindow;
    int _currentCustomPosition;
    QMap<int, QPixmap> _currentIconMap;
    QString _currentNumericInputTitle;
    QString _currentRatioInputTitle;
    int _currentMaxOptionIndex;
    bool _optionSubMenuEntered;

    QSystemTrayIcon *tray;

    ODLAStates _currentState;
    QStringList _lastIconNameList;

    QString _upperNumberString;
    QString _lowerNumberString;

    bool _tempoEditing;
    QString _textInput;

    void setCurrentSelectedKeyIdx(int newValue, int maxOptionIndex = -1);

    void initStringMaps();
    void extendedFeaturesRequested(ExtendedFeatures set);

    QImage makeNumPadIcon(int absoluteOptionNumber, QPixmap symbol, QSize size, bool disableKeyWithoutSymbol = true);
    QImage makeNumPadIcon(int absoluteOptionNumber, QString label, QSize size, bool disableKeyWithoutLabel = true);

signals:
    void dialogShown();
};

#endif // CONTROLPANEL_H