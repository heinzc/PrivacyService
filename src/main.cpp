#include "../include/stdafx.h"
#include "../include/rest_handler.h"
#include "../include/fhe_handler.h"
#include "../include/phe_handler.h"
#include "../include/seal_he_handler.h"
#include "../include/he_controller.h"
//#include "../include/db_access.h"
#include "../include/vicinity_handler.h"

#include <iostream>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

//std::unique_ptr<rest_handler> g_httpHandler;
rest_handler * g_httpHandler;

void on_initialize(const string_t& address)
{
    uri_builder uri(address);
  
    auto addr = uri.to_uri().to_string();
    //g_httpHandler = std::unique_ptr<rest_handler>(new rest_handler(addr));
    //g_httpHandler->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_shutdown()
{
    g_httpHandler->close().wait();
    return;
}


int main()
{	    
    he_controller controller = he_controller();
    db_access * db = new db_access("service.db");
    controller.setDB_access(db);
    vicinity_handler * vicinity = new vicinity_handler();
    std::cout << "INITNINITTJTT" << std::endl;
    vicinity->initialize("config_adapters.json");
    std::cout << "INITNINITTJTT" << std::endl;
    controller.setVICINITY_handler(vicinity);
	//he_handler * he = (he_handler*) (new phe_handler());
	he_handler * he = (he_handler*) (new seal_he_handler());
    controller.setHE_handler(he);
    he->initialize(); //must be after setting he handler to be able to access database
    //debugging
    if(db->isOwnDevice("oid1")) {
        std::cout << "1" << std::endl;
    }
    if(db->isOwnDevice("oid2")) {
        std::cout << "2" << std::endl;
    }
    if(db->hasAccessToDecrypt("oid1")) {
        std::cout << "3" << std::endl;
    }
    if(db->hasAccessToDecrypt("oid2")) {
        std::cout << "4" << std::endl;
    }
    if(db->hasAccessToDecrypt("oid3")) {
        std::cout << "5" << std::endl;
    }
    if(db->hasAccessToData("oid1", "oid1")) {
        std::cout << "6" << std::endl;
    }
    if(db->hasAccessToData("oid2", "oid1")) {
        std::cout << "7" << std::endl;
    }
    if(db->hasAccessToData("oid3", "oid1")) {
        std::cout << "8" << std::endl;
    }
    if(db->hasAccessToData("oid1", "oid2")) {
        std::cout << "9" << std::endl;
    }
    if(db->hasAccessToData("oid2", "oid2")) {
        std::cout << "10" << std::endl;
    }
    if(db->hasAccessToData("oid3", "oid2")) {
        std::cout << "11" << std::endl;
    }
    std::cout << "GG: " + db->getPrivacyService("oid11") << std::endl;
    std::cout << "GG: " + db->getPrivacyService("oid121") << std::endl;
    
    vicinity->readProperty(std::string("he_service"), std::string("trustedparties"));
    
    std::string value;
    std::string pubKey;
    std::vector<std::string> valuesvec;

    value = he->encrypt_as_string(71);
    pubKey = he->getPublicKey();
    
    std::cout << "summand 1: " << he->decrypt(value) << std::endl; //TESTING

    valuesvec.push_back(value);
    
    value = he->encrypt_as_string(71, pubKey);

    std::cout << "summand 2: " << he->decrypt(value) << std::endl; //TESTING
    valuesvec.push_back(value);

    std::string result = he->aggregate(valuesvec, db->get_own_key("PK").c_str());
    std::cout << "after aggregating" << std::endl; //TESTING
    int intval = he->decrypt(result);

    std::cout << "decrypted test result: " << intval << std::endl;

    //std::cout << he->getPublicKey() << std::endl;

	utility::string_t address = U("http://127.0.0.1:" + vicinity->getOwnPort()); //127.0.0.1    192.168.188.37

    //on_initialize(address);
    uri_builder uri(address);
  
    auto addr = uri.to_uri().to_string();
    //g_httpHandler = std::unique_ptr<rest_handler>(new rest_handler(addr));
    g_httpHandler = new rest_handler(addr);
    controller.setREST_handler(g_httpHandler);
    g_httpHandler->open().wait();

    vicinity->generateThingDescription();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;
    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);

    on_shutdown();
    return 0;
}
