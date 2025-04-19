#include "controlpanel.h"
#include "ui_controlpanel.h"
#include <QPropertyAnimation>
#include <QDesktopWidget>
#include <QScreen>
#include <QMenu>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include <QFontDatabase>
#include <algorithm> // std::reverse
#include "database.h"
#include "settingsdialog.h"
<<<<<<< HEAD:src/controlpanel.cpp
#include "musescore.h"
#include "voiceover.h"
=======
#include "musescoreplugin.h"
#include <vocalsynt.h>
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
//#include "mainwindow.h"

/*!
 * \brief ControlPanel::ControlPanel
 * \param parent
 */
ControlPanel::ControlPanel(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ControlPanel)
{
    ui->setupUi(this);

    centerOfScreen = QGuiApplication::screens().at(0)->geometry().center() - frameGeometry().center();
    _askForConfirmOnClosing = true;

    _currentPage = 0;
    _absCurrentSelectedKeyIdx = 0;
    _currentMaxOptionIndex = 0;
    _optionSubMenuEntered = false;

    _tempoEditing = false;
    _textInput = "";

    _disabledKeyColor = QColor(75, 76, 78);
    _enabledKeyColor = QColor(46, 47, 48);
    _selectedKeyColor = QColor(0, 0, 0);
    _highlightColor = QColor(255, 60, 57);
    _keyPadBackColor = QColor(40, 40, 40);

#ifdef Q_OS_WIN
    _keyNumberFont = QFont("Segoe UI");
#else
    _keyNumberFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
#endif
    _keyNumberFont.setPixelSize(16);
    _keyNumberFont.setBold(true);

#ifdef Q_OS_WIN
    _keyLabelFont = QFont("Segoe UI");
#else
    _keyLabelFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
#endif
    _keyLabelFont.setPixelSize(12);
    _keyLabelFont.setItalic(true);

#ifdef Q_OS_WIN
    _keySideLabelFont = QFont("Segoe UI");
#else
    _keySideLabelFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
#endif
    _keySideLabelFont.setPixelSize(24);
    _keySideLabelFont.setItalic(true);

#ifdef Q_OS_WIN
    _bigFont = QFont("Segoe UI");
#else
    _bigFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
#endif
    _bigFont.setPixelSize(36);

    ui->menuTitleLabel->setFont(_keyNumberFont);

    _keySize = 90.0f;
    _keySpacing = 5.0f;
    _arrowWidth = 10.0f;

//    _simulatorWindow = new MainWindow();
//    _simulatorWindow->setStyleSheet(qApp->styleSheet());
//    _simulatorWindow->setAboutAction(getAboutAction());
//    _simulatorWindow->setWindowModality(Qt::NonModal);

//    connect(ui->actionSimulator, &QAction::toggled, _simulatorWindow, &MainWindow::setVisible);
//    connect(_simulatorWindow, &MainWindow::closingMainWindow, ui->actionSimulator, [this]() {
//        ui->actionSimulator->setChecked(false);
//    });

    QMenu* buttonMenu = new QMenu(this);
    //menu->addAction(ui->actionODLA);
    //menu->addAction(ui->actionSimulator); // uncomment this line to enable Simulator tool
    buttonMenu->addAction(ui->action_Settings);
    buttonMenu->addSeparator();
    buttonMenu->addAction(ui->action_About);
    ui->settingsButton->setVisible(false);
    ui->settingsButton->setMenu(buttonMenu);

    QMenu* trayMenu = new QMenu(this);
    trayMenu->addAction(ui->action_Settings);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->action_About);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->action_Close);

    tray = new QSystemTrayIcon(this);
    tray->setIcon(QIcon(":/icons/kricon.png"));
    tray->setContextMenu(trayMenu);

    _odlaController = nullptr;
    _autoExpand = true;
    _collapsedHeight = 0;
    _expandedHeight = 0;
    _neverCollapse = false;

    // TODO: enable this to remove window decoration
    //setWindowFlags(Qt::FramelessWindowHint);

    // force this window to stay on top and without frame
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->setFixedSize(340, 484);
    initStringMaps();
<<<<<<< HEAD:src/controlpanel.cpp
    _odlaLocalization = LocalizationManager::instance();
    _odlaLocalization->onLanguageChanged(Database::instance()->getLanguage());
=======
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
}

/*!
 * \brief ControlPanel::~ControlPanel
 */
ControlPanel::~ControlPanel()
{
    delete ui;
//    delete _simulatorWindow;
}

/*!
 * \brief ControlPanel::changeEvent
 * \param event
 */
void ControlPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        // translate status message
        emit _odlaController->statusMessageReady(tr("Ready."));

        initStringMaps();

        // translate option list (if active)
        if (_optMap.contains(_currentState))
        {
            //prepareOptionList(_lastState, _lastIconNameList);

            if (isVisible())
                toggleCollapseAnimation();
        }
    }
    else
    {
        QWidget::changeEvent(event);
    }
}

/*!
 * \brief ControlPanel::closeEvent
 * \param event
 */
void ControlPanel::closeEvent(QCloseEvent *event)
{
    event->ignore();

    on_action_Close_triggered();
}

/*!
 * \brief ControlPanel::init
 */
void ControlPanel::init()
{
    //prepareNumPadOptions(ODLAStates::Idle); // ACCORDINO: verificare
    toggleCollapseAnimation();
    raise();
    move(centerOfScreen);
    hide();
    tray->show();
}

/*!
 * \brief ControlPanel::toggleCollapseAnimation
 */
void ControlPanel::toggleCollapseAnimation()
{    
    move(centerOfScreen);
    qDebug() << "toggleCollapseAnimation, pos: " <<  pos();
}

/*!
 * \brief ControlPanel::prepareNumPadOptions
 * \param opts
 */
void ControlPanel::prepareNumPadOptions(ODLAStates state,  QMap<int, QPixmap> iconMap, int customPosition)
{
    updateCurrentState(state);
    _currentActiveWindow = activeInputType::IconNumpad;

    _currentCustomPosition = customPosition;
    _currentIconMap = iconMap;

    int maxPageId = (iconMap.count() - 1) / 9;

    QPixmap numpadPix(ui->numPad->size());
    numpadPix.fill(_keyPadBackColor);

    QPainter painter(&numpadPix);

    float padSize = _keySize * 3 + _keySpacing * 2;

    float startX = numpadPix.width() / 2 - padSize / 2;
    float startY = numpadPix.height() / 2 - padSize / 2;

    int pageIdxOffset = _currentPage * 9;

    // draw keys
    for (int i = 0; i < 3; i++)
    {
        for (int fine = 0; fine < 3; fine++)
        {
            int x = i * 3 + fine;
            int optNumber = x + pageIdxOffset;

            QImage keyPix;
            if (optNumber == customPosition)
            {
                // add the string Custom in time signature menu
                keyPix = makeNumPadIcon(x, tr("Custom"), QSize(_keySize, _keySize));
            }
            else
            {
                // default: draw the option's symbol
                keyPix = makeNumPadIcon(x, iconMap.value(x + pageIdxOffset + 1), QSize(_keySize, _keySize));
            }
            QRect target(startX + (_keySpacing + _keySize) * fine, startY + (_keySpacing + _keySize) * i, keyPix.width(), keyPix.height());
            painter.drawPixmap(target, QPixmap::fromImage(keyPix), keyPix.rect());
        }
    }

    // draw page controls
    if (_currentPage > 0)
    {
        float margin = 10;
        QPointF a(margin, numpadPix.height() / 2);
        QPointF b(margin + _arrowWidth, numpadPix.height() / 2 - _arrowWidth);
        QPointF c(margin + _arrowWidth, numpadPix.height() / 2 + _arrowWidth);
        QPolygonF leftArrow;
        leftArrow.append(a);
        leftArrow.append(b);
        leftArrow.append(c);
        painter.setPen(_disabledKeyColor);
        painter.setBrush(_disabledKeyColor);
        painter.drawPolygon(leftArrow);
    }

    if (_currentPage < maxPageId)
    {
        float margin = numpadPix.width() - 10;
        QPointF a(margin, numpadPix.height() / 2);
        QPointF b(margin - _arrowWidth, numpadPix.height() / 2 - _arrowWidth);
        QPointF c(margin - _arrowWidth, numpadPix.height() / 2 + _arrowWidth);
        QPolygonF leftArrow;
        leftArrow.append(a);
        leftArrow.append(b);
        leftArrow.append(c);
        painter.setPen(_disabledKeyColor);
        painter.setBrush(_disabledKeyColor);
        painter.drawPolygon(leftArrow);
    }

    ui->numPad->setPixmap(numpadPix);
    QString optionDescription = _optMap.value(_currentState).at(_absCurrentSelectedKeyIdx);
<<<<<<< HEAD:src/controlpanel.cpp
    VoiceOver::instance()->say(optionDescription.split(";").last()); //wait until synt finished to speak
=======
    VocalSynt::say(optionDescription.split(";").last()); //wait until synt finished to speak
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
}

