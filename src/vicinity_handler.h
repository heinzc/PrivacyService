#pragma once

#include "he_controller.h"

#include <iostream>
#include <map>
#include <set>

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QMap>
#include <QNetworkReply>


using namespace std;

class vicinity_handler : public QObject
{
    Q_OBJECT

    public:
        vicinity_handler();
        virtual ~vicinity_handler();

        void setController(he_controller * controller) {
            m_pController = controller;
        }

        void initialize(QString configfile);

        void generateThingDescription();
        QJsonObject getThingDescription();

        QJsonObject readProperty(const QString& oid, const QString& pid);
        //needed for distributed aggregation -> get plain property of encrypted device
        //long readPlainProperty(std::string oid, std::string pid);
        QJsonObject writeProperty(const QString& oid, const QString& pid, const QJsonDocument& payload);
        //when an action request is received, this redirects it to the correct function
        //void postAction(std::string oid, std::string aid, std::string payload, std::string sourceOid);

        QString getOwnOid();
        QString getAdapterId();
        uint8_t getAgentPort();
        uint8_t getOwnPort();

        //void decrypt(std::string oid, std::string sourceOid, std::string payload);
        //void startAggregation(std::string oid, std::string sourceOid, std::string payload);
        //void sendShareAction(std::string oid, std::string sourceOid, std::string payload);
        //void participateInAggregation(std::string oid, std::string sourceOid, std::string payload);

        //returns task status; payload  must contain properties, participants, devices
        //std::string askToParticipateInAggregation(std::string destinationOid, std::string payload);

        //returns status
        //std::string getStatusOfAction(std::string destinationOid, std::string action, std::string taskId);
        //returns return value
        //std::string getReturnValueOfAction(std::string destinationOid, std::string action, std::string taskId);

        std::string getPublicKey(std::string oid);

        //std::string sendRandomShare(std::string destinationOid, std::string initiatorOid, int randomShare);

        std::string replaceAll(std::string str, const std::string from, const std::string to);

        //long getPropertyOfEncryptedDevice(std::string encryptedDeviceOid, std::string pid);

        //deltes a task, returns true if successful, else false
        //bool deleteTask(std::string destinationOid, std::string action, std::string taskId);

    public slots:
        void getAdapterTDFinished(QNetworkReply* reply);

    protected:

    private:
        he_controller * m_pController;
        QJsonObject m_configFile;
        QMap<QString, QString> m_endpoints;

        //bool updateTaskStatus(std::string aid, std::string status, std::string payload);

        uint8_t m_agentPort;
        QJsonObject m_ownTD;
        QString m_ownServiceOid;
        QString m_ownAdapterId;
};
