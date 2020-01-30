#pragma once

#include <iostream>
#include <vector>
#include "he_controller.h"

#include "db_access.h"

class he_handler
{
    public:
        he_handler(db_access * database) {
            m_pController = 0;
            db = database;
        };

        virtual ~he_handler() {

        };

        void setController(he_controller * controller) {
            m_pController = controller;
        }

        virtual void initialize() = 0;

        virtual std::string encrypt_as_string(long x) = 0;

        virtual int decrypt() = 0;
        virtual int decrypt(std::string & ctxt) = 0;

        virtual void aggregate(int count) = 0;
        virtual std::string aggregate(std::vector<std::string> & input, const char* publickey) = 0;
        virtual void add(std::string & ctxt, const char* publickey) = 0;

        virtual int getSum() = 0;
        
        virtual std::string getPublicKey() = 0;
        
        db_access * db;

    protected:

    private:
        he_controller * m_pController;
        
        virtual void setPublicKey(const char* json) = 0;
        virtual void setPrivateKey(const char* json) = 0;
};
