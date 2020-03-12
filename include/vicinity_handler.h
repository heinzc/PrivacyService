#pragma once

#include <iostream>

#include "stdafx.h"

#include "he_controller.h"

using namespace std;

class vicinity_handler
{
    public:
        vicinity_handler();
        virtual ~vicinity_handler();

        void setController(he_controller * controller) {
            m_pController = controller;
        }

        std::string generateThingDescription();

        std::string readProperty(std::string endpoint);


    protected:

    private:
        he_controller * m_pController;

};