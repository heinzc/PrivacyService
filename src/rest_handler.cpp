#include "../include/rest_handler.h"

//#include "FHE.h"
#include "../include/he_handler.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <exception>


rest_handler::rest_handler()
{
    //ctor
}
rest_handler::rest_handler(utility::string_t url):m_listener(url)
{
    m_listener.support(methods::GET, std::bind(&rest_handler::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&rest_handler::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&rest_handler::handle_post, this, std::placeholders::_1));
    m_listener.support(methods::DEL, std::bind(&rest_handler::handle_delete, this, std::placeholders::_1));

    input_counter = 0;
}
rest_handler::~rest_handler()
{
    //dtor
}

void rest_handler::setController(he_controller * controller) {
    m_pController = controller;
}

void rest_handler::handle_error(pplx::task<void>& t)
{
    try
    {
        t.get();
    }
    catch(...)
    {
        // Ignore the error, Log it if a logger is available
    }
}


//
// Get Request 
//
void rest_handler::handle_get(http_request message) {

    try {
        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));

        if(std::find(paths.begin(), paths.end(), "aggregation") != paths.end()) {
            if(input_counter < 3) {
                message.reply(status_codes::Forbidden, "not enough inputs provided");
                return;
            } else {
                m_pController->getHE_handler()->aggregate(input_counter);
                message.reply(status_codes::OK, "OK");
                return;
            }
        } else if(std::find(paths.begin(), paths.end(), "sum") != paths.end()) {
            int sum = m_pController->getHE_handler()->getSum();
            message.reply(status_codes::OK, to_string(sum));
            return;
        }else if(std::find(paths.begin(), paths.end(), "publickey") != paths.end()) {
            message.reply(status_codes::OK, m_pController->getHE_handler()->getPublicKey());
            return;
        } else if(std::find(paths.begin(), paths.end(), "TESTsetkey") != paths.end()) { //only for TESTING, to be REMOVED
            m_pController->getHE_handler()->setPublicKey("{\"n\":\"1yfw1AlQtvC2C88VariPBtPTc52rY5Ivw6EcTqdUkoxBJk9xo6OAgh8o8amcj58hvKtHfFE2TqeVzk0muT0H8HEbJG6vByglxGyXY5H0orgKPXGdOnUi6CI8DRmsB1M741DKjIMLfejSrLXQ5gcfq98f4AYuIbBjvsmvZtXG1XQZub7hizOUNJ44Z10Rci8SVKRAGm08PCljqPak767ZpQ0SSEdXIn39FndUOWG1OaM92B5fiIoSICF1GS7GNRCtyqpaix4ESqhcsXgWAZ5zB3mW5CMT8sZKQBXu4N8CWR78tbt0CsaE66IGIFheJH0lxPJx7pUSYGgIB7uikJJI1bAq9\"}");
            message.reply(status_codes::OK, string("DONE"));
            return;
        }

        message.reply(status_codes::NotFound,"WAT?!");
    }
    catch(exception& e) {
        message.reply(status_codes::BadRequest, e.what());
    }
    return ;

/*     concurrency::streams::fstream::open_istream(U("static/index.html"), std::ios::in).then([=](concurrency::streams::istream is)
    {
        message.reply(status_codes::OK, is,  U("text/html"))
		.then([](pplx::task<void> t)
		{
			try{
				t.get();
			}
			catch(...){
				//
			}
	});
    }).then([=](pplx::task<void>t)
	{
		try{
			t.get();
		}
		catch(...){
			message.reply(status_codes::InternalError,U("INTERNAL ERROR "));
		}
	}); */
}

//
// A POST request
//
void rest_handler::handle_post(http_request message) {
    try{
        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));

        if(std::find(paths.begin(), paths.end(), "produce") != paths.end()) {
            string stvalue = message.extract_string().get();
            produce_ctxt(stvalue);
            message.reply(status_codes::OK,message.to_string());
            input_counter++;
        }
        else if(std::find(paths.begin(), paths.end(), "encrypt") != paths.end()) {
            string stvalue = message.extract_string().get();
            
            message.reply(status_codes::OK,encrypt_ptxt(stvalue));
        }
        else if(std::find(paths.begin(), paths.end(), "add") != paths.end()) {
            string stvalue = message.extract_string().get();

            m_pController->getHE_handler()->add(stvalue);
            message.reply(status_codes::OK,"läuft!");
        }
        else if(std::find(paths.begin(), paths.end(), "aggregate") != paths.end()) {
            json::object request_json = message.extract_json().get().as_object();
            json::array values = request_json.at("values").as_array();
            std::vector<std::string> vec;
            for(auto it = values.begin(); it != values.end(); ++it) {
                std::cout << it->as_string() << std::endl;
                vec.push_back(it->as_string());
            }
            //string stvalue = message.extract_string().get();

            std::string result = m_pController->getHE_handler()->aggregate(vec);
            message.reply(status_codes::OK, m_pController->getHE_handler()->decrypt(result));
        }
        else if(std::find(paths.begin(), paths.end(), "decrypt") != paths.end()) {
            string stvalue = message.extract_string().get();

            message.reply(status_codes::OK, m_pController->getHE_handler()->decrypt(stvalue));
        }

        message.reply(status_codes::NotFound,"WAT?!");
    }
    catch(exception& e) {
        message.reply(status_codes::BadRequest, e.what());
    }
    return ;
}

//
// A DELETE request
//
void rest_handler::handle_delete(http_request message)
{
     ucout <<  message.to_string() << endl;

        string rep = U("WRITE YOUR OWN DELETE OPERATION");
      message.reply(status_codes::OK,rep);
    return;
}


//
// A PUT request 
//
void rest_handler::handle_put(http_request message)
{
    ucout <<  message.to_string() << endl;
     string rep = U("WRITE YOUR OWN PUT OPERATION");
     message.reply(status_codes::OK,rep);
    return;
}


void rest_handler::produce_ctxt(string pt) {
    int value = stoi(pt);

    //m_pController->getHE_handler()->encrypt_and_store(value, input_counter);
}

string rest_handler::encrypt_ptxt(string pt) {
    int value = stoi(pt);

    return m_pController->getHE_handler()->encrypt_as_string(value);
}