/*!
 * \brief ControlPanel::prepareNumPadOptions
 * \param state
 * \param options
 */
void ControlPanel::prepareNumPadOptions(ODLAStates state, QStringList options)
{
    updateCurrentState(state);
    _currentActiveWindow = activeInputType::TextNumpad;

    QPixmap numpadPix(ui->numPad->size());
    numpadPix.fill(_keyPadBackColor);

    QPainter painter(&numpadPix);

    float padSize = _keySize * 3 + _keySpacing * 2;

    float startX = numpadPix.width() / 2 - padSize / 2;
    float startY = numpadPix.height() / 2 - padSize / 2;

    int pageIdxOffset = _currentPage * 9;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int idx = j * 3 + i;

            QString label;
            if (options.length() > idx + pageIdxOffset)
                label = options.at(idx + pageIdxOffset);
            QImage keyPix = makeNumPadIcon(idx, label, QSize(_keySize, _keySize)); //.replace(" ", "\n")

            QRect target(startX + (_keySpacing + _keySize) * i, startY + (_keySpacing + _keySize) * j, keyPix.width(), keyPix.height());
            painter.drawPixmap(target, QPixmap::fromImage(keyPix), keyPix.rect());
        }
    }

    // draw page controls
    if (_currentPage > 0)
    {
        float margin = 10;
        QPointF a(margin, numpadPix.height() / 2);
        QPointF b(margin + _arrowWidth, numpadPix.height() / 2 - _arrowWidth);
        QPointF c(margin + _arrowWidth, numpadPix.height() / 2 + _arrowWidth);
        QPolygonF leftArrow;
        leftArrow.append(a);
        leftArrow.append(b);
        leftArrow.append(c);
        painter.setPen(_disabledKeyColor);
        painter.setBrush(_disabledKeyColor);
        painter.drawPolygon(leftArrow);
    }

    int maxPageId = (options.count() - 1) / 9;
    if (_currentPage < maxPageId)
    {
        float margin = numpadPix.width() - 10;
        QPointF a(margin, numpadPix.height() / 2);
        QPointF b(margin - _arrowWidth, numpadPix.height() / 2 - _arrowWidth);
        QPointF c(margin - _arrowWidth, numpadPix.height() / 2 + _arrowWidth);
        QPolygonF leftArrow;
        leftArrow.append(a);
        leftArrow.append(b);
        leftArrow.append(c);
        painter.setPen(_disabledKeyColor);
        painter.setBrush(_disabledKeyColor);
        painter.drawPolygon(leftArrow);
    }

    _currentOptionLabels = options;
    ui->numPad->setPixmap(numpadPix);
    QString optionDescription = _optMap.value(_currentState).at(_absCurrentSelectedKeyIdx);
<<<<<<< HEAD:src/controlpanel.cpp
    VoiceOver::instance()->say(optionDescription.split(";").last()); //wait until synt finished to speak only if is not visible
=======
    VocalSynt::say(optionDescription.split(";").last()); //wait until synt finished to speak only if is not visible
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
}

/*!
 * \brief ControlPanel::preparePlusMinusOptions
 * \param state
 */
void ControlPanel::preparePlusMinusOptions(ODLAStates state)
{
    updateCurrentState(state);
    _currentActiveWindow = activeInputType::PlusMinusOptions;

    QPixmap padPix(ui->numPad->size());
    padPix.fill(_keyPadBackColor);

    QPainter painter(&padPix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);

    float padSizeX = _keySize;
    float padSizeY = _keySize * 2 + _keySpacing;

    QFont font("Sans serif");
    font.setPointSizeF(22);

    QFont fontItalic("Sans serif");
    fontItalic.setPointSizeF(12);
    fontItalic.setItalic(true);

    QFontMetricsF metrics(font);

    QImage keyPixPlus = makeNumPadIcon(0, QPixmap(), QSize(_keySize, _keySize), false);
    QImage keyPixMinus = makeNumPadIcon(1, QPixmap(), QSize(_keySize, _keySize), false);

    float startX = padPix.width() / 2 - padSizeX / 2;
    float startY = padPix.height() / 2 - padSizeY / 2;

    QRect target1(startX, startY, keyPixPlus.width(), keyPixPlus.height());
    painter.drawPixmap(target1, QPixmap::fromImage(keyPixPlus), keyPixPlus.rect());

    QPainterPath textPath;
    textPath.addText(startX + keyPixPlus.width() / 2 - (metrics.width("+") / 2),
                     startY + keyPixPlus.height() - metrics.height(),
                     font, "+");

    textPath.addText(startX + _keySize + 10, startY + keyPixPlus.height() - metrics.height(),
                     fontItalic, tr("Above"));

    startY += _keySize + _keySpacing;

    QRect target2(startX, startY, keyPixMinus.width(), keyPixMinus.height());
    painter.drawPixmap(target2, QPixmap::fromImage(keyPixMinus), keyPixMinus.rect());

    textPath.addText(startX + keyPixMinus.width() / 2 - (metrics.width("-") / 2),
                     startY + keyPixMinus.height() - metrics.height(),
                     font, "-");

    textPath.addText(startX + _keySize + 10, startY + keyPixMinus.height() - metrics.height(),
                     fontItalic, tr("Below"));

    painter.drawPath(textPath);

    ui->numPad->setPixmap(padPix);
    QString optionDescription = _optMap.value(_currentState).at(_absCurrentSelectedKeyIdx);
<<<<<<< HEAD:src/controlpanel.cpp
    VoiceOver::instance()->say(optionDescription.split(";").last()); //wait until synt finished to speak
=======
    VocalSynt::say(optionDescription.split(";").last()); //wait until synt finished to speak
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
}

/*!
 * \brief ControlPanel::prepareAccidentalOptions
 * \param state
 */
