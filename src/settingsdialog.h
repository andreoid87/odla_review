#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "commons.h"
#include "localizationmanager.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    virtual void changeEvent(QEvent *event) override;

private slots:
    void on_okButton_clicked();
    void on_mscoreTemplateButton_clicked();

private:
    Ui::SettingsDialog *ui;

    void initGeneralOptions();
    void initScoreOptions();
    void initMuseScoreOptions();
};

#endif // SETTINGSDIALOG_H
