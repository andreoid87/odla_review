#include "localizationmanager.h"
<<<<<<< HEAD:src/localizationmanager.cpp
#include "database.h"
=======
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/localizationmanager.cpp
#include <QCoreApplication>
#include "voiceover.h"
#include <QDebug>
<<<<<<< HEAD:src/localizationmanager.cpp
LocalizationManager *LocalizationManager::_instance;
=======

LocalizationManager LocalizationManager::instance;
QStringList LocalizationManager::_availableLanguages;
QTranslator LocalizationManager::_qtTranslator;
QTranslator LocalizationManager::_qtBaseTranslator;
QTranslator LocalizationManager::_odlaTranslator;
QString LocalizationManager::_currentLang;
QLocale LocalizationManager::_locale;
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/localizationmanager.cpp

/*!
 * \brief LocalizationManager::LocalizationManager
 * \param parent
 */
LocalizationManager::LocalizationManager(QObject *parent) : QObject(parent)
{
}

bool LocalizationManager::isLanguageSupported(QString lang)
{
    foreach (const QString& var, supportedLanguages())
        if(var == lang) return true;
    return false;
}

/*!
 * \brief LocalizationManager::onLanguageChanged
 * \param lang
 */
void LocalizationManager::setLanguage(QString lang)
{
    //language is a lowercase, two-letter, ISO 639 language code (also some three-letter codes)
    _locale = QLocale(lang);
    _currentLang = lang;

    if (!_qtTranslator.isEmpty())
        qApp->removeTranslator(&_qtTranslator);
    _qtTranslator.load(":/lang/qt_" + lang + ".qm");
    qApp->installTranslator(&_qtTranslator);

    if (!_qtBaseTranslator.isEmpty())
        qApp->removeTranslator(&_qtBaseTranslator);
    _qtBaseTranslator.load(":/lang/qtbase_" + lang + ".qm");
    qApp->installTranslator(&_qtBaseTranslator);

    if (!_odlaTranslator.isEmpty())
        qApp->removeTranslator(&_odlaTranslator);
    _odlaTranslator.load(":/lang/odla_" + lang + ".qm");
    qApp->installTranslator(&_odlaTranslator);

    QLocale::setDefault(_locale);
<<<<<<< HEAD:src/localizationmanager.cpp
    emit languageChanged(_locale);

    Database::instance()->setLanguage(lang);
    VoiceOver::instance()->setLanguage(_locale);
}

/*!
 *
 * \brief LocalizationManager::getInstance
 * \return singletone instance of this class
 */
LocalizationManager *LocalizationManager::instance()
=======
}

QString LocalizationManager::langCode(QString language)
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/localizationmanager.cpp
{
    if(language == "Italiano")
        return "it";
    else
        return "en";
}

/*!
 * \brief LocalizationManager::supportedLanguages
 * \return
 */
QStringList LocalizationManager::supportedLanguages()
{
    return QStringList() << "en" << "it";
}

/*!
 * \brief LocalizationManager::getCurrentLocale
 */
QLocale LocalizationManager::getCurrentLocale()
{
    return _locale;
}

/*!
 * \brief LocalizationManager::getCurrentLanguage
 */
QString LocalizationManager::getCurrentLanguage()
{
    return _currentLang;
}

QString LocalizationManager::getSystemLanguage()
{
    // system returns something like: en-US, we use only first part of it
    QString lang = QLocale::system().uiLanguages().first().split('-').first();

    //if system language is not currently supported, then will set english
    if(!isLanguageSupported(lang))
        lang = "en";

    return lang;
}
