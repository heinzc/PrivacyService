#pragma once

#include "he_controller.h"
#include "rest_handler.h"


#define PrivacyPluginInterface_iid "com.github.tukcps.PrivacyService.PrivacyPluginInterface/1.0"

class PrivacyPluginInterface
{
public:
    virtual ~PrivacyPluginInterface() {}

    virtual QString pluginName() const = 0;

    virtual void initialize() = 0;

    void setController(he_controller* controller) {
        this->m_pController = controller;
    }

protected:
    he_controller* m_pController;

    template<typename ... Args>
    bool addRoute(QString path, QHttpServerRequest::Method method, Args && ... args) {
        return m_pController->getREST_handler()->addRoute(path, method, std::forward<Args>(args) ...);
    };
};

Q_DECLARE_INTERFACE(PrivacyPluginInterface, PrivacyPluginInterface_iid)