#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "database.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <cmath>

/*!
 * \brief SettingsDialog::SettingsDialog
 * \param parent
 */
SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);
    initGeneralOptions();
    initScoreOptions();
    initMuseScoreOptions();
}

/*!
 * \brief SettingsDialog::~SettingsDialog
 */
SettingsDialog::~SettingsDialog()
{
    delete ui;
}

/*!
 * \brief SettingsDialog::changeEvent
 * \param event
 */
void SettingsDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {   
        ui->retranslateUi(this);

        //initGeneralOptions();

        int clefIdx = ui->clefComboBox->currentIndex();
        int noteIdx = ui->durationComboBox->currentIndex();
        initScoreOptions();

        ui->clefComboBox->setCurrentIndex(clefIdx);
        ui->durationComboBox->setCurrentIndex(noteIdx);

        QString tempFile = Database::instance()->getMusescoreTemplateFilePath();
        if (!tempFile.isEmpty())
        {
            QFileInfo info(tempFile);
            ui->templateLabel->setText(info.baseName());
        }
    }
    else
    {
        QWidget::changeEvent(event);
    }
}

/*!
 * \brief SettingsDialog::initGeneralOptions
 */
void SettingsDialog::initGeneralOptions()
{
    // language
    QString lang = Database::instance()->getLanguage();
    int idx = LocalizationManager::supportedLanguages().indexOf(lang);
    if (idx >= 0)
        ui->langComboBox->setCurrentIndex(idx);

    // fn + options
    int optIdx = static_cast<int>(Database::instance()->getDefaultExtendedFeatureSet());
    if (optIdx >= 0 && optIdx < ui->optionComboBox->count())
        ui->optionComboBox->setCurrentIndex(optIdx);

    // panel animation speed
    bool collapseAllowed = Database::instance()->getInputPanelCollapsing();
    ui->pinCheckBox->setChecked(!collapseAllowed);
}

/*!
 * \brief SettingsDialog::initScoreOptions
 */
void SettingsDialog::initScoreOptions()
{
    ui->clefComboBox->clear();
    ui->clefComboBox->addItems(Metadata::clefNames().values());
    ui->clefComboBox->setCurrentIndex(static_cast<int>(Database::instance()->getDefaultClef()) - 1);

    ui->timeSigNumComboBox->setCurrentText(QString::number(Database::instance()->getDefaultTimeBeats()));
    ui->timeSigDenComboBox->setCurrentText(QString::number(Database::instance()->getDefaultTimeUnit()));

    ui->durationComboBox->clear();
    ui->durationComboBox->addItems(Metadata::noteValuesNames().values());
    Durations defDuration = Database::instance()->getDefaultNoteValue();
    int idx = 0;
    if (defDuration != Durations::DOUBLE_WHOLE)
        idx = (int) log2(static_cast<double>(defDuration)) + 1;
    ui->durationComboBox->setCurrentIndex(idx);
}

/*!
 * \brief SettingsDialog::initMuseScoreOptions
 */
void SettingsDialog::initMuseScoreOptions()
{
    ui->templateLabel->clear();
    ui->fileExtComboBox->setCurrentText(Database::instance()->getMusescoreFileExt());
    ui->defaultTemplateButton->setChecked(Database::instance()->getUseDefaultTemplate());
    ui->userTemplateRadioButton->setChecked(!Database::instance()->getUseDefaultTemplate());
    QFileInfo info(Database::instance()->getMusescoreTemplateFilePath());
    ui->templateLabel->setText(info.baseName());
}

/*!
 * \brief SettingsDialog::on_okButton_clicked
 */
void SettingsDialog::on_okButton_clicked()
{
    // apply settings
<<<<<<< HEAD:src/settingsdialog.cpp
    Database::instance()->setLanguage(LocalizationManager::supportedLanguages().at(ui->langComboBox->currentIndex()));
    Database::instance()->setDefaultExtendedFeatureSet(static_cast<ExtendedFeatures>(ui->optionComboBox->currentIndex()));
    Database::instance()->setInputPanelCollapsing(!ui->pinCheckBox->isChecked());
    Database::instance()->setDefaultClef(static_cast<Clefs>(ui->clefComboBox->currentIndex() + 1));
    Database::instance()->setDefaultTimeBeats(ui->timeSigNumComboBox->currentText().toInt());
    Database::instance()->setDefaultTimeUnit(ui->timeSigDenComboBox->currentText().toInt());
    Database::instance()->setDefaultNoteValue(static_cast<Durations>(int(pow(2, ui->durationComboBox->currentIndex() - 1))));

    Database::instance()->setUseDefaultTemplate(ui->defaultTemplateButton->isChecked());
    Database::instance()->setMusescoreFileExt(ui->fileExtComboBox->currentText());

    if (!Database::instance()->getUseDefaultTemplate() &&
        Database::instance()->getMusescoreTemplateFilePath().isEmpty())
=======
    ODLASettings::setLanguage(LocalizationManager::langCode(ui->langComboBox->currentText()));
    ODLASettings::setDefaultExtendedFeatureSet(static_cast<ExtendedFeatures>(ui->optionComboBox->currentIndex()));
    ODLASettings::setInputPanelCollapsing(!ui->pinCheckBox->isChecked());
    ODLASettings::setDefaultClef(static_cast<Clefs>(ui->clefComboBox->currentIndex() + 1));
    ODLASettings::setDefaultTimeBeats(ui->timeSigNumComboBox->currentText().toInt());
    ODLASettings::setDefaultTimeUnit(ui->timeSigDenComboBox->currentText().toInt());
    ODLASettings::setDefaultNoteValue(static_cast<Durations>(int(pow(2, ui->durationComboBox->currentIndex() - 1))));

    ODLASettings::setUseDefaultTemplate(ui->defaultTemplateButton->isChecked());
    ODLASettings::setMusescoreFileExt(ui->fileExtComboBox->currentText());

    if (!ODLASettings::getUseDefaultTemplate() &&
        ODLASettings::getMusescoreTemplateFilePath().isEmpty())
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/settingsdialog.cpp
    {
        QMessageBox::critical(this, tr("Ops..."), tr("Please provide template file for MuseScore."), QMessageBox::Ok);
    }
    else
    {
        accept();
    }
}

/*!
 * \brief SettingsDialog::on_mscoreTemplateButton_clicked
 */
void SettingsDialog::on_mscoreTemplateButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this,
                                 tr("Select a MuseScore template."),
                                 QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                 "MuseScore (*.mscx *.mscz)");

    if (!file.isEmpty())
    {
        Database::instance()->setMusescoreTemplateFilePath(file);

        QFileInfo info(file);
        ui->templateLabel->setText(info.baseName());
    }
}
