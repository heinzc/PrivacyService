#pragma once

#include <iostream>

using namespace std;

class he_handler;
class rest_handler;
class vicinity_handler;

class he_controller
{
    public:
        he_controller();
        virtual ~he_controller();

        void setHE_handler(he_handler * handler);
        he_handler * getHE_handler();

        void setREST_handler(rest_handler *  handler);
        rest_handler * getREST_handler();

        void setVICINITY_handler(vicinity_handler *  handler);
        vicinity_handler * getVICINITY_handler();

    protected:

    private:
        he_handler * m_pHE_handler;
        rest_handler * m_pREST_handler;
        vicinity_handler * m_pVICINITY_handler;

};