#pragma once

#include "../PrivacyPluginInterface.h"

#include <QObject>
#include <QtPlugin>
#include <QString>
#include <QJsonObject>


class mpc_plugin : public QObject, public PrivacyPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.github.tukcps.PrivacyService.PrivacyPluginInterface")
    Q_INTERFACES(PrivacyPluginInterface)

public:
    mpc_plugin();
    virtual ~mpc_plugin();

    QString pluginName() const {
        return "mpc";
    }

    void initialize();

private:
    QJsonObject handleHueHue();
};