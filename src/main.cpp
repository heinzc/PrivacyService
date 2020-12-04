#include "../include/stdafx.h"
#include "../include/rest_handler.h"
#include "../include/seal_handler.h"
#include "../include/he_controller.h"
#include "../include/vicinity_handler.h"

#include <iostream>
#include <QtCore>
#include "../third-party/qthttpserver/src/httpserver/qhttpserver.h"

using namespace std;


int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // initialize compenents
    // controller
    he_controller controller = he_controller();

    //database
    db_access * db = new db_access("service.db");
    controller.setDB_access(db);

    // vicinity
    //vicinity_handler * vicinity = new vicinity_handler();
    //controller.setVICINITY_handler(vicinity);
    //vicinity->initialize("config_adapters.json");

    // he
    he_handler * he = (he_handler*) (new seal_he_handler());
    controller.setHE_handler(he);
    he->initialize(); //must be after setting he handler to be able to access database
                      
    // RESTful API
    rest_handler* restHandler = new rest_handler();
    controller.setREST_handler(restHandler);
    restHandler->setup();

    //vicinity->generateThingDescription();

    
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
