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
        {"not",  NoteName           },
        {"dur",  DurationName       },
        {"bea",  BeatNumber         },
        {"mea",  MeasureNumber      },
        {"sta",  StaffNumber        },
        {"tim",  TimeSignFraction   },
        {"cle",  ClefName           },
        {"key",  KeySignName        },
        {"voi",  VoiceNumber        },
        {"bpm",  BPMNumber          }
    };

VoiceOver *VoiceOver::instance(QObject *parent)
{
    if(!_instance)
        _instance = new VoiceOver(parent);
    return _instance;
}

VoiceOver::VoiceOver(QObject *parent) : QThread(parent)
{

    _syntActive = false;
    _db = Database::instance(this);

    for(auto &key : flagMap.keys())
    {
        _cursorFlag.setFlag(flagMap[key], getFlag("cursor_" + key));
        _statusFlag.setFlag(flagMap[key], getFlag("status_" + key));
    }
    _synt.setRate(_voiceSpeed = getNumValue("speed"));
    _synt.setVolume(_voiceVolume = getNumValue("volume"));
    Metadata::addCallableInstance(this);
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
    _db->setSetting("speech_" + flagName["value"].toString(), "boolean_value", "1");
    if(splitted.at(0).contains("cursor",Qt::CaseInsensitive))
        _cursorFlag.setFlag(flagMap[splitted.at(1)], true);
    else if(splitted.at(0).contains("status",Qt::CaseInsensitive))
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
    _db->setSetting("speech_" + flagName["value"].toString(), "boolean_value", "0");
    if(splitted.at(0).contains("cursor",Qt::CaseInsensitive))
        _cursorFlag.setFlag(flagMap[splitted.at(1)], false);
    else if(splitted.at(0).contains("status",Qt::CaseInsensitive))
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
    toReturn = string.replace("64", _db->speechText("64th"));
    toReturn = string.replace("32", _db->speechText("32th"));
    toReturn = string.replace("16", _db->speechText("16th"));
    toReturn = string.replace("8",  _db->speechText("8th"));
    toReturn = string.replace("4",  _db->speechText("4th"));
    toReturn = string.replace("2",  _db->speechText("2nd"));
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
    _synt.say(_db->speechText(_syntActive ? "on_voiceover_enabled" : "on_voiceover_disabled"));
}

/*!
 *  \brief VoiceOver::updateLanguage
 *
 *  Method to be called whenever language is changed in order to set correct locale for QTexttospeech object
 */
