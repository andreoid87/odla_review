#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <algorithm>
#include <QMessageBox>
#include <QFontDatabase>
#include <QButtonGroup>
#include <QShortcut>
#include <QCloseEvent>
#include "musescoreplugin.h"

/*!
 * \brief MainWindow::MainWindow
 * \param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->menuBar->setVisible(false);

    _odlaCtrl = nullptr;

    // status bar
    ui->statusBar->addPermanentWidget(&_appStatusLabel, 0);
    _appStatusLabel.setText(tr("No score application detected."));

    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (auto button : buttons)
    {
        connect(button, &QPushButton::pressed, this, [this, button] () {
            ui->statusBar->showMessage(QString("%1 %2.").arg(button->accessibleName(), tr("pressed")), 1000);
        });
    }

    // use built-in fonts to setup UI labels
    //setupUISymbols();

    // set up shortcuts for modifiers
    setupUIShortCuts();
}

/*!
 * \brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * \brief MainWindow::closeEvent
 * \param event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    emit closingMainWindow();
    event->accept();
}

/*!
 * \brief MainWindow::changeEvent
 * \param event
 */
void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    else
    {
        QWidget::changeEvent(event);
    }
}

/*!
 * \brief MainWindow::scoreAppStatusChanged
 * \param message
 */
void MainWindow::scoreAppStatusChanged(QString message)
{
    _appStatusLabel.setText(message);
}

/*!
 * \brief MainWindow::on_action_MuseScore_Mode_triggered
 * \param checked
 */
void MainWindow::on_action_MuseScore_Mode_triggered(bool checked)
{
    if (checked)
    {
        // start musescore plugin
        MuseScorePlugin *app = new MuseScorePlugin(this);

        connect(app, &MuseScorePlugin::museScoreAppConnected, this, [this] () {
            scoreAppStatusChanged(QString(tr("MuseScore application detected.")));
        });

        connect(app, &MuseScorePlugin::museScoreAppDisonnected, this, [this] () {
            scoreAppStatusChanged(QString(tr("No score application detected.")));
        });

        connect(app, &MuseScorePlugin::museScoreAppMessageReceived,
                this, &MainWindow::scoreAppStatusChanged);

        connect(app, &MuseScorePlugin::accessibleMessageAvailable,
                ui->descriptionLabel, &QLabel::setText);

        if (_odlaCtrl != nullptr)
        {
            _odlaCtrl->setScoreApp(app);
            connect(app, &MuseScorePlugin::accessibleMessageAvailable, _odlaCtrl, &ODLAControllerV2::statusMessageReady);
        }
    }
}

/*!
 * \brief MainWindow::setODLAController
 * \param odlaCtrl
 */
