#ifndef APPMUSESCORE4H_H
#define APPMUSESCORE4H_H

#include "app.h"
#include <QFile>

/*!
 * \brief Musescore class
 *
 * It manage socket communication with musescore
 */
class AppMusescore4 : public App
{
    Q_OBJECT
    friend class App;

private:
    AppMusescore4(QObject *parent = nullptr);
    void installMusescorePlugin();
    bool checkPluginVersion(const QString &plugin);
    bool createTempPluginFolder(QString tempDirPath);

public slots:
    void send(QJsonObject commandKey) override;
    QString readCursor() override;
    QString readStatus() override;
    QString requireInput(QJsonObject command) override;

protected slots:
    // Pure virtual slots
    void onIncomingData(const QString &inputString) override;
    bool createFolder(const QString &tempDirPath, bool removeIfExists);
    void setFilePermissions(QFile &file);

    friend class App;
};

#endif // APPMUSESCORE4H_H
