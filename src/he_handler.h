#pragma once

#include "he_controller.h"

#include <iostream>

#include <QString>
#include <QJsonObject>

class he_handler
{
    public:
        he_handler() {
            m_pController = 0;
        };

        virtual ~he_handler() {

        };

        void setController(he_controller * controller) {
            m_pController = controller;
        }


        virtual void initialize() = 0;

        virtual QString encrypt_as_QString(double x, const QString& pubKey = QString() ) = 0;

        //virtual int decrypt(std::string & ctxt) = 0;
        virtual double decrypt(const QString& ctxt) = 0;

        virtual QString recrypt(const QString& ctxt) = 0;

        virtual void recrypt_for_svm(const QString& ctxt, int dimension, QJsonObject& retVal) = 0;

        virtual QString sum(QStringList& input, const QString& pubKey = QString()) = 0;

        virtual QString getPublicKey() = 0;

        virtual QString getEncryptionParameters() = 0;

        virtual QString getRelinKeys() = 0;
        virtual void getRelinKeys(QString& relinKeys) = 0;

        virtual QString getGaloisKeys() = 0;
        virtual void getGaloisKeys(QString& galoisKeys) = 0;

    protected:
        he_controller * m_pController;
};
