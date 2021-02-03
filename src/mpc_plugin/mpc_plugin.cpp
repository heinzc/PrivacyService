#include "mpc_plugin.h"

#include <QDebug>


mpc_plugin::mpc_plugin():
	QObject(), PrivacyPluginInterface() 
{

}
mpc_plugin::~mpc_plugin() 
{

}

void mpc_plugin::initialize() {
	addRoute("/mpc/huehuehue", QHttpServerRequest::Method::GET, [=](const QHttpServerRequest& request) {
		return handleHueHue();
		});
}

QJsonObject mpc_plugin::handleHueHue() {
	QJsonObject value;
	value.insert("value", "huehue");
	return value;
}