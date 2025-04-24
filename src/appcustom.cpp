#include "appcustom.h"
#include "metadata.h"
#include "keyboard.h"

AppCustom::AppCustom(QObject *parent) : App(parent)
{
    // _checkStateTimer must fire just once
    _checkStateTimer.setSingleShot(true);
    Metadata::addCallableInstance(this, false);
    QJsonObject wrapper;

    // SCEGLIERE DA DOVE PRENDERE I DATI DEI BOTTONI TOGGLE
    wrapper["value"] = _db->getActiveToggleExButtons("clef_toggle_ex").toInt();
    setClef(wrapper);
    wrapper["value"] = _db->getActiveToggleExButtons("keysig_toggle_ex").toInt();
    setKeysig(wrapper);
}

/*!
 * \brief AppCustom::keyToPitch
 *
 * Convert line number 0 for higher diatonic note 18 for lower note
 * to midi pitch taking in account current clef and key signature
 *
 * where:
 * line = 0 is the note with two ledger lines below
 * line = 18 is the note with two ledger lines above
 *
 * _currentClef is the number of semitones relative to treble clef
 * _currentClef = 0 is treble clef
 * _currentClef = -10 is bass clef
 * _currentClef = 10 is alto clef
 *
 * _currentKeysig is the number of sharps or flats in the key signature
 * _currentKeysig = 0 is C major or A minor
 * _currentKeysig = 1 is G major or E minor
 * _currentKeysig = -1 is F major or D minor
 *
 */
int AppCustom::keyToPitch(int key)
{


}


/*!
 * \brief AppCustom::checkState
 *
 * Slot connected to _checkStateTimer timeout signal emit appConnected(true)
 * since It is not possible to know the state of the custom app
 */
void AppCustom::checkState()
{
    emit appConnected(true);
}


void AppCustom::sendMidi(QJsonObject commandKey)
{
    int line = commandKey.take("line").toInt();
    commandKey["pitch"] = _keyPitchMap[line];   // Convert key to pitch
    Keyboard::instance()->sendMidi(commandKey);
}

void AppCustom::sendKeystroke(QJsonObject commandKey)
{
    Keyboard::instance()->sendKeystroke(commandKey);
}

/*!
 * \brief AppCustom::setClef
 *
 * Set clef in custom app changing base reference of midi pitch
 */
void AppCustom::setClef(QJsonObject clefWrapper)
{
    qDebug() << "clef: " << clefWrapper.value("value");
    _lowerPitch = clefWrapper.value("value").toInt();
    updateKeyPitchMap();
}

/*!
 * \brief AppCustom::setKeysig
 *
 * Set key signature in custom app changing base reference of midi pitch
 * _currentScale contains pitches of the diatonic scale starting from C
 * base _currentScale is [0, 2, 4, 5, 7, 9, 11] corresponding to C major scale
 *
 * n. |  key signature   | accidentals        | scaleIndexes to be incremented
 * 7 is C# major, sharps: F# C# G# D# A# E# B#  (3, 0, 4, 1, 5, 2, 6)
 * 6 is F# major, sharps: F# C# G# D# A# E#     (3, 0, 4, 1, 5, 2)
 * 5 is B major, sharps:  F# C# G# D# A#        (3, 0, 4, 1, 5)
 * 4 is E major, sharps:  F# C# G# D#           (3, 0, 4, 1)
 * 3 is A major, sharps:  F# C# G#              (3, 0, 4)
 * 2 is D major, sharps:  F# C#                 (3, 0)
 * 1 is G major, sharps:  F#                    (3)
 * 0 is C major, no sharps or flats
 *
 * n. |  key signature   | accidentals        | scaleIndexes to be decremented
 * -7 is Cb major, flats: Bb Eb Ab Db Gb Cb Fb  (6, 2, 5, 1, 4, 0, 3)
 * -6 is Gb major, flats: Bb Eb Ab Db Gb Cb     (6, 2, 5, 1, 4, 0)
 * -5 is Db major, flats: Bb Eb Ab Db Gb        (6, 2, 5, 1, 4)
 * -4 is Ab major, flats: Bb Eb Ab Db           (6, 2, 5, 1)
 * -3 is Eb major, flats: Bb Eb Ab              (6, 2, 5)
 * -2 is Bb major, flats: Bb Eb                 (6, 2)
 * -1 is F major, flats:  Bb                    (6)
 *
 */
void AppCustom::setKeysig(QJsonObject keysigWrapper)
{
    qDebug() << "keysig: " << keysigWrapper.value("value");
    _currentKeysig = keysigWrapper.value("value").toInt();
    updateKeyPitchMap();
}

/*!
 * \brief AppCustom::updateKeyPitchMap
 *
 * Update _keyPitchMap with the new key signature and clef (_lowerPitch)
 */
void AppCustom::updateKeyPitchMap()
{
    // Initialize the scale based on the base scale (C major)
    for (int i = 0; i < 7; i++)
        if(_currentKeysig > 0 && accidental[i] < _currentKeysig)
            _currentScale[i] = (C_Scale[i] + 1) % 12;     // sharp in scale
        else if(_currentKeysig < 0 && accidental[i] >= 7 + _currentKeysig)
            _currentScale[i] = (C_Scale[i] + 11) % 12;    // flat in scale
        else
            _currentScale[i] = C_Scale[i];

    qDebug() << "currentScale" << _currentScale;

    // switch case matching 19 keys (0 to 18) according starting from _lowerPitch
    // adjusting by _currentScale
    _keyPitchMap.clear();

    int pitch = _currentKeysig < -4 ? _lowerPitch - 1 : _lowerPitch;
    while(_keyPitchMap.size() < 19)
        if (_currentScale.contains(pitch++ % 12))
            _keyPitchMap << pitch - 1;

    qDebug() << "keyPitchMap" << _keyPitchMap;
}
