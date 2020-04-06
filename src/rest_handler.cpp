#include "../include/rest_handler.h"

//#include "FHE.h"
#include "../include/he_handler.h"
#include "../include/vicinity_handler.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <exception>
#include <regex>

#include <cpprest/http_client.h>

using namespace web::http;
using namespace web::http::client; 

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
    
    identifier = "123TESTID123";
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

        if(std::find(paths.begin(), paths.end(), "aggregation") != paths.end()) { //DEPRECATED
            if(input_counter < 3) {
                message.reply(status_codes::Forbidden, "not enough inputs provided");
                return;
            } else {
                m_pController->getHE_handler()->aggregate(input_counter);
                message.reply(status_codes::OK, "OK");
                return;
            }
        } else if(std::find(paths.begin(), paths.end(), "sum") != paths.end()) { //DEPRECATED
            int sum = m_pController->getHE_handler()->getSum();
            message.reply(status_codes::OK, to_string(sum));
        } else if(std::find(paths.begin(), paths.end(), "vicinity") != paths.end()) {
            handle_VICINITY_GET_request(message, paths);
            return;
        }else if(std::find(paths.begin(), paths.end(), "publickey") != paths.end()) {
            http_response response(status_codes::OK);
            response.headers().add(U("ID"), U(identifier.c_str())); //add own id to the headers
            response.set_body(m_pController->getHE_handler()->getPublicKey());            
            
            message.reply(response).then([](pplx::task<void> t) {});
            
            //message.reply(status_codes::OK, m_pController->getHE_handler()->getPublicKey());
            return;
        } else if(std::find(paths.begin(), paths.end(), "TESTsetkey") != paths.end()) { //only for TESTING, to be REMOVED
            //m_pController->getHE_handler()->setPublicKey("{\"n\":\"1yfw1AlQtvC2C88VariPBtPTc52rY5Ivw6EcTqdUkoxBJk9xo6OAgh8o8amcj58hvKtHfFE2TqeVzk0muT0H8HEbJG6vByglxGyXY5H0orgKPXGdOnUi6CI8DRmsB1M741DKjIMLfejSrLXQ5gcfq98f4AYuIbBjvsmvZtXG1XQZub7hizOUNJ44Z10Rci8SVKRAGm08PCljqPak767ZpQ0SSEdXIn39FndUOWG1OaM92B5fiIoSICF1GS7GNRCtyqpaix4ESqhcsXgWAZ5zB3mW5CMT8sZKQBXu4N8CWR78tbt0CsaE66IGIFheJH0lxPJx7pUSYGgIB7uikJJI1bAq9\"}");
            message.reply(status_codes::OK, string("DONE"));
            return;
        } else if(std::find(paths.begin(), paths.end(), "TESTgetforeignkey") != paths.end()) { //only for TESTING, to be REMOVED
            std::string x = get_public_key("44", message);
            std::cout << "KEY IS: " << "\n";
            std::cout << x.c_str() << "\n";
            message.reply(status_codes::OK, string("DONE"));
            return;
        } else if(std::find(paths.begin(), paths.end(), "TESThasaccess") != paths.end()) { //only for TESTING, to be REMOVED
            std::cout << "testhasaccess" << "\n";
            message.reply(status_codes::OK, "nicht fertig");//m_pController->getDB_access()->hasAccess("spc_serviceAAA"));
            return;
        }