void ControlPanel::prepareAccidentalOptions(ODLAStates state)
{
    updateCurrentState(state);
    _currentActiveWindow = activeInputType::AccidentalOptions;

    QPixmap padPix(ui->numPad->size());
    padPix.fill(_keyPadBackColor);

    QPainter painter(&padPix);

    float padSizeX = _keySize;

    float padSizeY = _keySize * 3 + _keySpacing * 2;

    auto map = Metadata::accidentalsIcons();

    QImage sharpPix = makeNumPadIcon(0, map.value(static_cast<int>(Accidentals::SHARP)), QSize(_keySize, _keySize));
    QImage naturalPix = makeNumPadIcon(1, map.value(static_cast<int>(Accidentals::NATURAL)), QSize(_keySize, _keySize));
    QImage flatPix = makeNumPadIcon(2, map.value(static_cast<int>(Accidentals::FLAT)), QSize(_keySize, _keySize));

    float startX = padPix.width() / 2 - padSizeX / 2;
    float startY = padPix.height() / 2 - padSizeY / 2;

    QRect target1(startX, startY, sharpPix.width(), sharpPix.height());
    painter.drawPixmap(target1, QPixmap::fromImage(sharpPix), sharpPix.rect());

    startY += _keySize + _keySpacing;

    QRect target2(startX, startY, naturalPix.width(), naturalPix.height());
    painter.drawPixmap(target2, QPixmap::fromImage(naturalPix), naturalPix.rect());

    startY += _keySize + _keySpacing;

    QRect target3(startX, startY, flatPix.width(), flatPix.height());
    painter.drawPixmap(target3, QPixmap::fromImage(flatPix), flatPix.rect());

    ui->numPad->setPixmap(padPix);
    QString optionDescription = _optMap.value(_currentState).at(_absCurrentSelectedKeyIdx);
<<<<<<< HEAD:src/controlpanel.cpp
    VoiceOver::instance()->say(optionDescription.split(";").last()); //wait until synt finished to speak
=======
    VocalSynt::say(optionDescription.split(";").last()); //wait until synt finished to speak
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
}

/*!
 * \brief ControlPanel::updateLastState
 * \param state
 */
void ControlPanel::updateCurrentState(ODLAStates state)
{
    _currentState = state;

    ui->menuTitleLabel->clear();
    QString title = _statesMap[state].toUpper().split("|").first();
    ui->menuTitleLabel->setText(title);
}

/*!
 * \brief ControlPanel::prepareNumericInput
 */
void ControlPanel::prepareNumericInput(ODLAStates state, QString title)
{
    updateCurrentState(state);
    _currentActiveWindow = activeInputType::NumericInput;
    _currentNumericInputTitle = title;

    QPixmap padPix(ui->numPad->size());
    padPix.fill(_keyPadBackColor);

    QPainter painter(&padPix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);


    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);

    // draw title
    QFontMetricsF titleMetrics(_keySideLabelFont);
    QRectF titleRect = titleMetrics.boundingRect(title).adjusted(0,0,5,5);
    titleRect.moveCenter(QPointF(padPix.width() / 2, titleMetrics.height()));

    painter.setFont(_keySideLabelFont);
    painter.drawText(titleRect, title);

    // draw num / den
    QFontMetricsF metrics(_bigFont);
    QRectF numRect = metrics.boundingRect(_textInput).adjusted(0,0,5,5);
    numRect.moveCenter(QPointF(padPix.width() / 2, (padPix.height() / 2) + 30));

    painter.setFont(_bigFont);

    painter.setPen(_highlightColor);
    painter.setBrush(_highlightColor);

    painter.drawText(numRect, _textInput);

    ui->numPad->setPixmap(padPix);
}

/*!
 * \brief ControlPanel::prepareRatioInput
 * \param state
 * \param title
 */
void ControlPanel::prepareRatioInput(ODLAStates state, QString title, bool numerator)
{
    _currentActiveWindow = activeInputType::RatioInput;
    _currentRatioInputTitle = title;

    QPixmap padPix(ui->numPad->size());
    padPix.fill(_keyPadBackColor);

    QPainter painter(&padPix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);


    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);

    // draw title
    QFontMetricsF titleMetrics(_keySideLabelFont);
    QRectF titleRect = titleMetrics.boundingRect(title).adjusted(0,0,5,5);
    titleRect.moveCenter(QPointF(padPix.width() / 2, titleMetrics.height()));

    painter.setFont(_keySideLabelFont);
    painter.drawText(titleRect, title);

    // draw num / den
    QFontMetricsF metrics(_bigFont);
    QRectF numRect = metrics.boundingRect(_upperNumberString).adjusted(0,0,5,5);
    QRectF denRect = metrics.boundingRect(_lowerNumberString).adjusted(0,0,5,5);
    numRect.moveCenter(QPointF(padPix.width() / 2, (padPix.height() / 4) + 30));
    denRect.moveCenter(QPointF(padPix.width() / 2, (padPix.height() / 4 * 3) - 30));

    painter.setFont(_bigFont);

    if (numerator)
    {
        painter.setPen(_highlightColor);
        painter.setBrush(_highlightColor);
    }

    painter.drawText(numRect, _upperNumberString);

    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    painter.drawLine(QPointF(padPix.width() / 4, padPix.height() / 2),
                     QPointF(padPix.width() / 4 * 3, padPix.height() / 2));

    if (!numerator)
    {
        painter.setPen(_highlightColor);
        painter.setBrush(_highlightColor);
    }

    painter.drawText(denRect, _lowerNumberString);

    ui->numPad->setPixmap(padPix);

    _currentState = state;
}

/*!
 * \brief ControlPanel::updateStatusMessage
 * \param msg
 */
void ControlPanel::updateStatusMessage(QString msg)
{
    ui->statusLabel->setText(msg);
}

/*!
 * \brief ControlPanel::onInputSequenceStarted
 * \param mod
 */
