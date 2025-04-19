#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QMainWindow>
#include <QParallelAnimationGroup>
#include <QMap>
#include <QSystemTrayIcon>
#include "odlacontroller.h"
#include "localizationmanager.h"
<<<<<<< HEAD:src/controlpanel.h
=======

>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.h

namespace Ui {
class ControlPanel;
}

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
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();

    virtual void changeEvent(QEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

    void init();
    void setODLAController(ODLAControllerV2* odlaController);
    QAction* getAboutAction() const;

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
    //MainWindow *_simulatorWindow;
    bool _askForConfirmOnClosing;

    QMap<ODLAStates, QString> _statesMap;
    QMap<ODLAStates, QStringList> _optMap;

    QPoint centerOfScreen;

    QColor _disabledKeyColor;
    QColor _enabledKeyColor;
    QColor _selectedKeyColor;
    QColor _highlightColor;
    QColor _keyPadBackColor;
    QFont _keyNumberFont;
    QFont _keyLabelFont;
    QFont _keySideLabelFont;
    QFont _bigFont;
    float _keySize;
    float _keySpacing;
    float _arrowWidth;

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

    // the current state of menu
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