/*      else if(std::find(paths.begin(), paths.end(), "hasaccess") != paths.end()) {
            if(message.headers().has(U("requester-id"))) { //is there the requester-id header?
                std::string header_id = ::utility::conversions::to_utf8string(message.headers().operator[](U("requester-id"))); //id

            }
            else {
                message.reply(status_codes::OK, string("FALSE"));
            }
            return;
        }*/

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
    std::cout << "POST!" << std::endl;
    try{
        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));

        if(std::find(paths.begin(), paths.end(), "produce") != paths.end()) { //DEPRECATED
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
            if(message.headers().has(U("ID"))) {
                std::string header_id = ::utility::conversions::to_utf8string(message.headers().operator[](U("ID")));
                m_pController->getHE_handler()->add(stvalue, get_public_key(header_id.c_str(), message).c_str());
                message.reply(status_codes::OK,"läuft!");
                return;
            }
            message.reply(status_codes::NotFound, "error, header \"id\" missing");
        }
        else if(std::find(paths.begin(), paths.end(), "aggregate") != paths.end()) {
            if(message.headers().has(U("ID"))) {
                std::string header_id = ::utility::conversions::to_utf8string(message.headers().operator[](U("ID")));
                json::object request_json = message.extract_json().get().as_object();
                json::array values = request_json.at("values").as_array();
                std::vector<std::string> vec;
                for(auto it = values.begin(); it != values.end(); ++it) {
                    std::cout << it->as_string() << std::endl;
                    vec.push_back(it->as_string());
                }
                //string stvalue = message.extract_string().get();
                
                std::string result = m_pController->getHE_handler()->aggregate(vec, get_public_key(header_id.c_str(), message).c_str());
                message.reply(status_codes::OK, result); //m_pController->getHE_handler()->decrypt(result));
            }
            message.reply(status_codes::NotFound, "error, header \"id\" missing");
        }
        else if(std::find(paths.begin(), paths.end(), "decrypt") != paths.end()) {
            std::cout << "DECRYYPPT!!!!" << std::endl;
            string stvalue = message.extract_string().get();

            message.reply(status_codes::OK, m_pController->getHE_handler()->decrypt(stvalue));
        }
        else if(std::find(paths.begin(), paths.end(), "sendpublickey") != paths.end()) {
            string stvalue = message.extract_string().get(); //key
            //TODO check if key is correct?
            if(message.headers().has(U("ID"))) { //is there the id header?
                std::string header_id = ::utility::conversions::to_utf8string(message.headers().operator[](U("ID"))); //id
                m_pController->getDB_access()->insert_public_key(header_id.c_str(), stvalue.c_str());
                message.reply(status_codes::OK, "ok");
            }
            message.reply(status_codes::NotFound, "error, header \"id\" missing");
        }
        else if(std::find(paths.begin(), paths.end(), "hasaccess") != paths.end()) {
            string stvalue = message.extract_string().get(); //oid
            if(true) { //TODO aktualisieren!!! //m_pController->getDB_access()->hasAccess(stvalue.c_str())) {
                message.reply(status_codes::OK, "true");
            }
            else {
                message.reply(status_codes::OK, "false");
            }
        }
        else if(std::find(paths.begin(), paths.end(), "vicinity") != paths.end()) {
            handle_VICINITY_POST_request(message, paths);
            message.reply(status_codes::OK, ""); //TODO "" ok?
            return;
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
    try{
        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));

        if(std::find(paths.begin(), paths.end(), "vicinity") != paths.end()) {
            handle_VICINITY_PUT_request(message, paths);
            return;
        }
        message.reply(status_codes::NotFound,"WAT?!");
    }
    catch(exception& e) {
        message.reply(status_codes::BadRequest, e.what());
    }
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


//
// get pulic key, requests the key if it is not in database
//
std::string rest_handler::get_public_key(const char* id, http_request message) {
    //cout << message.remote_address() << endl;
    //cout << message.absolute_uri().port() << endl;
    
    std::string key = m_pController->getDB_access()->get_public_key(id);
    if(key == "") { //key not in database -> get key
        std::string address = "http://";
        address.append(message.remote_address());
        address.append(":4242"); //TODO is there a way to get the port of the incoming message?
        
        http_client client(address.c_str());
        http_response response = client.request(methods::GET, "/publickey").get();
        std::string output = response.extract_string().get();
        m_pController->getDB_access()->insert_public_key(id, output.c_str()); //insert retrieved key into database
        return output;
    } else { //key was already in database
        return key;
    }
}


