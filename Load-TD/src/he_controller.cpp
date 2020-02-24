#include "../include/he_controller.h"
#include "../include/he_handler.h"
#include "../include/rest_handler.h"

he_controller::he_controller()
{
    //ctor
    m_pHE_handler = 0;
    m_pREST_handler = 0;
}

he_controller::~he_controller()
{
    //dtor
}

void he_controller::setHE_handler(he_handler * handler) {
    m_pHE_handler = handler;
    m_pHE_handler->setController(this);
}

he_handler * he_controller::getHE_handler() {
    return m_pHE_handler;
}

void he_controller::setREST_handler(rest_handler *  handler) {
    m_pREST_handler = handler;
    m_pREST_handler->setController(this);
}

rest_handler * he_controller::getREST_handler() {
    return m_pREST_handler;
}