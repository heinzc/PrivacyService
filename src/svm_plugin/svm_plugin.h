#pragma once

#include "../PrivacyPluginInterface.h"
#include "SVM_Handler.h"


#include <QObject>
#include <QtPlugin>
#include <QString>
#include <QJsonObject>

class svm_plugin : public QObject, public PrivacyPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.github.tukcps.PrivacyService.PrivacyPluginInterface")
    Q_INTERFACES(PrivacyPluginInterface)
    

    public:
        svm_plugin();
        
        virtual ~svm_plugin();
        
        QString pluginName() const {
            return "svm";
        }

        void initialize();

    private:
        SVM_handler handler;
        std::thread handler_thread_;
        QString test();

        /* SVM Task handle functions */
        void handle_incoming_svm_task(const QHttpServerRequest& request);
        void handle_sub_key(QJsonObject& body, shared_ptr<SVM_Task> task);
        void handle_incoming_recrypt_task(const QHttpServerRequest& request);
};

