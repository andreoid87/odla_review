#include "voiceover.h"
#include "voiceover.h"
#include "metadata.h"
#include <QDebug>

extern bool isDebug;

VoiceOver* VoiceOver::_instance;

/*!
 *  \brief VoiceOver::flagMap
 *
 *  Maps any flag to a 3 char string
 */
const QMap<QString, VoiceOver::SpeechField> VoiceOver::flagMap
    {
        {"NOT",  NoteName           },
        {"DUR",  DurationName       },
        {"BEA",  BeatNumber         },
        {"MEA",  MeasureNumber      },
        {"STA",  StaffNumber        },
        {"TIM",  TimeSignFraction   },
        {"CLE",  ClefName           },
        {"KEY",  KeySignName        },
        {"VOI",  VoiceNumber        },
        {"BPM",  BPMNumber          }
    };

VoiceOver *VoiceOver::instance(QObject *parent)
{
    if(!_instance)
        _instance = new VoiceOver(parent);
    Metadata::addCallableInstance(_instance);
    return _instance;
}

VoiceOver::VoiceOver(QObject *parent) : QThread(parent)
{
    
    _syntActive = false;
    _db = Database::instance(this);

    for(auto &key : flagMap.keys())
    {
        _cursorFlag.setFlag(flagMap[key], getFlag("CURSOR_" + key));
        _statusFlag.setFlag(flagMap[key], getFlag("STATUS_" + key));
    }
    QLocale locale(_db->getLanguage());
    QLocale::setDefault(locale);
    _synt.setLocale(locale);
    _synt.setRate(_voiceSpeed = getNumValue("SPEED"));
    _synt.setVolume(_voiceVolume = getNumValue("VOLUME"));
}

/*!
 * \brief VoiceOver::say
 *
 * \param newString (string to read)
 *
 * Save the string and starts the thread to be read ASAP
 */
void VoiceOver::say(QString newString)
{
    if(!_syntActive || newString.isEmpty()) return;
    _toRead = newString;
    start();
}

/*!
 * \brief VoiceOver::sayAfter
 *
 * \param newString (string to read)
 *
 * Save the string and say it immediatly if none saying
 * if not say it at end of currently
 */
void VoiceOver::sayAfter(QString newString)
{
    if(!_syntActive || newString.isEmpty()) return;
    _toReadAfter =  newString;
    start();
}

/*!
 * \brief VoiceOver::setFlag
 * \param flagName
 *
 * Set the flags to require specific field from musescore
 */
void VoiceOver::setFlag(QJsonObject flagName)
{
    if(isDebug)
        qDebug() << "setting" << flagName;
    auto splitted = flagName["value"].toString().split("_");
    if(splitted.size() != 2) return;
    _db->setValue("SPEECH_" + flagName["value"].toString(), "BOOLEAN_VALUE", "1");
    if(splitted.at(0).contains("CURSOR",Qt::CaseInsensitive))
        _cursorFlag.setFlag(flagMap[splitted.at(1)], true);
    else if(splitted.at(0).contains("STATUS",Qt::CaseInsensitive))
        _statusFlag.setFlag(flagMap[splitted.at(1)], true);
}

/*!
 * \brief VoiceOver::clearFlag
 * \param flagName
 *
 * Clear the flags to require specific field from musescore
 */
void VoiceOver::clearFlag(QJsonObject flagName)
{
    if(isDebug)
        qDebug() << "clear" << flagName;
    auto splitted = flagName["value"].toString().split("_");
    if(splitted.size() != 2) return;
    _db->setValue("SPEECH_" + flagName["value"].toString(), "BOOLEAN_VALUE", "0");
    if(splitted.at(0).contains("CURSOR",Qt::CaseInsensitive))
        _cursorFlag.setFlag(flagMap[splitted.at(1)], false);
    else if(splitted.at(0).contains("STATUS",Qt::CaseInsensitive))
        _statusFlag.setFlag(flagMap[splitted.at(1)], false);
}

/*!
 *  \brief VoiceOver::cardinalToOrdinal
 *  \par string
 *
 *  Replace string containing power of two digit until 64
 *  with their ordinal word (?)
 */
