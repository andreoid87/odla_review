#ifndef MUSESCORE_PLUGIN_H
#define MUSESCORE_PLUGIN_H

#include <QWebSocket>
#include <QJsonObject>
#include <QTimer>
#include <QProcess>
#include <QEventLoop>
#include "app.h"

#define SIGLE_ELEMENT_SIZE sizeof(state_message_t::element_fields_t)
#define RANGE_ELEMENT_SIZE sizeof(state_message_t::range_fields_t)
#define NO_ELEMENT_SIZE sizeof(state_message_t::common_fields_t)

/*!
 * \brief Musescore class
 *
 * It manage socket communication with musescore
 */
class AppMusescore3 : public App
{
    Q_OBJECT
public:
    static App *instance(QObject *parent = nullptr);

private:
    AppMusescore3(QObject *parent);

public slots:
    void send(QJsonObject commandKey) override; // load command and vocal message from DB
    QString readCursor()    override;
    QString readStatus()    override;

protected slots:
    void onIncomingData(const QString &speechMessage) override;
};

#endif // MUSESCORE_PLUGIN_H