void rest_handler::handle_VICINITY_GET_request(http_request message, std::vector<utility::string_t> path) {
    // check if request path has correct size
    // exactly two items. could be a request for objects... lets see...
    if(path.size() == 2) {
        // check if Thing Description is requested
        if(path.at(1) == "objects") {
            // TD request
            string td = m_pController->getVICINITY_handler()->generateThingDescription();
            message.reply(status_codes::OK, td);
        }
        else {
            message.reply(status_codes::BadRequest, "What is it?");
        }
    }
    // more than two arguments? most likely read property
    else if(path.size() > 2) {
        // TODO: sanitycheck for properties
//        try {
//        std::regex re("/vicinity/objects/(.+)/properties/(.+)");
//        std::smatch match;
//        if (std::regex_search(subject, match, re) && match.size() == 2) {
//            oid = match.str(1);
//            pid = match.str(2);
//        } else {
//            cout << "no match: " << match.size() << endl;
//        }
//        } catch (std::regex_error& e) {
//        // Syntax error in the regular expression
//        }        
        
        string oid = path[2];
        string pid = path[4];
        string payload = m_pController->getVICINITY_handler()->readProperty(oid, pid);
        message.reply(status_codes::OK, payload);

    }
    // otherwise not enough arguments
    else {
        message.reply(status_codes::BadRequest, "not enough arguments");
    }

//    for(auto it=path.begin(); it != path.end(); ++it){
//        std::cout << *it << std::endl;
//    }

}


void rest_handler::handle_VICINITY_POST_request(http_request message, std::vector<utility::string_t> path) {
    std::cout << "ACTION!!!!" << std::endl;
    std::cout << "ACTION REC PYLD : " + message.extract_string().get() << std::endl;
    std::cout << "REQUEST: " + message.to_string() << std::endl; //korrekt!
            std::cout << "ABS URI: " + message.absolute_uri().to_string() << std::endl;
            std::cout << "REL URI: " + message.relative_uri().to_string() << std::endl;
            std::cout << "REQ URI: " + message.request_uri().to_string() << std::endl;
            
    if(path.size() < 3) {
        message.reply(status_codes::BadRequest, "not enough arguments");
    }
    else {
        if(path.at(1) == "objects" && path.at(3) == "actions") { //is it an action?
            string oid = path[2];
            string aid = path[4];
            std::cout << "ACTION CALLED : " + oid + " " + aid << std::endl; //korrekt!
            string payload = message.extract_string().get();
            std::cout << "ACTION REC PYLD : " + payload << std::endl;
            
            std::cout << "XX: " + message.remote_address() << std::endl;
            
            std::cout << "REQUEST: " + message.to_string() << std::endl; //korrekt!
            std::cout << "ABS URI: " + message.absolute_uri().to_string() << std::endl;
            std::cout << "REL URI: " + message.relative_uri().to_string() << std::endl;
            std::cout << "REQ URI: " + message.request_uri().to_string() << std::endl;
            string sender = "SENDER"; //TODO
            m_pController->getVICINITY_handler()->postAction(oid, aid, payload, sender);
            message.reply(status_codes::OK, ""); //there is always a task id returned, no need for us to return sth.
        }
        else if(path.at(1) == "decrypt") { //decrypt action called
            std::cout << "REQUEST: " + message.to_string() << std::endl; //korrekt!
            std::cout << "ABS URI: " + message.absolute_uri().to_string() << std::endl;
            std::cout << "REL URI: " + message.relative_uri().to_string() << std::endl;
            std::cout << "REQ URI: " + message.request_uri().to_string() << std::endl;
            
            string oid = path[2];
            string payload = message.extract_string().get();
            string sender = "SENDER"; //TODO muesste hier direkt in den params/args dabei sein!!! wie auslesen????
            //m_pController->getVICINITY_handler()->decrypt(oid, sender, payload) TODO
        }
        else if(path.at(1) == "objects" && path.at(3) == "properties") { //is it a property? (needed for actions) TODO is this needed? where to get source id?
            string oid = path[2];
            string pid = path[4];
            std::cout << "POST ACTION PROP : " + pid << std::endl;
        }
    }
}


void rest_handler::handle_VICINITY_PUT_request(http_request message, std::vector<utility::string_t> path) {
    std::cout << "ACTION!!!!" << std::endl;
    if(path.size() < 3) {
        message.reply(status_codes::BadRequest, "not enough arguments");
    }
    else {
        if(path.at(1) == "objects" && path.at(3) == "properties") { //prop?
            string oid = path[2];
            string pid = path[4];
            string payload = message.extract_string().get();
            
            string payload2 = m_pController->getVICINITY_handler()->writeProperty(oid, pid, payload);
            message.reply(status_codes::OK, payload2); //there is always a task id returned, no need for us to return sth.
        }
    }
}