void ControlPanel::onInputSequenceStarted(ModifiersV2 mod)
{
    setCurrentSelectedKeyIdx(0);
    _currentIconMap.clear();
    _currentOptionLabels.clear();
    _currentCustomPosition = 0;

    show();
    raise();
    move(centerOfScreen);
    emit dialogShown();

    // map modifier to state
    switch(mod)
    {
    case ModifiersV2::NONE:
        updateCurrentState(ODLAStates::Idle);
        setCurrentSelectedKeyIdx(0);
        onInputSequenceCompleted();
        return;

    case ModifiersV2::MENU:
        updateCurrentState(ODLAStates::Menu);
        setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::Menu].size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::Menu, _optMap[ODLAStates::Menu]);
        break;

    case ModifiersV2::GOTO:
        updateCurrentState(ODLAStates::GotoMeasure);
        setCurrentSelectedKeyIdx(0, 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), false);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumericInput(ODLAStates::GotoMeasure, tr("Enter measure"));
        break;

    case ModifiersV2::ADD_MEASURES:
        updateCurrentState(ODLAStates::AddMeasures);
        setCurrentSelectedKeyIdx(0, 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState));
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumericInput(ODLAStates::AddMeasures, tr("How many measures?"));
        break;

    case ModifiersV2::CLEF:
        updateCurrentState(ODLAStates::Clefs);
        setCurrentSelectedKeyIdx(0, Metadata::clefsIcons().size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::Clefs, Metadata::clefsIcons());
        break;

    case ModifiersV2::TIMESIG:
        updateCurrentState(ODLAStates::TimeSig);
        setCurrentSelectedKeyIdx(0, Metadata::timeSignaturesIcons().size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::TimeSig, Metadata::timeSignaturesIcons(), 0);
        break; // 1 means: key 1 is used for custom option

    case ModifiersV2::VOICE:
        updateCurrentState(ODLAStates::Voices);
        setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::Voices].size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::Voices, _optMap[ODLAStates::Voices]);
        break;

    case ModifiersV2::TUPLET:
        updateCurrentState(ODLAStates::Tuplet1);
        setCurrentSelectedKeyIdx(1, _optMap[ODLAStates::Tuplet1].size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::Tuplet1, _optMap[ODLAStates::Tuplet1]);
        break;

    case ModifiersV2::KEYSIG:
        updateCurrentState(ODLAStates::KeySig);
        setCurrentSelectedKeyIdx(0, 2);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareAccidentalOptions(ODLAStates::KeySig);
        break;

    case ModifiersV2::BAR:
        updateCurrentState(ODLAStates::Bars);
        setCurrentSelectedKeyIdx(0, Metadata::barlinesIcons().size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::Bars, Metadata::barlinesIcons());
        break;

    case ModifiersV2::INTERVALS:
        updateCurrentState(ODLAStates::Intervals);
        setCurrentSelectedKeyIdx(0, 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        preparePlusMinusOptions(ODLAStates::Intervals);
        break;

    case ModifiersV2::OPTIONS:
        updateCurrentState(ODLAStates::Options);
        setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::Options].size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::Options, _optMap[ODLAStates::Options]);
        break;

    case ModifiersV2::TEMPO:
        updateCurrentState(ODLAStates::Tempo);
        setCurrentSelectedKeyIdx(0, Metadata::tempoIcons().size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::Tempo, Metadata::tempoIcons());
        break;

    case ModifiersV2::SELECT_MEASURES:
        updateCurrentState(ODLAStates::SelectMeasure);
        setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::SelectMeasure].size() - 1);
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        prepareNumPadOptions(ODLAStates::SelectMeasure, _optMap[ODLAStates::SelectMeasure]);
        break;

    case ModifiersV2::PREFERRED_OPTION:
<<<<<<< HEAD:src/controlpanel.cpp
        updateCurrentState(Database::instance()->getStateFromPreferredFeatureSet());
        VoiceOver::instance()->say(_statesMap.value(_currentState), true);
=======
        updateCurrentState(ODLASettings::getStateFromPreferredFeatureSet());
        VocalSynt::say(_statesMap.value(_currentState), VocalSynt::HIGH);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        extendedFeaturesRequested(_odlaController->defaultExtendedFeatureSet());
        break;

    default:
        break;
    }
}

void ControlPanel::onInputMoveSelection(direction_t dir)
{
    switch(dir)
    {
        case direction_t::PREV_ELEMENT:
            setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx - 1);
            break;

        case direction_t::NEXT_ELEMENT:
            setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx + 1);
            break;
        case direction_t::PREV_ROW:
            if(_currentActiveWindow == activeInputType::AccidentalOptions
            || _currentActiveWindow == activeInputType::PlusMinusOptions)
                setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx - 1);
            else
                setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx - 3);
            break;

        case direction_t::NEXT_ROW:
            if(_currentActiveWindow == activeInputType::AccidentalOptions
            || _currentActiveWindow == activeInputType::PlusMinusOptions)
                setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx + 1);
            else
                setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx + 3);
            break;

        case direction_t::PREV_PAGE:
            setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx - 9);
            break;

        case direction_t::NEXT_PAGE:
             setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx + 9);
            break;

        default:
            break;
    }

    switch(_currentActiveWindow)
    {
    case activeInputType::IconNumpad:
       prepareNumPadOptions(_currentState, _currentIconMap, _currentCustomPosition);
        break;

    case activeInputType::TextNumpad:
        prepareNumPadOptions(_currentState, _currentOptionLabels);
        break;

    case activeInputType::NumericInput:
        prepareNumericInput(_currentState, _currentNumericInputTitle);
        break;

    case activeInputType::AccidentalOptions:
        prepareAccidentalOptions(_currentState);
        break;

    case activeInputType::RatioInput:
        // any action?
        break;

    case activeInputType::PlusMinusOptions:
        preparePlusMinusOptions(_currentState);
        break;

    default:
        break;
    }
}


/*!
 * \brief ControlPanel::onEnterKeyPressEvent
 */
void ControlPanel::onOkEvent(ModifiersV2 modifier, bool fn)
{
    switch (modifier)
    {

    case ModifiersV2::KEYSIG:
        if(_currentActiveWindow == activeInputType::AccidentalOptions)
        {
            switch(_absCurrentSelectedKeyIdx)
            {
            case 0:
                _odlaController->onSharpKeyPressed(fn);
                break;
            case 1:
                _odlaController->onNaturalKeyPressed(fn);
                break;
            case 2:
                _odlaController->onFlatKeyPressed(fn);
                break;
            default:
                break;
            }
            break;
        }
        else
        {
            int emulatedKey = (_absCurrentSelectedKeyIdx % 9) + 1; //save current selection
            _odlaController->onNumberKeyPressed(emulatedKey, fn); //emulate key press
        }
        break;

    case ModifiersV2::INTERVALS:
        if(_currentActiveWindow == activeInputType::PlusMinusOptions)
        {
            switch(_absCurrentSelectedKeyIdx)
            {
            case 0:
                _odlaController->onPlusKeyPressed(fn);
                break;
            case 1:
                _odlaController->onMinusKeyPressed(fn);
            }
            break;
        }
        else
        {
            int emulatedKey = (_absCurrentSelectedKeyIdx % 9) + 1; //save current selection
            _odlaController->onNumberKeyPressed(emulatedKey, fn); //emulate key press
        }
        break;

    default:
        if(_currentActiveWindow == activeInputType::NumericInput)
            _odlaController->setTempo();
        else
        {
            int emulatedKey = (_absCurrentSelectedKeyIdx % 9) + 1; //save current selection
            //qDebug() << "emulating key" << emulatedKey;
            _odlaController->onNumberKeyPressed(emulatedKey, fn); //emulate key press
        }
        break;
    }
}

/*!
 * \brief ControlPanel::onInputSequenceProgress
 * \param mod
 * \param step
 * \param opt
 */
