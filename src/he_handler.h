#pragma once

#include <iostream>
#include <vector>
#include "he_controller.h"

#include "db_access.h"

#include <QtCore>

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

        virtual QString encrypt_as_QString(double x, std::string pubKey = std::string() ) = 0;

        //virtual int decrypt(std::string & ctxt) = 0;
        virtual double decrypt(QString& ctxt) = 0;

        QString sum(QStringList& input, std::string pubKey = std::string());

        virtual std::string getPublicKey() = 0;

    protected:
        he_controller * m_pController;
};
