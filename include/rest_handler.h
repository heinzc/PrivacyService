#pragma once

#include <iostream>
#include "stdafx.h"
#include "he_controller.h"

#include "db_access.h"

class QHttpServer;

using namespace std;
//using namespace web;
//using namespace http;
//using namespace utility;
//using namespace http::experimental::listener;

class rest_handler
{
    public:
        rest_handler();
        //rest_handler(utility::string_t url);
        virtual ~rest_handler();

        void setController(he_controller * controller);

        int setup();

        //pplx::task<void>open(){return m_listener.open();}
        //pplx::task<void>close(){return m_listener.close();}

    protected:

    private:
        //void handle_get(http_request message);
        //void handle_put(http_request message);
        //void handle_post(http_request message);
        //void handle_delete(http_request message);
        //void handle_error(pplx::task<void>& t);

        //void produce_ctxt(string pt);
        //string encrypt_ptxt(string pt);
        //
        //void handle_VICINITY_GET_request(http_request message, std::vector<utility::string_t> path);
        //void handle_VICINITY_POST_request(http_request message, std::vector<utility::string_t> path);
        //void handle_VICINITY_PUT_request(http_request message, std::vector<utility::string_t> path);

        //http_listener m_listener;
        QHttpServer* m_pHttpServer;

        he_controller * m_pController;

        int input_counter;
};
