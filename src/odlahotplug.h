#ifndef ODLAHOTPLUG_H
#define ODLAHOTPLUG_H

#include <QObject>

class ODLAHotplug : public QObject
{
    Q_OBJECT
public:
    explicit ODLAHotplug(QObject *parent = nullptr);
    ~ODLAHotplug();

signals:
    void deviceConnectionChanged(bool found, QString path);

public slots:

protected slots:
    void pollCallback();

protected:
};

#endif // ODLAHOTPLUG_H
