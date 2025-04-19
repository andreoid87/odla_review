#include "odlamenuitemnumsettings.h"
#include <odlasettings.h>
#include <QDebug>

OdlaMenuItemNumSettings::OdlaMenuItemNumSettings(QWidget* parent, QMap<QString, QVariant> dbFields) : OdlaMenuItem(parent, dbFields)
{
    _step = 0.1;//temporary
    _min = -1.0;//temporary
    _max = 1.0; //temporary
    _numValue = dbFields["numvalue"].toDouble();
    _nameLabel.setText(_title + QString("\n-  %1  +").arg(_numValue));
    QToolButton::update();
}

void OdlaMenuItemNumSettings::changeValue(bool increment)
{
    if((_numValue >= _max && increment) || (_numValue <= _min && !increment))
        return;
    _numValue += increment ? _step : -_step;
    _numValue = ((int)(_numValue * 10)) / 10.0;
    ODLASettings::saveNumSetting(_ID, _numValue);
    _nameLabel.setText(_title + QString("\n-  %1  +").arg(_numValue));
    emit editedItem(this);
    QToolButton::update();
}

void OdlaMenuItemNumSettings::enterClick()
{
    emit clickedItem(this);
}