void ControlPanel::onInputSequenceProgress(ModifiersV2 mod, int opt)
{
    setCurrentSelectedKeyIdx(0);
    switch (mod)
    {
        case ModifiersV2::FILE:
            setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::File].size() - 1);
            prepareNumPadOptions(ODLAStates::File, _optMap[ODLAStates::File]);
            break;

        case ModifiersV2::EDIT:
            setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::Edit].size() - 1);
            prepareNumPadOptions(ODLAStates::Edit, _optMap[ODLAStates::Edit]);
            break;

        case ModifiersV2::CLEF:
            prepareNumPadOptions(ODLAStates::Clefs, Metadata::clefsIcons());
            break;

        case ModifiersV2::TIMESIG:
            if (opt == -2)
                prepareRatioInput(ODLAStates::TimeSigCustomNum, tr("Enter numerator"), true);
            else if (opt == -3)
                prepareRatioInput(ODLAStates::TimeSigCustomDen, tr("Enter denominator"), false);
            else
            {
                setCurrentSelectedKeyIdx(0, Metadata::timeSignaturesIcons().size() - 1);
                prepareNumPadOptions(ODLAStates::TimeSig, Metadata::timeSignaturesIcons(), 0); // 1 means: key 1 is used for custom option
            }
            break;

        case ModifiersV2::BAR:
            setCurrentSelectedKeyIdx(0, Metadata::barlinesIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Bars, Metadata::barlinesIcons());
            break;

        case ModifiersV2::KEYSIG:
        {
            Accidentals acc = static_cast<Accidentals>(opt);
            switch (acc) {
            case Accidentals::SHARP:
                setCurrentSelectedKeyIdx(0, Metadata::sharpKeySignaturesIcons().size() - 1);
                prepareNumPadOptions(ODLAStates::SharpKeySig, Metadata::sharpKeySignaturesIcons());
                break;
            case Accidentals::FLAT:
                setCurrentSelectedKeyIdx(0, Metadata::flatKeySignaturesIcons().size() - 1);
                prepareNumPadOptions(ODLAStates::FlatKeySig, Metadata::flatKeySignaturesIcons());
                break;
            default:
                break;
            }
        }
            break;

        case ModifiersV2::GOTO:
            prepareNumericInput(ODLAStates::GotoMeasure, tr("Enter measure"));
            break;

        case ModifiersV2::ADD_MEASURES:
            prepareNumericInput(ODLAStates::AddMeasures, tr("How many measures?"));
            break;

        case ModifiersV2::INTERVALS:
            if (static_cast<IntervalSigns>(opt) == IntervalSigns::ABOVE)
            {
                setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::IntervalsAbove].size() - 1);
                prepareNumPadOptions(ODLAStates::IntervalsAbove, _optMap[ODLAStates::IntervalsAbove]);
            }
            else if (static_cast<IntervalSigns>(opt) == IntervalSigns::BELOW)
            {
                setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::IntervalsBelow].size() - 1);
                prepareNumPadOptions(ODLAStates::IntervalsBelow, _optMap[ODLAStates::IntervalsBelow]);
            }
            break;

        case ModifiersV2::OPTIONS:
        {
            if (opt == 101)
            {
                prepareNumericInput(ODLAStates::TempoEditing, tr("Enter tempo indication"));
            }
            else
            {
                ExtendedFeatures extFeat = static_cast<ExtendedFeatures>(opt);

                if (extFeat == ExtendedFeatures::None)
                {
                    // navigate options menu
                    setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::Options].size() - 1);
                    prepareNumPadOptions(ODLAStates::Options, _optMap[ODLAStates::Options]);
                }
                else
                {
                    if (!_optionSubMenuEntered)
                    {
                        _optionSubMenuEntered = true;
                    }

                    extendedFeaturesRequested(extFeat);
                }
            }
        }

        case ModifiersV2::SELECT_MEASURES:
            if (opt == -2)
                prepareRatioInput(ODLAStates::SelectRangeFrom, tr("Measure from"), true);
            else if (opt == -3)
                prepareRatioInput(ODLAStates::SelectRangeTo, tr("Measure to"), false);
            break;

        case ModifiersV2::TEMPO:
        {
            if (opt == 101)
            {
                prepareNumericInput(ODLAStates::TempoEditing, tr("Enter tempo indication"));
            }
            else
                extendedFeaturesRequested(ExtendedFeatures::Tempo);
            break;
        }

        default:
            break;
    }
}

/*!
 * \brief ControlPanel::onInputSequenceCompleted
 */
void ControlPanel::onInputSequenceCompleted()
{
    if(isVisible())
    {
<<<<<<< HEAD:src/controlpanel.cpp
        VoiceOver::instance()->sayAfterBusy(tr("Closed Window"));
=======
        VocalSynt::say(tr("Closed Window"),VocalSynt::LOW);
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/controlpanel.cpp
        hide();
    }

    _optionSubMenuEntered = false;

    _currentState = ODLAStates::Idle;
    _currentActiveWindow = activeInputType::None;
    clearNumericInputField();

    // collapse option panel if exapanded, can collapse and
    // no other input is expected, i.e. idling
    if (isVisible() && !_neverCollapse)
    {
        toggleCollapseAnimation();
    }
}

/*!
 * \brief ControlPanel::onInputError
 * \param type
 * \param msg
 */
void ControlPanel::onInputError(MessageTypes type, QString msg)
{
    switch (type) {
        case MessageTypes::NOTIFY_USER_ERROR_MSG:
            QMessageBox::critical(nullptr, tr("Ops..."), msg);
            clearNumericInputField();
            break;
        default:
            break;
    }
}

/*!
 * \brief ControlPanel::setODLAController
 * \param odlaController
 */
void ControlPanel::setODLAController(ODLAControllerV2* odlaController)
{
    _odlaController = odlaController;

//    _simulatorWindow->setODLAController(_odlaController);
//    _simulatorWindow->on_action_MuseScore_Mode_triggered(true);

    if (_odlaController != nullptr)
    {
        connect(_odlaController, &ODLAControllerV2::statusMessageReady, this, &ControlPanel::updateStatusMessage);
        connect(_odlaController, &ODLAControllerV2::inputDigitReady, this, &ControlPanel::onDigitKeyPressed);
        connect(_odlaController, &ODLAControllerV2::inputSequenceActivated, this, &ControlPanel::onInputSequenceStarted);
        connect(_odlaController, &ODLAControllerV2::inputSequenceProgress, this, &ControlPanel::onInputSequenceProgress);
        connect(_odlaController, &ODLAControllerV2::inputMoveSelection, this, &ControlPanel::onInputMoveSelection);
        connect(_odlaController, &ODLAControllerV2::enterEvent, this, &ControlPanel::onOkEvent);
        connect(_odlaController, &ODLAControllerV2::inputSequenceCompleted, this, &ControlPanel::onInputSequenceCompleted);
        connect(_odlaController, &ODLAControllerV2::inputSequenceAborted, this, &ControlPanel::onInputSequenceCompleted);
        connect(_odlaController, &ODLAControllerV2::inputError, this, &ControlPanel::onInputError);
        connect(this, &ControlPanel::currentPageChanged, _odlaController, &ODLAControllerV2::onOptionPageChanged);
        connect(this, &ControlPanel::ratioInputChanged, _odlaController, &ODLAControllerV2::onRatioInputChanged);
        connect(this, &ControlPanel::textInputChanged, _odlaController, &ODLAControllerV2::onTextInputChanged);
        connect(this, &ControlPanel::transposeToolSelected, _odlaController, &ODLAControllerV2::onTransposeToolSelected);
        connect(_odlaController, &ODLAControllerV2::quitRequested, this, [this] () {
            _askForConfirmOnClosing = false;
            on_action_Close_triggered();
        });
    }

    // show input panel
    init();
}

/*!
 * \brief ControlPanel::extendedFeaturesRequested
 */
