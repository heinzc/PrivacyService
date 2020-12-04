#include "../include/rest_handler.h"

//#include "FHE.h"
#include "../include/he_handler.h"
#include "../include/vicinity_handler.h"

//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>
//
//#include <exception>
//#include <regex>

//#include <cpprest/http_client.h>

#include <QtCore>

#include "../third-party/qthttpserver/src/httpserver/qhttpserver.h"


//using namespace web::http;
//using namespace web::http::client; 

rest_handler::rest_handler()
{
    //ctor
}
//rest_handler::rest_handler(utility::string_t url):m_listener(url)
//{
//    //m_listener.support(methods::GET, std::bind(&rest_handler::handle_get, this, std::placeholders::_1));
//    //m_listener.support(methods::PUT, std::bind(&rest_handler::handle_put, this, std::placeholders::_1));
//    //m_listener.support(methods::POST, std::bind(&rest_handler::handle_post, this, std::placeholders::_1));
//    //m_listener.support(methods::DEL, std::bind(&rest_handler::handle_delete, this, std::placeholders::_1));
//
//    input_counter = 0;
//}
rest_handler::~rest_handler()
{
    //dtor
}

void rest_handler::setController(he_controller * controller) {
    m_pController = controller;
}

int rest_handler::setup() {
    m_pHttpServer = new QHttpServer();
    m_pHttpServer->route("/", []() {
        qDebug() << "hi";

        return "Hello world";
        });

    const auto port = m_pHttpServer->listen(QHostAddress::Any, 4242);
    if (!port) {
        qDebug() << QCoreApplication::translate(
            "QHttpServerExample", "Server failed to listen on a port.");
        return 0;
    }

    qDebug() << QCoreApplication::translate(
        "QHttpServerExample", "Running on http://127.0.0.1:%1/ (Press CTRL+C to quit)").arg(port);

    return 0;
}


