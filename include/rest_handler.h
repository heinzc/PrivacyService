#pragma once

#include <iostream>
#include "stdafx.h"
#include "he_controller.h"
#include "db_access.h"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class rest_handler
{
    public:
        rest_handler();
        rest_handler(utility::string_t url);
        virtual ~rest_handler();

        void setController(he_controller * controller);

        pplx::task<void>open(){return m_listener.open();}
        pplx::task<void>close(){return m_listener.close();}
        
        //get pulic key, gets the key if it is not in database
        std::string get_public_key(const char* id, http_request message);

    protected:

    private:
        void handle_get(http_request message);
        void handle_put(http_request message);
        void handle_post(http_request message);
        void handle_delete(http_request message);
        void handle_error(pplx::task<void>& t);

        void produce_ctxt(string pt);
        string encrypt_ptxt(string pt);

        http_listener m_listener;

        he_controller * m_pController;

        int input_counter;
        
        std::string id;
        
        db_access * db;
};
