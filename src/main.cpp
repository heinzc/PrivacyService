#include "rest_handler.h"
#include "seal_handler.h"
#include "he_controller.h"
#include "vicinity_handler.h"
#include "PrivacyPluginInterface.h"

#include <iostream>

#include <QtCore>
#include <QDebug>

#include "../third-party/qthttpserver/src/httpserver/qhttpserver.h"

using namespace std;

// controller
he_controller controller = he_controller();

void load_plugins() {
    // look for plugins in the "plugins" subdirectory of the application
    QDir pluginsDir = QDir(QCoreApplication::applicationDirPath());
    pluginsDir.cd("plugins");

    const auto entryList = pluginsDir.entryList(QDir::Files);
    for (const QString& fileName : entryList) {
        qDebug() << "loading plugins from: " << pluginsDir.absoluteFilePath(fileName) << "...";
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject* plugin = loader.instance();
        if (plugin) {
            auto privacyplugin = qobject_cast<PrivacyPluginInterface*>(plugin);
            if (privacyplugin) {
                controller.registerPrivacyPlugin(privacyplugin);
            }
        }
        else {
            qDebug() << "Failed to load plugin: " << loader.fileName() << ": " << loader.errorString();
        }
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // initialize compenents
    //database
    db_access * db = new db_access("service.db");
    controller.setDB_access(db);

    // he
    he_handler * he = (he_handler*) (new seal_he_handler());
    controller.setHE_handler(he);
    he->initialize(); //must be after setting he handler to be able to access database
                      
    // RESTful API
    rest_handler* restHandler = new rest_handler();
    controller.setREST_handler(restHandler);

    // vicinity
    vicinity_handler* vicinity = new vicinity_handler();
    controller.setVICINITY_handler(vicinity);
    vicinity->initialize("config_adapters.json");


    // load plugins. This has to be the last step, as Plugins may interact with REST, the DB or VICINITY
    load_plugins();

    
    //debugging... to be moved into unittesting
    
    //std::cout << "Trusted parties: " + vicinity->readProperty(std::string("he_service"), std::string("trustedparties")) << std::endl;
    
    //std::string value;
    //std::string pubKey;
    //std::vector<std::string> valuesvec;

    //value = he->encrypt_as_string(71);
    //pubKey = he->getPublicKey();

    //valuesvec.push_back(value);
    //
    //value = he->encrypt_as_string(71, pubKey);

    //valuesvec.push_back(value);

    //std::string result = he->aggregate(valuesvec, db->get_own_key("PK").c_str());
    //int intval = he->decrypt(result);

    //std::cout << "decrypted test result: " << intval << std::endl;

    //std::cout << he->getPublicKey() << std::endl;

    return app.exec();
}