//void rest_handler::handle_error(pplx::task<void>& t)
//{
//    try
//    {
//        t.get();
//    }
//    catch(...)
//    {
//        // Ignore the error, Log it if a logger is available
//    }
//}
//
//
////
//// Get Request 
////
//void rest_handler::handle_get(http_request message) {
//    std::cout << "GET!" << std::endl;
//    try {
//        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));
//
//        if(std::find(paths.begin(), paths.end(), "aggregation") != paths.end()) { //DEPRECATED
//            if(input_counter < 3) {
//                message.reply(status_codes::Forbidden, "not enough inputs provided");
//                return;
//            } else {
//                m_pController->getHE_handler()->aggregate(input_counter);
//                message.reply(status_codes::OK, "OK");
//                return;
//            }
//        } else if(std::find(paths.begin(), paths.end(), "sum") != paths.end()) { //DEPRECATED
//            int sum = m_pController->getHE_handler()->getSum();
//            message.reply(status_codes::OK, to_string(sum));
//        } else if(std::find(paths.begin(), paths.end(), "vicinity") != paths.end()) {
//            handle_VICINITY_GET_request(message, paths);
//            return;
//        }
//        message.reply(status_codes::NotFound,"WAT?!");
//    }
//    catch(exception& e) {
//        message.reply(status_codes::BadRequest, e.what());
//    }
//    return ;
//
///*     concurrency::streams::fstream::open_istream(U("static/index.html"), std::ios::in).then([=](concurrency::streams::istream is)
//    {
//        message.reply(status_codes::OK, is,  U("text/html"))
//		.then([](pplx::task<void> t)
//		{
//			try{
//				t.get();
//			}
//			catch(...){
//				//
//			}
//	});
//    }).then([=](pplx::task<void>t)
//	{
//		try{
//			t.get();
//		}
//		catch(...){
//			message.reply(status_codes::InternalError,U("INTERNAL ERROR "));
//		}
//	}); */
//}
//
////
//// A POST request
////
//void rest_handler::handle_post(http_request message) {
//    std::cout << "POST!" << std::endl;
//    try{
//        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));
//
//        if(std::find(paths.begin(), paths.end(), "produce") != paths.end()) { //DEPRECATED
//            string stvalue = message.extract_string().get();
//            produce_ctxt(stvalue);
//            message.reply(status_codes::OK,message.to_string());
//            input_counter++;
//        }
//        else if(std::find(paths.begin(), paths.end(), "encrypt") != paths.end()) {
//            std::cout << "Encrypt called locally." << std::endl;
//            string stvalue = message.extract_string().get();
//            
//            message.reply(status_codes::OK,encrypt_ptxt(stvalue));
//        }
//        else if(std::find(paths.begin(), paths.end(), "add") != paths.end()) { //DEPRECATED
//            string stvalue = message.extract_string().get();
//            
//            m_pController->getHE_handler()->add(stvalue, "placeholder");
//            message.reply(status_codes::OK,"läuft!");
//        }
//        else if(std::find(paths.begin(), paths.end(), "aggregate") != paths.end()) {
//            std::cout << "Aggregate called locally." << std::endl;
//            //string stvalue = message.extract_string().get();
//            //std::cout << "Body: " + stvalue << std::endl;
//            json::object request_json = message.extract_json().get().as_object();
//            if(request_json.find("values") != request_json.end()) {
//                json::array values = request_json.at("values").as_array();
//                std::vector<std::string> vec;
//                for(auto it = values.begin(); it != values.end(); ++it) {
//                    //std::cout << it->as_string() << std::endl;
//                    try {
//                        vec.push_back(it->as_string());
//                    } catch (...) {
//                        message.reply(status_codes::BadRequest,"Payload incorrect");
//                        return;
//                    }
//                }
//                std::string sourceOid = "";
//                if(message.headers().has(U("sourceOid"))) { //does not need to be provided
//                    sourceOid = ::utility::conversions::to_utf8string(message.headers().operator[](U("sourceOid")));
//                }
//                std::string result = "";
//                if(sourceOid == "") {
//                    result = m_pController->getHE_handler()->aggregate(vec, ""); //use own key
//                } else {
//                    result = m_pController->getHE_handler()->aggregate(vec, (m_pController->getVICINITY_handler()->getPublicKey(sourceOid)).c_str()).c_str();
//                }
//                //std::cout << "Result of aggregation: " + result << std::endl;
//                std::cout << "Aggregate locally finished." << std::endl;
//                message.reply(status_codes::OK, result);
//                return;
//            }
//        }
//        else if(std::find(paths.begin(), paths.end(), "decrypt") != paths.end()) {
//            std::cout << "Decrypt called locally." << std::endl;
//            string stvalue = message.extract_string().get();
//
//            message.reply(status_codes::OK, m_pController->getHE_handler()->decrypt(stvalue));
//        }
//        else if(std::find(paths.begin(), paths.end(), "hasaccess") != paths.end()) { //DEPRECATED
//            string stvalue = message.extract_string().get(); //oid
//            if(m_pController->getDB_access()->hasAccessToDecrypt(stvalue.c_str())) {
//                message.reply(status_codes::OK, "true");
//            }
//            else {
//                message.reply(status_codes::OK, "false");
//            }
//        }
//        else if(std::find(paths.begin(), paths.end(), "vicinity") != paths.end()) {
//            handle_VICINITY_POST_request(message, paths);
//            return;
//        }
//        message.reply(status_codes::NotFound,"WAT?!");
//    }
//    catch(exception& e) {
//        message.reply(status_codes::BadRequest, e.what());
//    }
//    return ;
//}
//
////
//// A DELETE request
////
//void rest_handler::handle_delete(http_request message)
//{
//     ucout <<  message.to_string() << endl;
//
//        string rep = U("WRITE YOUR OWN DELETE OPERATION");
//      message.reply(status_codes::OK,rep);
//    return;
//}
//
//
////
//// A PUT request 
////
//void rest_handler::handle_put(http_request message)
//{
//    try{
//        auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));
//
//        if(std::find(paths.begin(), paths.end(), "vicinity") != paths.end()) {
//            handle_VICINITY_PUT_request(message, paths);
//            return;
//        }
//        message.reply(status_codes::NotFound,"WAT?!");
//    }
//    catch(exception& e) {
//        message.reply(status_codes::BadRequest, e.what());
//    }
//    return;
//}
//
//
//void rest_handler::produce_ctxt(string pt) {
//    int value = stoi(pt);
//
//    //m_pController->getHE_handler()->encrypt_and_store(value, input_counter);
//}
//
//
//string rest_handler::encrypt_ptxt(string pt) {
//    int value = stoi(pt);
//
//    return m_pController->getHE_handler()->encrypt_as_string(value);
//}
//
//void rest_handler::handle_VICINITY_GET_request(http_request message, std::vector<utility::string_t> path) {
//    std::cout << "VICINITY Get Request!" << std::endl;
//    // check if request path has correct size
//    // exactly two items. could be a request for objects... lets see...
//    if(path.size() == 2) {
//        // check if Thing Description is requested
//        if(path.at(1) == "objects") {
//            // TD request
//            string td = m_pController->getVICINITY_handler()->generateThingDescription();
//            message.reply(status_codes::OK, td);
//        }
//        else {
//            message.reply(status_codes::BadRequest, "What is it?");
//        }
//    }
//    // more than two arguments? most likely read property
//    else if(path.size() > 2) {
//        // TODO: sanitycheck for properties
////        try {
////        std::regex re("/vicinity/objects/(.+)/properties/(.+)");
////        std::smatch match;
////        if (std::regex_search(subject, match, re) && match.size() == 2) {
////            oid = match.str(1);
////            pid = match.str(2);
////        } else {
////            cout << "no match: " << match.size() << endl;
////        }
////        } catch (std::regex_error& e) {
////        // Syntax error in the regular expression
////        }        
//        
//        string oid = path[2];
//        string pid = path[4];
//        string payload = m_pController->getVICINITY_handler()->readProperty(oid, pid);
//        std::cout << "Read Property. Sending answer back." << std::endl;
//        message.reply(status_codes::OK, payload);
//
//    }
//    // otherwise not enough arguments
//    else {
//        message.reply(status_codes::BadRequest, "not enough arguments");
//    }
//
////    for(auto it=path.begin(); it != path.end(); ++it){
////        std::cout << *it << std::endl;
////    }
//
//}
//
//
//void rest_handler::handle_VICINITY_POST_request(http_request message, std::vector<utility::string_t> path) {
//    std::cout << "VICINITY Post Request!" << std::endl;
//    if(path.size() < 3) {
//        message.reply(status_codes::BadRequest, "not enough arguments");
//    }
//    else { //seems to be an action
//        string sourceOid = message.absolute_uri().query();
//        //remove "sourceOid=" from sourceOid (request parameter)
//        string toRemove = "sourceOid=";
//        std::string::size_type i = sourceOid.find(toRemove);
//        if (i != std::string::npos) sourceOid.erase(i, toRemove.length());
//        string oid = path[2];
//        string aid = path.at(1);
//        string payload = message.extract_string().get(); //can only be extracted once!
//        message.reply(status_codes::OK, "{\"status\":\"running\"}");
//        m_pController->getVICINITY_handler()->postAction(oid, aid, payload, sourceOid);
//    }
//}
//
//
//void rest_handler::handle_VICINITY_PUT_request(http_request message, std::vector<utility::string_t> path) {
//    std::cout << "VICINITY Put Request!" << std::endl;
//    if(path.size() < 4) {
//        message.reply(status_codes::BadRequest, "not enough arguments");
//    }
//    else {
//        if(path.at(1) == "objects" && path.at(3) == "properties") { //property
//            string oid = path[2];
//            string pid = path[4];
//            string payload = message.extract_string().get();
//            
//            string payload2 = m_pController->getVICINITY_handler()->writeProperty(oid, pid, payload);
//            message.reply(status_codes::OK, payload2);
//        }
//    }
//}