void VoiceOver::updateLanguage(QString lang)
{
    QLocale locale(lang);
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
        setSetting("speed", QString::number(_voiceSpeed,'f',1));
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
    double delta = deltaString["value"].toDouble();
    qDebug() << deltaString << delta;
    if(_voiceVolume + delta < 1.01 && _voiceVolume + delta >= -0.01)
    {
        _voiceVolume += delta;
        _synt.setVolume(_voiceVolume);
        setSetting("volume", QString::number(_voiceVolume,'f',1));
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

    if(input.take("version").toString() == "ms4")
        input = ms4Translate(input);

    if(!input.value("in").isUndefined())
        input["in"] = _db->speechText(input["in"].toString());
    if(!input.value("elementname").isUndefined())
        input["elementname"] = input["elementname"].toString().replace("_", " ");

    if(!input.value("range").isUndefined()) //Range is selected
        return input["range"].toString().replace("selected from: ", _db->speechText("selected_from")).replace("to: ", _db->speechText("to:"));
    else
    {
        for (auto &key :  input.keys())
        {
            QString tmp;
            if(key == "dur")
                tmp = readableFraction(input["dur"].toString());
            if(key == "tim")
                tmp = readableFraction(input["tim"].toString());
            if(key == "bpm")
                tmp = input["bpm"].toString().replace("beat per minute", _db->speechText("beat_per_minute"));
            else
                tmp = input[key].toString();

            orderedMap[getPos(QString("%1_%2").arg(statusRequested ? "status":"cursor", key))] = tmp;
        }

        for(auto &string : orderedMap)
            tosay.append(string + (string.contains(";") ? "" : "; "));
        return tosay;
    }
    return "";
}

QJsonObject VoiceOver::ms4Translate(QJsonObject input)
{
    if(!input.value("range").isUndefined()) //Range is selected
    {
        QString toSay =
            "selected from: "
            + _db->speechText("staff") + " " + QString::number(input.value("staffstart").toInt()) + ";"
            + _db->speechText("measure") + " " + QString::number(input.value("measurestart").toInt()) + ";"
            + _db->speechText("beat") + " " + QString::number(input.value("beatstart").toInt()) + ";"
                                                                                                  "to: "
            + _db->speechText("staff") + " " + QString::number(input.value("staffend").toInt()) + ";"
            + _db->speechText("measure") + " " + QString::number(input.value("measureend").toInt()) + ";"
            + _db->speechText("beat") + " " + QString::number(input.value("beatend").toInt()) + ";";
        input["range"] = toSay;
        return input;
    }

    if(input.keys().contains("pitch"))
    {
        if(input.keys().contains("unpitched") && input.take("unpitched").toBool(false) == true)
        {
            QString drumsetPitch = QString::number(input.take("pitch").toInt());
            if(drumsetPitch == "-1")
                input["not"] = _db->speechText("rest");
            else
                input["not"] = _db->speechText("drumset_" + drumsetPitch);
        }
        else
        {
            int tpc = input.take("tpc").toInt();
            int pitch = input.take("pitch").toInt();
            input["not"] = pitchToNoteName(tpc, pitch);
        }
    }
    if(input.keys().contains("durationtype"))
        input["dur"] = durationDecode(input.take("durationtype").toInt(), input.take("durationdots").toInt());
    if(input.keys().contains("bea"))
        input["bea"] = _db->speechText("beat") + " " + QString::number(input["bea"].toInt());
    if(input.keys().contains("mea"))
        input["mea"] = _db->speechText("measure") + " " + QString::number(input["mea"].toInt());
    if(input.keys().contains("sta"))
        input["sta"] = _db->speechText("staff") + " " + QString::number(input["sta"].toInt());
    if(input.keys().contains("cle"))
        input["cle"] = clefDecode(input.take("cle").toInt());
    // input["tim"] time signature is already in readable String form
    if(input.keys().contains("durationtype"))
        input["dur"] = durationDecode(input.take("durationtype").toInt(), input.take("durationdots").toInt());
    if(input.keys().contains("bpm"))
        input["bpm"] = QString::number(input["bpm"].toInt()) + " " +_db->speechText("beat_per_minute");
    if(input.keys().contains("voi"))
        input["voi"] = _db->speechText("voice") + " " + QString::number(input["voi"].toInt());

    return input;
}

QString VoiceOver::pitchToNoteName(int tpc, int pitch)
{
    if(pitch == -1)
        return _db->speechText("rest");
    if(pitch == -2)
        return "";

    QString octave = QString::number((pitch / 12) - 1);
    QString name;
    QString accidental;
    switch (tpc % 7)
    {
    case 0: name = _db->speechText("note_c"); break;
    case 1: name = _db->speechText("note_g"); break;
    case 2: name = _db->speechText("note_d"); break;
    case 3: name = _db->speechText("note_a"); break;
    case 4: name = _db->speechText("note_e"); break;
    case 5: name = _db->speechText("note_b"); break;
    case 6: name = _db->speechText("note_f"); break;
    }

    if(tpc < 6)
        accidental = _db->speechText("double_flat");
    else if(tpc < 13)
        accidental = _db->speechText("flat");
    else if(tpc < 20)
        accidental = ""; // _db->speechText("natural");
    else if(tpc < 27)
        accidental = _db->speechText("sharp");
    else
        accidental = _db->speechText("double_sharp");

    return name + octave + accidental;
}

QString VoiceOver::durationDecode(int type, int dots)
{
    QString duration;
    switch (type)
    {
    case 0: duration = _db->speechText("longa_note"); break;
    case 1: duration = _db->speechText("breve_note"); break;
    case 2: duration = _db->speechText("semibreve_note"); break;
    case 3: duration = _db->speechText("half_note"); break;
    case 4: duration = _db->speechText("crotchet_note"); break;
    case 5: duration = _db->speechText("quaver_note"); break;
    case 6: duration = _db->speechText("16th_note"); break;
    case 7: duration = _db->speechText("32nd_note"); break;
    case 8: duration = _db->speechText("64th_note"); break;
    }
    if(dots == 1)
        duration += " " + _db->speechText("with_dot");
    else if(dots > 1)
        duration += " " + _db->speechText("with_dots").arg(dots);
    return duration;
}

QString VoiceOver::keySignatureDecode(int ks)
{
    switch (ks)
    {
    case -1:    return _db->speechText("f_major,_d_minor");
    case -2:    return _db->speechText("b_flat_major,_g_minor");
    case -3:    return _db->speechText("e_flat_major,_c_minor");
    case -4:    return _db->speechText("a_flat_major,_f_minor");
    case -5:    return _db->speechText("d_flat_major,_b_flat_minor");
    case -6:    return _db->speechText("g_flat_major,_e_flat_minor");
    case -7:    return _db->speechText("c_flat_major,_a_flat_minor");
    case 0:     return _db->speechText("c_major,_a_minor");
    case 1:     return _db->speechText("g_major,_e_minor");
    case 2:     return _db->speechText("d_major,_b_minor");
    case 3:     return _db->speechText("a_major,_f_sharp_minor");
    case 4:     return _db->speechText("e_major,_c_sharp_minor");
    case 5:     return _db->speechText("b_major,_g_sharp_minor");
    case 6:     return _db->speechText("f_sharp_major,_d_sharp_minor");
    case 7:     return _db->speechText("c_sharp_major,_a_sharp_minor");
    default:    return "";
    }
}

QString VoiceOver::clefDecode(int posYpitch60)
{
    switch(posYpitch60)
    {
    case -80:   return _db->speechText("bass_clef_lower_15th");
    case -45:   return _db->speechText("bass_clef_lower_octave");
    case -20:   return _db->speechText("treble_clef_lower_15th");
    case -10:   return _db->speechText("bass_clef");
    case 0:     return _db->speechText("baritone_clef");
    case 10:    return _db->speechText("tenor_clef");
    case 15:    return _db->speechText("treble_clef_lower_octave");
    case 20:    return _db->speechText("alto_clef");
    case 25:    return _db->speechText("bass_clef_upper_octave");
    case 30:    return _db->speechText("mezzo-soprano_clef");
    case 40:    return _db->speechText("soprano_clef");
    case 50:    return _db->speechText("treble_clef");
    case 60:    return _db->speechText("bass_clef_upper_15th");
    case 85:    return _db->speechText("treble_clef_upper_octave");
    case 120:   return _db->speechText("treble_clef_upper_15th");
    default :   return "";
    }
}