QString VoiceOver::cardinalToOrdinal(QString string)
{
    QString toReturn;
    toReturn = string.replace("64", _db->speechText("64TH"));
    toReturn = string.replace("32", _db->speechText("32TH"));
    toReturn = string.replace("16", _db->speechText("16TH"));
    toReturn = string.replace("8",  _db->speechText("8TH"));
    toReturn = string.replace("4",  _db->speechText("4TH"));
    toReturn = string.replace("2",  _db->speechText("2ND"));
    return toReturn;
}

/*!
 *  \brief VoiceOver::readableFraction
 *  \par string
 *
 *  Makes fraction readable, replacing denominator with ordinal string
 */
QString VoiceOver::readableFraction(QString string)
{
    QString finalString = string;
    QRegularExpression re("/(\\d+)");
    QRegularExpressionMatchIterator i = re.globalMatch(string);
    if(!i.hasNext())
        return finalString;
    // If function detected call and replace it with its return value
    if (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        QString denominator = match.captured(1);
        finalString.replace("/"+denominator, " "+cardinalToOrdinal(denominator));
    }
    return finalString;
}

/*!
 * \brief VoiceOver::toggleActivation
 *
 * Toggle activation of voice over
 */
void VoiceOver::toggleActivation()
{
    _syntActive = !_syntActive;
    _synt.say(_db->speechText(_syntActive ? "ON_VOICEOVER_ENABLED" : "ON_VOICEOVER_DISABLED"));
}

/*!
 *  \brief VoiceOver::setLanguage
 *
 *  Method to be called whenever language is changed in order to set correct locale for QTexttospeech object
 */
void VoiceOver::setLanguage()
{
    QLocale locale(_db->getLanguage());
    QLocale::setDefault(locale);
    _synt.setLocale(locale);
}

/*!
 * \brief VoiceOver::changeSpeed
 * \par deltaString
 *
 * Slot called by database (with meta-call support), it can increase or
 * decrease voice speed by a value of 0.1 from 0 to 1
 */
void VoiceOver::changeSpeed(QJsonObject deltaString)
{
    double delta = deltaString["value"].toDouble();
    if(_voiceSpeed + delta < 1.01 && _voiceSpeed + delta >= -0.01)
    {
        _voiceSpeed += delta;
        _synt.setRate(_voiceSpeed);
        setValue("SPEED", QString::number(_voiceSpeed,'f',1));
    }
}

/*!
 * \brief VoiceOver::changeVolume
 * \par deltaString
 *
 * Slot called by database (with meta-call support), it can increase or
 * decrease voice volume by a value of 0.1 from 0 to 1 (setVolume?)
 */
void VoiceOver::changeVolume(QJsonObject deltaString)
{
    double delta = deltaString["delta"].toDouble();
    qDebug() << deltaString << delta;
    if(_voiceVolume + delta < 1.01 && _voiceVolume + delta >= -0.01)
    {
        _voiceVolume += delta;
        _synt.setVolume(_voiceVolume);
        setValue("VOLUME", QString::number(_voiceVolume,'f',1));
    }
}

/*!
 * \brief VoiceOver::run
 *
 * Thread code
 */
void VoiceOver::run()
{
    _synt.say(_toRead + ", " + _toReadAfter);
    if(isDebug)
        qDebug() << tr("saying: %1, %2").arg(_toRead, _toReadAfter);
    _toRead = "";
    _toReadAfter = "";
}

/*!
 * \brief VoiceOver::sortAndSay
 *
 * Sort speech feedback received by musescore and say it
 * The order is defined by buttons position in menu and saved in DB
 */
QString VoiceOver::sort(QJsonObject input, bool statusRequested)
{
    QString tosay;
    QMap<int, QString> orderedMap;

    if(input.take("version").toString() == "MS4")
        input = ms4Translate(input);

    if(!input.value("IN").isUndefined())
        input["IN"] = _db->speechText(input["IN"].toString());
    if(!input.value("elementName").isUndefined())
        input["elementName"] = input["elementName"].toString().replace("_", " ");

    if(!input.value("RANGE").isUndefined()) //Range is selected
        return input["RANGE"].toString().replace("selected from: ", _db->speechText("SELECTED_FROM")).replace("to: ", _db->speechText("TO:"));
    else
    {
        for (auto &key :  input.keys())
        {
            QString tmp;
            if(key == "DUR")
                tmp = readableFraction(input["DUR"].toString());
            if(key == "TIM")
                tmp = readableFraction(input["TIM"].toString());
            if(key == "BPM")
                tmp = input["BPM"].toString().replace("beat per minute", _db->speechText("BEAT_PER_MINUTE"));
            else
                tmp = input[key].toString();

            orderedMap[getPos(QString("%1_%2").arg(statusRequested ? "STATUS":"CURSOR", key))] = tmp;
        }

        for(auto &string : orderedMap)
            tosay.append(string + (string.contains(";") ? "" : "; "));
        return tosay;
    }
    return "";
}