void MainWindow::setODLAController(ODLAControllerV2* odlaCtrl)
{
    _odlaCtrl = odlaCtrl;

    if (_odlaCtrl != nullptr)
    {
        // get staff buttons sorted by y position and
        QList<QPushButton*> staffButtons = ui->staffFrame->findChildren<QPushButton*>();
        std::sort(staffButtons.begin(), staffButtons.end(), [] (const QPushButton* b1, const QPushButton* b2) -> bool {
            return b1->y() < b2->y();
        });

        // map line number to staff buttons from -5 to 13
        int lineid = -5;
        for (auto button : staffButtons)
        {
            connect(button, &QPushButton::clicked, this, [this, lineid] () {
                _odlaCtrl->onStaffKeyPressed(lineid);
            });
            lineid++;
        }

        // get blocknum buttons and map numbers
        QList<QPushButton*> numberButtons = ui->blockNumFrame->findChildren<QPushButton*>();
        for (auto button : numberButtons)
        {
            bool isNumberButton = false;
            int number = button->objectName().remove("button").toInt(&isNumberButton);
            if (isNumberButton)
            {
                connect(button, &QPushButton::clicked, this, [this, number] () {
                    _odlaCtrl->onNumberKeyPressed(number);
                });
                lineid++;
            }
        }

        // connect UI elements with controller V2
        connect(ui->menuButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onMenuKeyPressed);
        connect(ui->gotoButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onGotoKeyPressed);
        connect(ui->optionsButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onOptionsKeyPressed);
        connect(ui->questionButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onQuestionKeyPressed);
        connect(ui->helpButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onHelpKeyPressed);
        connect(ui->copyButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onCopyKeyPressed);
        connect(ui->pasteButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onPasteKeyPressed);
        connect(ui->upTransposeButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onPlusKeyPressed);
        connect(ui->downTransposeButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onMinusKeyPressed);
        connect(ui->flatAccButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onFlatKeyPressed);
        connect(ui->sharpAccButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onSharpKeyPressed);
        connect(ui->naturalAccButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onNaturalKeyPressed);
        connect(ui->dotButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onDotKeyPressed);
        connect(ui->slurButton, &QToolButton::toggled, this, [this] (bool checked) {
            if (checked)
                _odlaCtrl->onSlurKeyPressed();
            else
                _odlaCtrl->onSlurKeyReleased();
        });
        connect(ui->clefButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onClefKeyPressed);
        connect(ui->voiceButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onVoiceKeyPressed);
        connect(ui->functionButton1, &QToolButton::toggled, this, [this] (bool checked) {
            if (checked)
                _odlaCtrl->onFunctionKeyPressed();
            else
                _odlaCtrl->onFunctionKeyReleased();
        });
        connect(ui->tupletButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onTupletKeyPressed);
        connect(ui->keySignatureButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onKeySignatureKeyPressed);
        connect(ui->timeButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onTimeSignatureKeyPressed);
        connect(ui->chordButton, &QToolButton::toggled, this, [this] (bool checked) {
            if (checked)
                _odlaCtrl->onChordKeyPressed();
            else
                _odlaCtrl->onChordKeyReleased();
        });
        connect(ui->barsButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onBarKeyPressed);
        connect(ui->intervalButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onIntervalKeyPressed);
        connect(ui->selectButton, &QToolButton::toggled, this, [this] (bool checked) {
            if (checked)
                _odlaCtrl->onSelectKeyPressed();
            else
                _odlaCtrl->onSelectKeyReleased();
        });
        connect(ui->playButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onPlayKeyPressed);
        connect(ui->pauseButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onPauseKeyPressed);
        connect(ui->metronomeButton, &QPushButton::toggled, _odlaCtrl, &ODLAControllerV2::onMetronomeKeyPressed);
        connect(ui->arrowUpButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onArrowUpKeyPressed);
        connect(ui->arrowDownButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onArrowDownKeyPressed);
        connect(ui->arrowLeftButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onArrowLeftKeyPressed);
        connect(ui->arrowRightButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onArrowRightKeyPressed);
        connect(ui->enterButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onEnterKeyPressed);
        connect(ui->cancelButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onCancelKeyPressed);
        connect(ui->undoButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onUndoKeyPressed);
        connect(ui->redoButton, &QPushButton::clicked, _odlaCtrl, &ODLAControllerV2::onRedoKeyPressed);
    }
}

/*!
 * \brief MainWindow::setAboutAction
 * \param about
 * \return
 */
void MainWindow::setAboutAction(QAction* about)
{
    ui->menuAbout->addAction(about);
}

/*!
 * \brief MainWindow::setupUISymbols
 */
