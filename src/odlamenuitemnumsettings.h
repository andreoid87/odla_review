#ifndef ODLAMENUITEMNUMSETTINGS_H
#define ODLAMENUITEMNUMSETTINGS_H

#include "odlamenuitem.h"

class OdlaMenuItemNumSettings : public OdlaMenuItem
{
    Q_OBJECT

public:
    OdlaMenuItemNumSettings(QWidget *parent, QMap<QString, QVariant> dbFields);
    float getValue() {return _numValue;}
    void changeValue(bool increment);
    void enterClick();
    menu_item_t type() {return NUM_SETTINGS;}
    bool isClicked() {return false;} //pointless for incNumber

private:
    float _numValue;
    float _step;
    float _min;
    float _max;

signals:
    void editedItem(OdlaMenuItem*);
};

#endif // ODLAMENUITEMNUMSETTINGS_H
