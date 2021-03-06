#include "he_controller.h"
#include "he_handler.h"
#include "rest_handler.h"
#include "db_access.h"
#include "vicinity_handler.h"
#include "PrivacyPluginInterface.h"

he_controller::he_controller()
{
    //ctor
    m_pHE_handler = 0;
    m_pREST_handler = 0;
    m_pDB_access = 0;
    m_pVICINITY_handler = 0;

    m_Plugins = QMap<QString, PrivacyPluginInterface*>();
}

he_controller::~he_controller()
{
    //dtor
    delete m_pHE_handler;
    delete m_pREST_handler;
    delete m_pDB_access;
    delete m_pVICINITY_handler;
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

void he_controller::setDB_access(db_access *  dba) {
    m_pDB_access = dba;
}

db_access * he_controller::getDB_access() {
    return m_pDB_access;
}

void he_controller::setVICINITY_handler(vicinity_handler *  handler) {
    m_pVICINITY_handler = handler;
    m_pVICINITY_handler->setController(this);
}

vicinity_handler * he_controller::getVICINITY_handler() {
    return m_pVICINITY_handler;
}

void he_controller::registerPrivacyPlugin(PrivacyPluginInterface* plugin) {
    m_Plugins.insert(plugin->pluginName(), plugin);
    plugin->setController(this);
    plugin->initialize();
    qDebug() << "Plugin " << plugin->pluginName() << " successfully added.";
}

PrivacyPluginInterface* he_controller::getPrivacyPluginHandle(const QString& pluginName) {
    return m_Plugins.value(pluginName);
}