#ifndef LOCALIZATIONMANAGER_H
#define LOCALIZATIONMANAGER_H

#include "commons.h"
#include <QObject>
#include <QTranslator>
#include <QLocale>

/*!
 * \brief The LocalizationManager class handle language changes
 */
class LocalizationManager : public QObject
{
    Q_OBJECT
public:
<<<<<<< HEAD:src/localizationmanager.h
    static LocalizationManager *instance();
=======
>>>>>>> 8e720e43ca3709044504952c47f15cddf387d0ba:odla/localizationmanager.h
    static QStringList supportedLanguages();
    static QLocale getCurrentLocale();
    static QString getCurrentLanguage();
    static QString getSystemLanguage();
    static void setLanguage(QString lang);
    static QString langCode(QString language);

protected:
    explicit LocalizationManager(QObject *parent = nullptr);
    static bool isLanguageSupported(QString lang);
    static LocalizationManager instance;
    static QStringList _availableLanguages;
    static QTranslator _qtTranslator;
    static QTranslator _qtBaseTranslator;
    static QTranslator _odlaTranslator;
    static QString _currentLang;
    static QLocale _locale;

};

#endif // LOCALIZATIONMANAGER_H
