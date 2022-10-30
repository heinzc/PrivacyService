#pragma once

#include <iostream>
#include <functional>

#include "he_controller.h"
#include "db_access.h"

#include <QObject>

#include <QHttpServer>

#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>



class QNetworkAccessManager;


using namespace std;



class rest_handler : public QObject
{
    Q_OBJECT

    public:
        rest_handler();
        virtual ~rest_handler();

        void setController(he_controller * controller);

        void get(const QUrl & endpoint, QObject* caller);


public slots:
        QJsonDocument get_blocking(const QUrl& endpoint, const QList<QPair<QByteArray, QByteArray>>& headers = QList<QPair<QByteArray, QByteArray>>());

        QJsonDocument put_blocking(const QUrl& endpoint, const QJsonDocument& payload);

        QJsonDocument post_blocking(const QUrl& endpoint, const QJsonDocument& payload);

    protected:
        template<typename ... Args>
        bool addRoute(QString path, QHttpServerRequest::Method method, Args && ... args) {
            return m_pHttpServer->route(path, method, std::forward<Args>(args) ...);
        }


    private:
        int setupRoutes();
        //void handle_post(http_request message);

        //void produce_ctxt(string pt);
        //string encrypt_ptxt(string pt);
        QJsonDocument extractJsonFromRequest(const QHttpServerRequest& request);

        double extract_double_from_request(const QHttpServerRequest& request);
        QString extract_ctxt_from_request(const QHttpServerRequest& request);

        QJsonObject handle_VICINITY_GET_objects(const QHttpServerRequest& request, QHttpServerResponder &responder);
        QJsonObject handle_VICINITY_GET_properties(QString oid, QString pid, const QHttpServerRequest& request, QHttpServerResponder &responder);
        QJsonObject handle_VICINITY_PUT_properties(QString oid, QString pid, const QHttpServerRequest& request, QHttpServerResponder &responder);
        QJsonObject handle_VICINITY_POST_action(QString oid, QString aid, const QHttpServerRequest& request, QHttpServerResponder &responder);

        QJsonObject handle_local_encrypt(const QHttpServerRequest& request, QHttpServerResponder &responder);
        QJsonObject handle_local_aggregate(const QHttpServerRequest& request, QHttpServerResponder &responder);
        QJsonObject handle_local_decrypt(const QHttpServerRequest& request, QHttpServerResponder &responder);

        //http_listener m_listener;
        QHttpServer* m_pHttpServer;

        QNetworkAccessManager* m_pNetManager;

        he_controller * m_pController;

    friend class PrivacyPluginInterface;
};
