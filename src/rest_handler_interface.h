#pragma once

#include <iostream>
#include <functional>

#include "he_controller.h"

#include <QObject>

#include <QUrl>
#include <QJsonDocument>


using namespace std;


class rest_handler_interface : public QObject
{
    Q_OBJECT

    public:
        virtual void setController(he_controller * controller) = 0;

        virtual void get(const QUrl & endpoint, QObject* caller) = 0;

public slots:
        virtual QJsonDocument get_blocking(const QUrl& endpoint) = 0;

        virtual QJsonDocument put_blocking(const QUrl& endpoint, const QJsonDocument& payload) = 0;

        virtual QJsonDocument post_blocking(const QUrl& endpoint, const QJsonDocument& payload) = 0;
};
