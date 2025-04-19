#ifndef APPCUSTOM_H
#define APPCUSTOM_H

#include "app.h"
#include <QFile>

class AppCustom : public App
{
public:
    explicit AppCustom(QObject *parent = nullptr, QString appID = "Custom");    
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
};

#endif // APPCUSTOM_H
