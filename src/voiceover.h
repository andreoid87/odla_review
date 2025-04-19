#ifndef VOICEOVER_H
#define VOICEOVER_H

#include <QObject>
#include <QTextToSpeech>
#include <QThread>
#include <QQueue>
#include <QTimer>
#include "database.h"

#define SAY_NOTE_DELAY 400

/*!
 *  \brief VoiceOver class
 *
 *  This class manage vocal guide, it creates a thread because QTextToSpeech it is not optimized
 */
class VoiceOver : public QThread
{
    Q_OBJECT
public:
    /**
     * \brief SpeechField
     *
     * each flags require specific field that has to be queried to musescore
     * to be read by vocal feedback during music insertion e/o music navigation
     */
    enum SpeechField
    {
        NoteName        =   1<<0,   /**< Requires Note name or "pause"  */
        DurationName    =   1<<1,   /**< Requires Duration name         */
        BeatNumber      =   1<<2,   /**< Requires Beat number           */
        MeasureNumber   =   1<<3,   /**< Requires Measure Number        */
        StaffNumber     =   1<<4,   /**< Requires Staff number          */
        TimeSignFraction=   1<<5,   /**< Requires Time signature        */
        ClefName        =   1<<6,   /**< Requires Clef name             */
        KeySignName     =   1<<7,   /**< Requires Key signature         */
        VoiceNumber     =   1<<8,   /**< Requires Voice number          */
        BPMNumber       =   1<<9    /**< Requires Beat for minutes value*/
    };
    Q_DECLARE_FLAGS(SpeechFields, SpeechField)
    Q_FLAG(SpeechFields)

    static VoiceOver* instance(QObject *parent = nullptr);
    static const QMap<QString, SpeechField> flagMap;
    SpeechFields getStatusFlags() {return _statusFlag;}
    SpeechFields getCursorFlags() {return _cursorFlag;}
    int getStatusFlagsString() {return _syntActive ? (unsigned int)(getStatusFlags()) : 0;}
    int getCursorFlagsString() {return _syntActive ? (unsigned int)(getCursorFlags()) : 0;}


private:
    explicit VoiceOver(QObject *parent = nullptr);
    void run() override;
    bool getFlag(QString typeAndFlagName)           {return _db->getValue("SPEECH_" + typeAndFlagName, "BOOLEAN_VALUE").toBool();}
    int getPos(QString typeAndFlagName)             {return _db->getButtonPosition("SPEECH_" + typeAndFlagName);}
    float getNumValue(QString typeAndFlagName)      {return _db->getValue("SPEECH_" + typeAndFlagName, "NUMERIC_VALUE").toReal();}
    void setValue(QString typeAndFlagName, QString value)  {_db->setValue("SPEECH_" + typeAndFlagName, "NUMERIC_VALUE", value);}

    static VoiceOver* _instance;
    bool _syntActive;
    QString _toRead;
    QString _toReadAfter;
    QTextToSpeech _synt;
    float _voiceSpeed;
    float _voiceVolume;
    SpeechFields _cursorFlag;
    SpeechFields _statusFlag;
    QMap <QString, int> _cursorPos;
    QMap <QString, int> _statusPos;
    Database * _db;

public slots:
    void say(QString newString);
    void sayAfter(QString newString);
    QString sort(QJsonObject input, bool cursor);
    void toggleFlag(QJsonObject flagName) {getFlag(flagName["value"].toString()) ?  clearFlag(flagName) : setFlag(flagName);}
    void setFlag(QJsonObject flagName);
    void clearFlag(QJsonObject flagName);
    void changeSpeed(QJsonObject deltaString);
    void changeVolume(QJsonObject deltaString);
    void toggleActivation();
    void setLanguage();
    QString cardinalToOrdinal(QString string);
    QString readableFraction(QString string);

protected slots:
    QString pitchToNoteName(int tpc, int pitch);
    QString durationDecode(int type, int dots);
    QString keySignatureDecode(int ks);
    QString clefDecode(int posYpitch60);
    QJsonObject ms4Translate(QJsonObject input);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(VoiceOver::SpeechFields)

#endif // VOICEOVER_H
