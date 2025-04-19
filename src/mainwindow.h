#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QMap>
#include "odlacontroller.h"
#include "iscorewriterapp.h"

namespace Ui {
class MainWindow;
}

enum class Clefs : int;
enum class Durations : int;

/*!
 * \brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void changeEvent(QEvent *event) override;

    void setODLAController(ODLAControllerV2* odlaCtrl);
    void setAboutAction(QAction* about);

signals:
    void closingMainWindow();

public slots:
    void scoreAppStatusChanged(QString message);
    void on_action_MuseScore_Mode_triggered(bool checked);

private:
    Ui::MainWindow *ui;
    ODLAControllerV2* _odlaCtrl;
    QLabel _appStatusLabel;

    void setupUISymbols();
    void setupUIShortCuts();
};

#endif // MAINWINDOW_H