void ControlPanel::extendedFeaturesRequested(ExtendedFeatures set)
{
    switch (set)
    {
        case ExtendedFeatures::None:
        case ExtendedFeatures::Dynamics:
            setCurrentSelectedKeyIdx(0, Metadata::dynamicsIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Dynamics, Metadata::dynamicsIcons()); // dynamics use "0" to remove
            break;
        case ExtendedFeatures::Tremolo:
            setCurrentSelectedKeyIdx(0, Metadata::tremolosIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Tremolo, Metadata::tremolosIcons());
            break;
        case ExtendedFeatures::Articulations:
            setCurrentSelectedKeyIdx(0, Metadata::articulationsIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Articulations, Metadata::articulationsIcons());
            break;
        case ExtendedFeatures::Ornaments:
            setCurrentSelectedKeyIdx(0, Metadata::ornamentsIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Ornaments, Metadata::ornamentsIcons());
            break;
        case ExtendedFeatures::GraceNotes:
            setCurrentSelectedKeyIdx(0, Metadata::graceNotesIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::GraceNotes, Metadata::graceNotesIcons());
            break;
//        case ExtendedFeatures::BreathsAndPauses:
//            setCurrentSelectedKeyIdx(0, Metadata::breathsAndPausesIcons().size() - 1);
//            prepareNumPadOptions(ODLAStates::BreathsAndPauses, Metadata::breathsAndPausesIcons());
//            break;
        case ExtendedFeatures::NoteHeads:
            setCurrentSelectedKeyIdx(0, Metadata::noteHeadsIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::NoteHeads, Metadata::noteHeadsIcons());
            break;  
        case ExtendedFeatures::Lines:
            setCurrentSelectedKeyIdx(0, Metadata::linesIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Lines, Metadata::linesIcons());
            break;
        case ExtendedFeatures::Fingering:
            setCurrentSelectedKeyIdx(0, Metadata::fingeringIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Fingering, Metadata::fingeringIcons());
            break;
        case ExtendedFeatures::Arpeggios:
            setCurrentSelectedKeyIdx(0, Metadata::arpeggioIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Arpeggios, Metadata::arpeggioIcons());
            break;
        case ExtendedFeatures::Tempo:
            setCurrentSelectedKeyIdx(0, Metadata::tempoIcons().size() - 1);
            prepareNumPadOptions(ODLAStates::Tempo, Metadata::tempoIcons());
            break;
        case ExtendedFeatures::Transpose:            
            emit transposeToolSelected(); //the only without a submenu (until now)
            break;
        case ExtendedFeatures::Text:
            setCurrentSelectedKeyIdx(0, _optMap[ODLAStates::Text].size() - 1);
            prepareNumPadOptions(ODLAStates::Text, _optMap[ODLAStates::Text]);
            break;
    }
}

/*!
 * \brief ControlPanel::makeNumPadIcon
 * \param num
 * \param symbol
 * \param size
 * \return
 */
QImage ControlPanel::makeNumPadIcon(int relativeKeyIdxToPaint, QPixmap symbol, QSize size, bool disableKeyWithoutSymbol)
{
    int relativeSelectedKeyIdx = _absCurrentSelectedKeyIdx % 9;

    QString relativeCurrentKeyString = QString::number(relativeKeyIdxToPaint + 1);

    QImage pix(size, QImage::Format_ARGB32);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);


    bool enabledKey = !symbol.isNull() || !disableKeyWithoutSymbol;

    // draw key
    QPainterPath shapePath;
    shapePath.addRoundedRect(QRectF(0, 0, size.width(), size.height()), 16, 16);
    if(enabledKey)
    {
        if(relativeKeyIdxToPaint == relativeSelectedKeyIdx)
        {
            painter.fillPath(shapePath, _selectedKeyColor);
        }
        else
        {
            painter.fillPath(shapePath, _enabledKeyColor);
        }
    }
    else
    {
        painter.fillPath(shapePath, _disabledKeyColor);
    }

    // setup symbol offset and size
    float symbolScaledSize = symbol.width();
    float symbolXOffset = size.width() / 2.0 - symbolScaledSize / 2.0;
    float symbolYOffset = size.height() / 2.0 - symbolScaledSize / 2.0;

    int textMargin = 8;
    QRectF contourMargin(textMargin, textMargin, size.width() - 2 * textMargin, size.height() - 2 * textMargin);

    if (_currentActiveWindow == activeInputType::IconNumpad  || _currentActiveWindow == activeInputType::TextNumpad)
    {
        // top-left number
        QPen whitePen(Qt::white);
        whitePen.setWidth(1);
        painter.setPen(enabledKey ? QColor(Qt::white) : _disabledKeyColor);
        QBrush whiteBrush(enabledKey ? QColor(Qt::white) : _disabledKeyColor, Qt::SolidPattern);
        painter.setBrush(whiteBrush);

        painter.setFont(_keyNumberFont);
        Qt::Alignment flags(Qt::AlignTop | Qt::AlignLeft);
        painter.drawText(contourMargin, flags, relativeCurrentKeyString);

        // move symbol at bottom-right corner and scale it down
        symbolScaledSize = 64;
        symbolXOffset = size.width() / 2 - 30;
        symbolYOffset = size.height() / 2 - 20;
    }

    // draw symbol
    if (enabledKey && !symbol.isNull())
    {
        QPixmap symbolScaled = symbol.scaled(symbolScaledSize, symbolScaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect target(symbolXOffset, symbolYOffset, symbolScaled.width(), symbolScaled.height());
        painter.drawPixmap(target, symbolScaled, symbolScaled.rect());
    }

    return pix;
}

/*!
 * \brief ControlPanel::makeNumPadIcon
 * \param num
 * \param label
 * \param size
 * \param disableKeyWithoutLabel
 * \return
 */
QImage ControlPanel::makeNumPadIcon(int relativeCurrentKeyIdx, QString label, QSize size, bool disableKeyWithoutLabel)
{
    setWindowModality(Qt::NonModal);

    label = label.split("|").first();

    int relativeSelectedKeyIdx = _absCurrentSelectedKeyIdx % 9;
    QString relativeCurrentKeyString = QString::number(relativeCurrentKeyIdx + 1);

    QImage pix(size, QImage::Format_ARGB32);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    bool enabledKey = !label.isEmpty() || !disableKeyWithoutLabel;
    // draw key
    QPainterPath shapePath;
    QRectF contour = QRectF(0, 0, size.width(), size.height());
    shapePath.addRoundedRect(contour, 16, 16);
    if(enabledKey)
    {
        if(relativeCurrentKeyIdx == relativeSelectedKeyIdx)
        {
            painter.fillPath(shapePath, _selectedKeyColor);
        }
        else
        {
            painter.fillPath(shapePath, _enabledKeyColor);
        }
    }
    else
    {
        painter.fillPath(shapePath, _disabledKeyColor);
    }

    int textMargin = 8;
    QRectF contourMargin = QRectF(textMargin, textMargin, size.width() - 2 * textMargin, size.height() - 2 * textMargin);

    if (_currentActiveWindow == activeInputType::IconNumpad  || _currentActiveWindow == activeInputType::TextNumpad)
    {
        // top-left number
        QPen whitePen(Qt::white);
        whitePen.setWidth(1);
        painter.setPen(enabledKey ? QColor(Qt::white) : _disabledKeyColor);
        QBrush whiteBrush(enabledKey ? QColor(Qt::white) : _disabledKeyColor, Qt::SolidPattern);
        painter.setBrush(whiteBrush);

        painter.setFont(_keyNumberFont);
        Qt::Alignment flags(Qt::AlignTop | Qt::AlignLeft);
        painter.drawText(contourMargin, flags, relativeCurrentKeyString);
    }

    // draw symbol
    if (enabledKey)
    {
        Qt::Alignment flags(Qt::TextWordWrap | Qt::AlignCenter | Qt::AlignBottom);
        painter.setFont(_keyLabelFont);
        painter.drawText(contourMargin, flags ,label);
    }

    return pix;
}

