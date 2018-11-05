#include "../include/stdafx.h"
#include "../include/rest_handler.h"
#include "../include/he_handler.h"
#include "../include/he_controller.h"

#include <cassert>
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
	he_handler * he = new he_handler();
	he->initialize();
    controller.setHE_handler(he);

/*     Ctxt ctx1 = he->encrypt(24);
    std::fstream ciphertext1("ciphertext1.txt", fstream::out|fstream::trunc);
    assert(ciphertext1.is_open());
    ciphertext1 << ctx1;
    ciphertext1.close();

	Ctxt ctx2 = he->encrypt(15);
    std::fstream ciphertext2("ciphertext2.txt", fstream::out|fstream::trunc);
    assert(ciphertext2.is_open());
    ciphertext2 << ctx2;
    ciphertext2.close();
	

	//Ctxt ctSum = ctx1;                   // Create a ciphertext to hold the sum and initialize it with Enc(2)
	//ctSum += ctx2;                       // Perform Enc(2) + Enc(3)

    Ctxt * ctSum = he->sum_up();

    ZZX ptSum = he->decrypt(ctSum);

	std::cout << "24 + 15 = " << ptSum[0] << std::endl; */

	utility::string_t address = U("http://127.0.0.1:4242");

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