QJsonObject VoiceOver::ms4Translate(QJsonObject input)
{
    if(!input.value("RANGE").isUndefined()) //Range is selected
    {
        QString toSay =
            "selected from: "
            + _db->speechText("STAFF") + " " + QString::number(input.value("staffStart").toInt()) + ";"
            + _db->speechText("MEASURE") + " " + QString::number(input.value("measureStart").toInt()) + ";"
            + _db->speechText("BEAT") + " " + QString::number(input.value("beatStart").toInt()) + ";"
                                                                                                  "to: "
            + _db->speechText("STAFF") + " " + QString::number(input.value("staffEnd").toInt()) + ";"
            + _db->speechText("MEASURE") + " " + QString::number(input.value("measureEnd").toInt()) + ";"
            + _db->speechText("BEAT") + " " + QString::number(input.value("beatEnd").toInt()) + ";";
        input["RANGE"] = toSay;
        return input;
    }

    if(input.keys().contains("pitch"))
    {
        if(input.keys().contains("unpitched") && input.take("unpitched").toBool(false) == true)
        {
            QString drumsetPitch = QString::number(input.take("pitch").toInt());
            if(drumsetPitch == "-1")
                input["NOT"] = _db->speechText("REST");
            else
                input["NOT"] = _db->speechText("DRUMSET_" + drumsetPitch);
        }
        else
        {
            int tpc = input.take("tpc").toInt();
            int pitch = input.take("pitch").toInt();
            input["NOT"] = pitchToNoteName(tpc, pitch);
        }
    }
    if(input.keys().contains("durationType"))
        input["DUR"] = durationDecode(input.take("durationType").toInt(), input.take("durationDots").toInt());
    if(input.keys().contains("BEA"))
        input["BEA"] = _db->speechText("BEAT") + " " + QString::number(input["BEA"].toInt());
    if(input.keys().contains("MEA"))
        input["MEA"] = _db->speechText("MEASURE") + " " + QString::number(input["MEA"].toInt());
    if(input.keys().contains("STA"))
        input["STA"] = _db->speechText("STAFF") + " " + QString::number(input["STA"].toInt());
    if(input.keys().contains("CLE"))
        input["CLE"] = clefDecode(input.take("CLE").toInt());
    // input["TIM"] time signature is already in readable String form
    if(input.keys().contains("durationType"))
        input["DUR"] = durationDecode(input.take("durationType").toInt(), input.take("durationDots").toInt());
    if(input.keys().contains("BPM"))
        input["BPM"] = QString::number(input["BPM"].toInt()) + " " +_db->speechText("BEAT_PER_MINUTE");
    if(input.keys().contains("VOI"))
        input["VOI"] = _db->speechText("VOICE") + " " + QString::number(input["VOI"].toInt());

    return input;
}

QString VoiceOver::pitchToNoteName(int tpc, int pitch)
{
    if(pitch == -1)
        return _db->speechText("REST");
    if(pitch == -2)
        return "";

    QString octave = QString::number((pitch / 12) - 1);
    QString name;
    QString accidental;
    switch (tpc % 7)
    {
    case 0: name = _db->speechText("NOTE_C"); break;
    case 1: name = _db->speechText("NOTE_G"); break;
    case 2: name = _db->speechText("NOTE_D"); break;
    case 3: name = _db->speechText("NOTE_A"); break;
    case 4: name = _db->speechText("NOTE_E"); break;
    case 5: name = _db->speechText("NOTE_B"); break;
    case 6: name = _db->speechText("NOTE_F"); break;
    }

    if(tpc < 6)
        accidental = _db->speechText("DOUBLE_FLAT");
    else if(tpc < 13)
        accidental = _db->speechText("FLAT");
    else if(tpc < 20)
        accidental = ""; // _db->speechText("NATURAL");
    else if(tpc < 27)
        accidental = _db->speechText("SHARP");
    else
        accidental = _db->speechText("DOUBLE_SHARP");

    return name + octave + accidental;
}