void MainWindow::setupUISymbols()
{
    // application font
    QFont bravura;
    int id1 = QFontDatabase::addApplicationFont(":/Bravura.otf");
    if (id1 >= 0)
        bravura = QFontDatabase::applicationFontFamilies(id1).at(0);

    QFont fontAwsome;
    int id2 = QFontDatabase::addApplicationFont(":/fa-regular-400.otf");
    if (id2 >= 0)
        fontAwsome = QFontDatabase::applicationFontFamilies(id2).at(0);

    QFont fontAwsome14(fontAwsome);
    fontAwsome14.setPointSize(14);
    QFont bravura18(bravura);
    bravura18.setPointSize(18);
    QFont bravura14(bravura);
    bravura14.setPointSize(14);

    // play controls
    ui->playButton->setFont(fontAwsome14);
    ui->playButton->setText("\uF04B");
    ui->pauseButton->setFont(fontAwsome14);
    ui->pauseButton->setText("\uF04C");

    // blocknum modifiers
    ui->timeButton->setFont(bravura18);
    ui->timeButton->setText("\uE08A");
    ui->slurButton->setFont(bravura18);
    ui->slurButton->setText("\uE552");
    ui->clefButton->setFont(bravura);
    ui->clefButton->setText("\uE050");
    ui->voiceButton->setFont(fontAwsome);
    ui->voiceButton->setText("\uF039");
    ui->tupletButton->setFont(bravura);
    ui->tupletButton->setText("\u266B");
    ui->intervalButton->setFont(bravura);
    ui->intervalButton->setText("\uECA5\uECA6");

    // copy/paste
    ui->copyButton->setFont(fontAwsome14);
    ui->copyButton->setText("\uF0C5");
    ui->pasteButton->setFont(fontAwsome14);
    ui->pasteButton->setText("\uF0EA");

    // undo/redo
    ui->undoButton->setFont(fontAwsome14);
    ui->undoButton->setText("\uF0E2");
    ui->redoButton->setFont(fontAwsome14);
    ui->redoButton->setText("\uF01E");

    // enter/cancel
    ui->enterButton->setFont(fontAwsome14);
    ui->enterButton->setText("\uF00C");
    ui->cancelButton->setFont(fontAwsome14);
    ui->cancelButton->setText("\uF00D");

    // help/question
    ui->questionButton->setFont(fontAwsome14);
    ui->questionButton->setText("\uF128");
    ui->helpButton->setFont(fontAwsome14);
    ui->helpButton->setText("\uF129");

    // accidentals
    ui->flatAccButton->setFont(bravura18);
    ui->flatAccButton->setText(QString::fromUtf8("\u266D"));
    ui->naturalAccButton->setFont(bravura18);
    ui->naturalAccButton->setText("\uE261");
    ui->sharpAccButton->setFont(bravura18);
    ui->sharpAccButton->setText("\uE262");

    // durations
    ui->button0->setFont(bravura14);
    ui->button0->setText("\uF45D \uE4E5");
    ui->button1->setFont(bravura14);
    ui->button1->setText("\uF45E \uECAD");
    ui->button2->setFont(bravura14);
    ui->button2->setText("\uF45F \uECAB");
    ui->button3->setFont(bravura14);
    ui->button3->setText("\uF460 \uECA9");
    ui->button4->setFont(bravura14);
    ui->button4->setText("\uF461 \uECA7");
    ui->button5->setFont(bravura14);
    ui->button5->setText("\uF462 \uECA5");
    ui->button6->setFont(bravura14);
    ui->button6->setText("\uF463 \uECA3");
    ui->button7->setFont(bravura14);
    ui->button7->setText("\uF464 \uECA2");
    ui->button8->setFont(bravura14);
    ui->button8->setText("\uF465 \uECA0");
    ui->button9->setFont(bravura14);
    ui->button9->setText("\uF466");

    // transpose controls
    ui->upTransposeButton->setFont(fontAwsome14);
    ui->upTransposeButton->setText("\uF067");
    ui->downTransposeButton->setFont(fontAwsome14);
    ui->downTransposeButton->setText("\uF068");
}

/*!
 * \brief MainWindow::setupUIShortCuts
 */
