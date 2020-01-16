#include "../include/stdafx.h"
#include "../include/rest_handler.h"
#include "../include/fhe_handler.h"
#include "../include/phe_handler.h"
#include "../include/he_controller.h"
#include "../include/db_access.h"

//#include <cassert>
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
    //TEST
    db_access * db = new db_access("test.db");
    //db->insert_public_key(1, "ke22eeye");
    //TEST
    
    
    
    
    he_controller controller = he_controller();
	he_handler * he = (he_handler*) (new phe_handler());
	he->initialize();
    controller.setHE_handler(he);

    //debugging
    std::string value;
    value = he->encrypt_as_string(42);
    std::vector<std::string> valuesvec;
    valuesvec.push_back(value);
    valuesvec.push_back(value);

    std::string result = he->aggregate(valuesvec);

    int intval = he->decrypt(result);

    std::cout << intval << std::endl;

	utility::string_t address = U("http://192.168.188.37:4242"); //127.0.0.1

    //on_initialize(address);
    uri_builder uri(address);
  
    auto addr = uri.to_uri().to_string();
    //g_httpHandler = std::unique_ptr<rest_handler>(new rest_handler(addr));
    g_httpHandler = new rest_handler(addr);
    controller.setREST_handler(g_httpHandler);
    g_httpHandler->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;
    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);

    on_shutdown();
    return 0;
}