QString VoiceOver::durationDecode(int type, int dots)
{
    QString duration;
    switch (type)
    {
    case 0: duration = _db->speechText("LONGA_NOTE"); break;
    case 1: duration = _db->speechText("BREVE_NOTE"); break;
    case 2: duration = _db->speechText("SEMIBREVE_NOTE"); break;
    case 3: duration = _db->speechText("HALF_NOTE"); break;
    case 4: duration = _db->speechText("CROTCHET_NOTE"); break;
    case 5: duration = _db->speechText("QUAVER_NOTE"); break;
    case 6: duration = _db->speechText("16TH_NOTE"); break;
    case 7: duration = _db->speechText("32ND_NOTE"); break;
    case 8: duration = _db->speechText("64TH_NOTE"); break;
    }
    if(dots == 1)
        duration += " " + _db->speechText("WITH_DOT");
    else if(dots > 1)
        duration += " " + _db->speechText("WITH_DOTS").arg(dots);
    return duration;
}

QString VoiceOver::keySignatureDecode(int ks)
{
    switch (ks)
    {
    case -1:    return _db->speechText("F_MAJOR,_D_MINOR");
    case -2:    return _db->speechText("B_FLAT_MAJOR,_G_MINOR");
    case -3:    return _db->speechText("E_FLAT_MAJOR,_C_MINOR");
    case -4:    return _db->speechText("A_FLAT_MAJOR,_F_MINOR");
    case -5:    return _db->speechText("D_FLAT_MAJOR,_B_FLAT_MINOR");
    case -6:    return _db->speechText("G_FLAT_MAJOR,_E_FLAT_MINOR");
    case -7:    return _db->speechText("C_FLAT_MAJOR,_A_FLAT_MINOR");
    case 0:     return _db->speechText("C_MAJOR,_A_MINOR");
    case 1:     return _db->speechText("G_MAJOR,_E_MINOR");
    case 2:     return _db->speechText("D_MAJOR,_B_MINOR");
    case 3:     return _db->speechText("A_MAJOR,_F_SHARP_MINOR");
    case 4:     return _db->speechText("E_MAJOR,_C_SHARP_MINOR");
    case 5:     return _db->speechText("B_MAJOR,_G_SHARP_MINOR");
    case 6:     return _db->speechText("F_SHARP_MAJOR,_D_SHARP_MINOR");
    case 7:     return _db->speechText("C_SHARP_MAJOR,_A_SHARP_MINOR");
    default:    return "";
    }
}

QString VoiceOver::clefDecode(int posYpitch60)
{
    switch(posYpitch60)
    {
    case -80:   return _db->speechText("BASS_CLEF_LOWER_15TH");
    case -45:   return _db->speechText("BASS_CLEF_LOWER_OCTAVE");
    case -20:   return _db->speechText("TREBLE_CLEF_LOWER_15TH");
    case -10:   return _db->speechText("BASS_CLEF");
    case 0:     return _db->speechText("BARITONE_CLEF");
    case 10:    return _db->speechText("TENOR_CLEF");
    case 15:    return _db->speechText("TREBLE_CLEF_LOWER_OCTAVE");
    case 20:    return _db->speechText("ALTO_CLEF");
    case 25:    return _db->speechText("BASS_CLEF_UPPER_OCTAVE");
    case 30:    return _db->speechText("MEZZO-SOPRANO_CLEF");
    case 40:    return _db->speechText("SOPRANO_CLEF");
    case 50:    return _db->speechText("TREBLE_CLEF");
    case 60:    return _db->speechText("BASS_CLEF_UPPER_15TH");
    case 85:    return _db->speechText("TREBLE_CLEF_UPPER_OCTAVE");
    case 120:   return _db->speechText("TREBLE_CLEF_UPPER_15TH");
    default :   return "";
    }
}