void MainWindow::setupUIShortCuts()
{
    // modifiers
    QShortcut* f1Shortcut = new QShortcut(QKeySequence(Qt::Key_F), ui->functionButton1);
    connect(f1Shortcut, &QShortcut::activated, ui->functionButton1, &QToolButton::toggle);
    QShortcut* clefShortcut = new QShortcut(QKeySequence(Qt::Key_C), ui->clefButton);
    connect(clefShortcut, &QShortcut::activated, ui->clefButton, &QToolButton::toggle);
    QShortcut* voiceShortcut = new QShortcut(QKeySequence(Qt::Key_V), ui->voiceButton);
    connect(voiceShortcut, &QShortcut::activated, ui->voiceButton, &QToolButton::toggle);
    QShortcut* tupletShortcut = new QShortcut(QKeySequence(Qt::Key_T), ui->tupletButton);
    connect(tupletShortcut, &QShortcut::activated, ui->tupletButton, &QToolButton::click);

    // +/-
    QShortcut* plusShortcut = new QShortcut(QKeySequence(Qt::Key_Control + Qt::Key_Plus), ui->upTransposeButton);
    connect(plusShortcut, &QShortcut::activated, ui->upTransposeButton, &QPushButton::click);
    QShortcut* minusShortcut = new QShortcut(QKeySequence(Qt::Key_Control + Qt::Key_Minus), ui->downTransposeButton);
    connect(minusShortcut, &QShortcut::activated, ui->downTransposeButton, &QPushButton::click);

    plusShortcut->setContext(Qt::ApplicationShortcut);
    minusShortcut->setContext(Qt::ApplicationShortcut);

    // copy/paste
    QShortcut* copyShortcut = new QShortcut(QKeySequence(QKeySequence::Copy), ui->copyButton);
    connect(copyShortcut, &QShortcut::activated, ui->copyButton, &QPushButton::click);
    QShortcut* pasteShortcut = new QShortcut(QKeySequence(QKeySequence::Paste), ui->pasteButton);
    connect(pasteShortcut, &QShortcut::activated, ui->pasteButton, &QPushButton::click);

    // undo/redo
    QShortcut* undoShortcut = new QShortcut(QKeySequence(QKeySequence::Undo), ui->undoButton);
    connect(undoShortcut, &QShortcut::activated, ui->undoButton, &QPushButton::click);
    QShortcut* redoShortcut = new QShortcut(QKeySequence(QKeySequence::Redo), ui->redoButton);
    connect(redoShortcut, &QShortcut::activated, ui->redoButton, &QPushButton::click);

    // numbers
    QShortcut* num0Shortcut = new QShortcut(QKeySequence(Qt::Key_0), ui->button0);
    connect(num0Shortcut, &QShortcut::activated, ui->button0, &QPushButton::click);
    QShortcut* num1Shortcut = new QShortcut(QKeySequence(Qt::Key_1), ui->button1);
    connect(num1Shortcut, &QShortcut::activated, ui->button1, &QPushButton::click);
    QShortcut* num2Shortcut = new QShortcut(QKeySequence(Qt::Key_2), ui->button2);
    connect(num2Shortcut, &QShortcut::activated, ui->button2, &QPushButton::click);
    QShortcut* num3Shortcut = new QShortcut(QKeySequence(Qt::Key_3), ui->button3);
    connect(num3Shortcut, &QShortcut::activated, ui->button3, &QPushButton::click);
    QShortcut* num4Shortcut = new QShortcut(QKeySequence(Qt::Key_4), ui->button4);
    connect(num4Shortcut, &QShortcut::activated, ui->button4, &QPushButton::click);
    QShortcut* num5Shortcut = new QShortcut(QKeySequence(Qt::Key_5), ui->button5);
    connect(num5Shortcut, &QShortcut::activated, ui->button5, &QPushButton::click);
    QShortcut* num6Shortcut = new QShortcut(QKeySequence(Qt::Key_6), ui->button6);
    connect(num6Shortcut, &QShortcut::activated, ui->button6, &QPushButton::click);
    QShortcut* num7Shortcut = new QShortcut(QKeySequence(Qt::Key_7), ui->button7);
    connect(num7Shortcut, &QShortcut::activated, ui->button7, &QPushButton::click);
    QShortcut* num8Shortcut = new QShortcut(QKeySequence(Qt::Key_8), ui->button8);
    connect(num8Shortcut, &QShortcut::activated, ui->button8, &QPushButton::click);
    QShortcut* num9Shortcut = new QShortcut(QKeySequence(Qt::Key_9), ui->button9);
    connect(num9Shortcut, &QShortcut::activated, ui->button9, &QPushButton::click);

    num0Shortcut->setContext(Qt::ApplicationShortcut);
    num1Shortcut->setContext(Qt::ApplicationShortcut);
    num2Shortcut->setContext(Qt::ApplicationShortcut);
    num3Shortcut->setContext(Qt::ApplicationShortcut);
    num4Shortcut->setContext(Qt::ApplicationShortcut);
    num5Shortcut->setContext(Qt::ApplicationShortcut);
    num6Shortcut->setContext(Qt::ApplicationShortcut);
    num7Shortcut->setContext(Qt::ApplicationShortcut);
    num8Shortcut->setContext(Qt::ApplicationShortcut);
    num9Shortcut->setContext(Qt::ApplicationShortcut);

    // arrows
    QShortcut* upShortcut = new QShortcut(QKeySequence(Qt::Key_Up), ui->arrowUpButton);
    connect(upShortcut, &QShortcut::activated, ui->arrowUpButton, &QPushButton::click);
    QShortcut* downShortcut = new QShortcut(QKeySequence(Qt::Key_Down), ui->arrowDownButton);
    connect(downShortcut, &QShortcut::activated, ui->arrowDownButton, &QPushButton::click);
    QShortcut* leftShortcut = new QShortcut(QKeySequence(Qt::Key_Left), ui->arrowLeftButton);
    connect(leftShortcut, &QShortcut::activated, ui->arrowLeftButton, &QPushButton::click);
    QShortcut* rightShortcut = new QShortcut(QKeySequence(Qt::Key_Right), ui->arrowRightButton);
    connect(rightShortcut, &QShortcut::activated, ui->arrowRightButton, &QPushButton::click);

    upShortcut->setContext(Qt::ApplicationShortcut);
    downShortcut->setContext(Qt::ApplicationShortcut);
    leftShortcut->setContext(Qt::ApplicationShortcut);
    rightShortcut->setContext(Qt::ApplicationShortcut);
}