/*!
 * \brief ControlPanel::getAboutAction
 * \return
 */
QAction*ControlPanel::getAboutAction() const
{
   return ui->action_About;
}

/*!
 * \brief ControlPanel::onDigitKeyPressed
 * \param digit
 */
void ControlPanel::onDigitKeyPressed(int digit)
{   
    if (_currentState == ODLAStates::TimeSigCustomNum)
    {
        // append digit to current number
        _upperNumberString = QString("%1%2").arg(_upperNumberString, QString::number(digit));

        emit ratioInputChanged(_upperNumberString.toInt(), _lowerNumberString.toInt());
    }

    if (_currentState == ODLAStates::TimeSigCustomDen)
    {
        // append digit to current number
        _lowerNumberString = QString("%1%2").arg(_lowerNumberString, QString::number(digit));

        emit ratioInputChanged(_upperNumberString.toInt(), _lowerNumberString.toInt());
    }

    if (_currentState == ODLAStates::TempoEditing ||
        _currentState == ODLAStates::GotoMeasure ||
        _currentState == ODLAStates::AddMeasures)
    {
        // append digit to current number
        _textInput = QString("%1%2").arg(_textInput, QString::number(digit));
        VoiceOver::instance()->say(_textInput);
        emit textInputChanged(_textInput);
    }

    if (_currentState == ODLAStates::SelectRangeFrom)
    {
        // append digit to current number
        _upperNumberString = QString("%1%2").arg(_upperNumberString, QString::number(digit));

        emit ratioInputChanged(_upperNumberString.toInt(), _lowerNumberString.toInt());
    }

    if (_currentState == ODLAStates::SelectRangeTo)
    {
        // append digit to current number
        _lowerNumberString = QString("%1%2").arg(_lowerNumberString, QString::number(digit));

        emit ratioInputChanged(_upperNumberString.toInt(), _lowerNumberString.toInt());
    }
}

/*!
 * \brief ControlPanel::clearNumericInputField
 */
void ControlPanel::clearNumericInputField()
{
    _upperNumberString.clear();
    _lowerNumberString.clear();

    _tempoEditing = false;
    _textInput.clear();
}

/*!
 * \brief ControlPanel::on_action_Settings_triggered
 */
void ControlPanel::on_action_Settings_triggered()
{
    SettingsDialog dialog;
    dialog.setFixedSize(dialog.sizeHint());

    if (dialog.exec() == QDialog::Accepted)
    {
        _odlaController->setDefaultExtendedFeatureSet(Database::instance()->getDefaultExtendedFeatureSet());

        Musescore* app = dynamic_cast<Musescore*>(_odlaController->scoreApp());
        if (app)
        {
            if (Database::instance()->getUseDefaultTemplate())
                app->setTemplateFilePath("");
            else
                app->setTemplateFilePath(Database::instance()->getMusescoreTemplateFilePath());

            app->setFileExtension(Database::instance()->getMusescoreFileExt());
            app->setDefaultClef(Database::instance()->getDefaultClef());
            app->setDefaultDuration(Database::instance()->getDefaultNoteValue());
            app->setDefaultTimeBeats(Database::instance()->getDefaultTimeBeats());
            app->setDefaultTimeUnit(Database::instance()->getDefaultTimeUnit());
            app->setComposerName(Database::instance()->getDefaultComposer());

//            bool canCollapse = ODLASettings::instance()->getInputPanelCollapsing();
//            if (!canCollapse)
//                ui->pinButton->setChecked(true);
        }
    }
}


/*!
 * \brief ControlPanel::on_action_About_triggered
 */
void ControlPanel::on_action_About_triggered()
{
    #pragma message(VERSION)
    QMessageBox *msgBox = new QMessageBox;
    msgBox->setParent(nullptr);
    msgBox->setWindowTitle(tr("About"));
    msgBox->setText(QString("%1<br>%2 %3<br><br>%4").arg(
                           PRODUCT_NAME,
                           tr("Version"),
                           VERSION,
                           tr("Learn more at <a href=\"https://www.odlamusic.com\">odlamusic.com</a>")));
    msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox->show();
}


/*!
 * \brief ControlPanel::setCurrentSelectedKeyIdx
 * \param newValue
 *
 * Set new value in selection setting new limits
 */
void ControlPanel::setCurrentSelectedKeyIdx(int newValue, int maxOptionIndex)
{
    if(maxOptionIndex != -1)
        _currentMaxOptionIndex = maxOptionIndex;

    int firstOfPage = (newValue / 9) * 9;

    if(newValue < 0 || newValue > _currentMaxOptionIndex)
    {
        if(firstOfPage < 0 || firstOfPage > _currentMaxOptionIndex)
            return;
    }
    else
        _absCurrentSelectedKeyIdx = newValue;

    _odlaController->setCurrentSelectedKeyIdx(_absCurrentSelectedKeyIdx);
    setCurrentPage();
}

/*!
 * \brief ControlPanel::setCurrentPage
 * \param currentPage
 */
void ControlPanel::setCurrentPage()
{
    int maxPageIndex = int(_currentMaxOptionIndex/9);
    _currentPage = int(_absCurrentSelectedKeyIdx/9);

    // check page boundaries
    if (_currentPage < 0)
        _currentPage = 0;

    if (maxPageIndex >= 0)
    {
        if (_currentPage > maxPageIndex)
            _currentPage = maxPageIndex;
    }

    emit currentPageChanged(_currentPage);
}

/*!
 * \brief ControlPanel::hide
 */
void ControlPanel::silentHide()
{
    _optionSubMenuEntered = false;
    _currentState = ODLAStates::Idle;
    _currentActiveWindow = activeInputType::None;
    clearNumericInputField();
    _odlaController->silentHide();

    // collapse option panel if exapanded, can collapse and
    // no other input is expected, i.e. idling
    if (isVisible() && !_neverCollapse)
    {
        toggleCollapseAnimation();
    }
    hide();
}

/*!
 * \brief ControlPanel::on_action_Close_triggered
 */
void ControlPanel::on_action_Close_triggered()
{
    if (_askForConfirmOnClosing)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, tr("Confirm"),
                                                                   tr("Do you really want to close ODLA?"),
                                                                   QMessageBox::Yes | QMessageBox::Cancel);

        if (resBtn == QMessageBox::Yes)
        {
            QCoreApplication::quit();
        }
    }
    else
    {
        QCoreApplication::quit();
    }
}

/*!
 * \brief ControlPanel::initStringMaps
 */
