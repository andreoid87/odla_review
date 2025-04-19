#ifndef I_SCORE_WRITER_APP_PLUGIN_H
#define I_SCORE_WRITER_APP_PLUGIN_H

#include "commons.h"
#include <QObject>

class IScoreWriterAppPlugin
{
public:
    IScoreWriterAppPlugin() {}
    virtual ~IScoreWriterAppPlugin() {}

    virtual void setTemplateFilePath(QString fileName)
    {
        _templateFilePath = fileName;
    }

    virtual void setFileExtension(QString fileExt)
    {
        _fileExtension = fileExt;
    }

    virtual void setDefaultClef(Clefs c)
    {
        _defaultClef = c;
    }

    virtual void setDefaultDuration(Durations d)
    {
        _defaultDuration = d;
    }

    virtual void setDefaultTimeBeats(int beats)
    {
        _timeBeats = beats;
    }

    virtual void setDefaultTimeUnit(int unit)
    {
        _timeUnit = unit;
    }

    virtual void setComposerName(QString name)
    {
        _composer = name;
    }

    virtual void init() = 0;
    virtual void newScore() = 0;
    virtual void openScore() = 0;
    virtual void closeScore() = 0;
    virtual void saveScore() = 0;
    virtual void saveScoreAs() = 0;
    virtual void quit() = 0;
    virtual void exportScore() = 0;
    virtual void printScore() = 0;
    virtual void showPageFormatOption() = 0;
    virtual void showInstrumentsSelection() = 0;
    virtual void toggleConcertPitch() = 0;
    virtual void insertMeasures(int measures) = 0;
    virtual void deleteSelection() = 0;
    virtual void insertFrames() = 0;
    virtual void transpose() = 0;
    virtual void explode() = 0;
    virtual void implode() = 0;
    virtual void extractParts() = 0;
    virtual void setViewMode(int opt) = 0;
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    virtual void setLoop() = 0;
    virtual void showMixer() = 0;
    virtual void showPlayPanel() = 0;
    virtual void advancedSelection() = 0;
    virtual void fillWithSlashes() = 0;
    virtual void toogleSlashesNotation() = 0;
    virtual void toogleImageCapture() = 0;
    virtual void applyDefaultScoreSettings() = 0;
    virtual void gotoBeginning() = 0;
    virtual void gotoEnd() = 0;
    virtual void gotoMeasure(int m) = 0;
    virtual void gotoStaff(int s) = 0;
    virtual void getStatus() = 0;
    virtual void updateScoreStatus() = 0;
    // Playback methods
    virtual void startPlayback() = 0;
    virtual void stopPlayback() = 0;
    virtual void pausePlayback() = 0;
    virtual void resumePlayback() = 0;
    virtual void toggleMetronome(bool enabled) = 0;
    virtual bool isPlaying() = 0;

    virtual void addNoteToStaffLine(int line, bool keepChord = false, bool slur = false) = 0;
    virtual void setNoteValue(Durations duration) = 0;
    virtual void setAugmentationDots(Durations resetDuration, Augmentations dots = Augmentations::NONE) = 0;
    virtual void addRest(Durations duration = Durations::NONE) = 0;
    virtual void addTie() = 0;
    virtual void addDynamics(Dynamics dyn) = 0;
    virtual void addTremolo(Tremolo t) = 0;
    virtual void addArticulation(Articulations f) = 0;
    virtual void addOrnament(Ornaments o) = 0;
    virtual void addGraceNote(GraceNotes gn) = 0;
    //virtual void addBreathsAndPauses(BreathsAndPauses bp) = 0;
    virtual void addNoteHead(NoteHeads nh) = 0;
    virtual void addFingering(Fingering f) = 0;
    virtual void addArpeggio(Arpeggio a) = 0;
    virtual void pitchUpDiatonic() = 0;
    virtual void pitchDownDiatonic() = 0;
    virtual void pitchUpOctave() = 0;
    virtual void pitchDownOctave() = 0;
    virtual void setAccidental(Accidentals accidental, bool withBrackets = false) = 0;
    virtual void insertClef(Clefs clef) = 0;
    virtual void insertTimeSignature(int numerator, int denominator) = 0;
    virtual void insertKeySignature(Accidentals accidental, int count) = 0;
    virtual void setVoice(int voice) = 0;
    virtual void insertTuplet(int numerator, Durations fullDuration, Augmentations dots) = 0;
    virtual void insertBarline(Bars bar) = 0;
    virtual void insertLine(Lines line) = 0;
    virtual void insertTempo(Tempo timing, QString text) = 0;
    virtual void insertInterval(Intervals interval) = 0;
    // methods to move cursor
    virtual void moveToUpperNote() = 0;
    virtual void moveToLowerNote() = 0;
    virtual void moveToNextChord() = 0;
    virtual void moveToPrevChord() = 0;
    virtual void moveToNextMeasure() = 0;
    virtual void moveToPrevMeasure() = 0;
    virtual void moveToNextTrack() = 0;
    virtual void moveToPrevTrack() = 0;
    // methods to exapand selection
    virtual void selectNextChord() = 0;
    virtual void selectPrevChord() = 0;
    virtual void selectNextMeasure() = 0;
    virtual void selectPrevMeasure() = 0;
    virtual void selectBeginLine() = 0;
    virtual void selectEndLine() = 0;
    virtual void selectBeginScore() = 0;
    virtual void selectEndScore() = 0;
    virtual void selectStaffAbove() = 0;
    virtual void selectStaffBelow() = 0;

    virtual void copySelection() = 0;
    virtual void pasteSelection() = 0;
    virtual void repeatSelection() = 0;
    virtual void emptySelection() = 0;
    virtual void selectRange(int from, int to) = 0;
    virtual void selectAllMeasures() = 0;
    virtual void notifyAccessibleMessage(QString, bool) = 0;
    virtual void addAboveStaffText() = 0;
    virtual void addBelowStaffText() = 0;
    virtual void addLyrics() = 0;
    virtual void addChordText() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void sendEscape() = 0;



protected:
    QString _templateFilePath;
    QString _fileExtension;
    Clefs _defaultClef;
    Durations _defaultDuration;
    int _timeBeats;
    int _timeUnit;
    QString _composer;

};

#endif // I_SCORE_WRITER_APP_PLUGIN_H