void ControlPanel::initStringMaps()
{
    _statesMap.clear();
    _statesMap.insert(ODLAStates::Idle, tr("Waiting for input."));
    _statesMap.insert(ODLAStates::Canceled, tr("Operation canceled."));
    _statesMap.insert(ODLAStates::Error, tr("Invalid option"));
    _statesMap.insert(ODLAStates::Menu, tr("Menu"));
    _statesMap.insert(ODLAStates::Goto, tr("Go to"));
    _statesMap.insert(ODLAStates::GotoMeasure, tr("Digit measure number"));
    _statesMap.insert(ODLAStates::Clefs, tr("Clefs"));
    _statesMap.insert(ODLAStates::Voices, tr("Voices"));
    _statesMap.insert(ODLAStates::Tuplet1, tr("Tuplets"));
    _statesMap.insert(ODLAStates::Tuplet2, tr("Define tuplet divisions."));
    _statesMap.insert(ODLAStates::TimeSig, tr("Time signatures"));
    _statesMap.insert(ODLAStates::TimeSigCustomNum, tr("Enter numerator"));
    _statesMap.insert(ODLAStates::SelectRangeFrom, tr("Enter measure from"));
    _statesMap.insert(ODLAStates::SelectMeasure, tr("Select measures"));
    _statesMap.insert(ODLAStates::KeySig, tr("Key signatures"));
    _statesMap.insert(ODLAStates::NaturalKeySig, tr("Natural key signatures"));
    _statesMap.insert(ODLAStates::FlatKeySig, tr("Flat key signatures"));
    _statesMap.insert(ODLAStates::SharpKeySig, tr("Sharp key signatures"));
    _statesMap.insert(ODLAStates::Bars, tr("Bar lines and repeat signs"));
    _statesMap.insert(ODLAStates::Intervals, tr("Double notes"));
    _statesMap.insert(ODLAStates::IntervalsAbove, tr("Intervals above"));
    _statesMap.insert(ODLAStates::IntervalsBelow, tr("Intervals below"));
    _statesMap.insert(ODLAStates::Options, tr("Additional features"));
    _statesMap.insert(ODLAStates::Dynamics, tr("Dynamics"));
    _statesMap.insert(ODLAStates::Tremolo, tr("Tremolo"));
    _statesMap.insert(ODLAStates::Articulations, tr("Articulations"));
    _statesMap.insert(ODLAStates::Ornaments, tr("Ornaments"));
    //_statesMap.insert(ODLAStates::BreathsAndPauses, tr("Breaths and pauses"));
    _statesMap.insert(ODLAStates::NoteHeads, tr("Note heads"));
    _statesMap.insert(ODLAStates::GraceNotes, tr("Grace notes"));
    _statesMap.insert(ODLAStates::Lines, tr("Lines"));
    _statesMap.insert(ODLAStates::Fingering, tr("Fingering"));
    _statesMap.insert(ODLAStates::Arpeggios, tr("Arpeggios and glissandi"));
    _statesMap.insert(ODLAStates::Tempo, tr("Metronome indication"));
    _statesMap.insert(ODLAStates::TempoEditing, tr("Tempo indication (bmp)"));
    _statesMap.insert(ODLAStates::File, tr("File"));
    _statesMap.insert(ODLAStates::Edit, tr("Edit score"));
    _statesMap.insert(ODLAStates::Tools, tr("Advanced tools"));
    _statesMap.insert(ODLAStates::AddMeasures, tr("Digit measures number"));
    _statesMap.insert(ODLAStates::Text, tr("Text"));

    _optMap.clear();
    _optMap.insert(ODLAStates::Menu, QStringList() << tr("File") << tr("Edit") << tr("Quit")); //<< tr("Tools")
    _optMap.insert(ODLAStates::Goto, QStringList() << tr("Beginning") << tr("End") << tr("Measure"));
    _optMap.insert(ODLAStates::Clefs, Metadata::clefNames().values());
    _optMap.insert(ODLAStates::TimeSig, Metadata::timeSignaturesNames().values());
    _optMap.insert(ODLAStates::Voices, QStringList() << tr("Voice 1") << tr("Voice 2") << tr("Voice 3") << tr("Voice 4"));
    QStringList noteValues = Metadata::noteValuesNames().values();
    std::reverse(noteValues.begin(), noteValues.end());
    _optMap.insert(ODLAStates::Tuplet1, Metadata::tupletNames());
//    _optMap.insert(ODLAStates::Tuplet1, noteValues);
//    _optMap.insert(ODLAStates::Tuplet2, Metadata::tupletNames()); // TODO: make this tuplet 1 and remove tuplet 2
    _optMap.insert(ODLAStates::KeySig, QStringList()
                   << Metadata::accidentalsNames().values().at(1)   // sharp
                   << Metadata::accidentalsNames().values().at(0)   // natural
                   << Metadata::accidentalsNames().values().at(2)); // flat
    _optMap.insert(ODLAStates::FlatKeySig, Metadata::flatKeySignaturesNames());
    _optMap.insert(ODLAStates::SharpKeySig, Metadata::sharpKeySignaturesNames());
    _optMap.insert(ODLAStates::Bars, Metadata::barlineNames().values());
    _optMap.insert(ODLAStates::Intervals, QStringList() << tr("Above") << tr("Below"));
    _optMap.insert(ODLAStates::IntervalsAbove, Metadata::intervalAboveNames().values());
    QStringList tmp = Metadata::intervalBelowNames().values();
    std::reverse(tmp.begin(), tmp.end());
    _optMap.insert(ODLAStates::IntervalsBelow, tmp);

    // the order in the list must match the order in the enum ExtendedFeature
    _optMap.insert(ODLAStates::Options, QStringList()
                   << tr("Dynamics")
                   << tr("Tremolo")
                   << tr("Articulations")
                   << tr("Ornaments")
//                   << tr("Breaths, pauses")
                   << tr("Note heads")
                   << tr("Grace notes")
                   << tr("Lines")
                   << tr("Fingering")
                   << tr("Arpeggios, glissandi")
                   << tr("Metronome indication")
                   << tr("Transpose")
                   << tr("Text")
                   );
    _optMap.insert(ODLAStates::Dynamics, Metadata::dynamicsNames().values());
    _optMap.insert(ODLAStates::Tremolo, Metadata::tremoloNames().values());
    _optMap.insert(ODLAStates::Articulations, Metadata::articulationsNames().values());
    _optMap.insert(ODLAStates::Ornaments, Metadata::ornamentsNames().values());
//    _optMap.insert(ODLAStates::BreathsAndPauses, Metadata::breathsAndPausesNames().values());
    _optMap.insert(ODLAStates::NoteHeads, Metadata::noteHeadsNames().values());
    _optMap.insert(ODLAStates::GraceNotes, Metadata::graceNotesNames().values());
    _optMap.insert(ODLAStates::Lines, Metadata::linesNames().values());
    _optMap.insert(ODLAStates::Fingering, Metadata::fingeringNames().values());
    _optMap.insert(ODLAStates::Arpeggios, Metadata::arpeggioNames().values());
    _optMap.insert(ODLAStates::Tempo, Metadata::tempoNames().values());
    _optMap.insert(ODLAStates::File, QStringList() << tr("New") << tr("Open") << tr("Save") << tr("Save as") << tr("Export") << tr("Close") << tr("Print") << tr("Page format") << tr("Extract parts"));
    _optMap.insert(ODLAStates::Edit, QStringList() << tr("Instruments") << tr("Page view") << tr("Line view") << tr("Loop|Loop") << tr("Mixer") << tr("Play panel") << tr("Insert measures")<< tr("Delete measures")); //<< tr("Concert pitch") << tr("Selection filter")
    _optMap.insert(ODLAStates::Tools, QStringList() << tr("Zoom in") << tr("Zoom out") << tr("Transpose") << tr("Explode") << tr("Implode") << tr("Fill slashes") << tr("Toggle slashes") << tr("Image capture"));
    _optMap.insert(ODLAStates::SelectMeasure, QStringList() << tr("All measures") << tr("From range"));
    _optMap.insert(ODLAStates::Text, QStringList() << tr("Staff text")<< tr("Text below staff") << tr("Chord text") << tr("Lyrics"));
}


