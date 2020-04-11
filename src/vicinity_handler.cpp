#include "../include/vicinity_handler.h"
#include "../include/rest_handler.h"
#include "../include/he_handler.h"

#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include <sstream>
#include <utility>
#include <regex>


using namespace std;
using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;


vicinity_handler::vicinity_handler()
{
    //ctor
}

vicinity_handler::~vicinity_handler()
{
    //dtor
}

void vicinity_handler::initialize(string configfile) {
    // for convenience
    using json = nlohmann::json;

    // read config. could be copied from agent or could be a separate one
    ifstream in(configfile);
    m_configFile = json::parse(in);

    in.close();

    for (auto& adapter : m_configFile["adapters"]) {
        // query the endpoint for /objects
        string adapterid = adapter["adapter-id"].get<string>();
        string endpoint = adapter["endpoint"].get<string>();

        m_endpoints.emplace(make_pair(adapterid, endpoint));
    }
    
    //load parameters
    ifstream in2("ServiceThingDescription.json");
    json thing_description = json::parse(in2);
    in2.close();
    ownOid = thing_description["thing-descriptions"][0]["oid"];
    port = thing_description["port"];
    agentPort = thing_description["agent-port"];
    adapterId = thing_description["adapter-id"];
}

string vicinity_handler::generateThingDescription() {
    // for convenience
    using json = nlohmann::json;

    //encrypted thing description structure to add json objects from adapters
    auto adapter_td = json::parse(R"(
        {
            "adapter-id": ")" + adapterId + R"(",
            "thing-descriptions": []
        }
    )");

    // iterate over all adapters in config file
    for (auto& adapter : m_configFile["adapters"]) {
        // query the endpoint for /objects
        string adapterid = adapter["adapter-id"].get<string>();
        string endpoint = adapter["endpoint"].get<string>();
        http_client client(endpoint);

        // as the requests may take time, we will perform them asynchronous...
        pplx::task<web::http::http_response> requestTask = client.request(methods::GET, uri_builder(U("/objects")).to_string());

        web::http::http_response response = requestTask.get();
        //fix for thing descriptions e.g. of python sample
        std::string responseStr = response.extract_string().get();
        std::replace( responseStr.begin(), responseStr.end(), '\'', '\"'); //replace ' with "
        json objects = json::parse(responseStr);//response.extract_string().get());

        for (auto& td : objects["thing-descriptions"]) {
            td["oid"] = adapterid + std::string(":") + td["oid"].get<std::string>() + std::string("_enc");
            td["name"] = td["name"].get<std::string>() + std::string(" (encrypted)");

            for (auto& prop : td["properties"]) {
                prop["read_link"]["href"] = std::string("/objects/{oid}/properties/{pid}");
                try {
                    prop.erase("write_link");
                }
                catch(const std::exception& e) {
                    cout << "info: remove write link failed" << endl;
                }
            }

            //json result = json_value["thing-descriptions"][0];
            // And use emplace_back+move to detach the object from `json_value`
            // and move it to the back of `td_file["b"]`.
            adapter_td["thing-descriptions"].emplace_back(std::move(td));
            //std::cout << "Obtained result:" << result.dump(4) << std::endl;
        }
    }

    //read and add thing description file for the he value added service
    ifstream in2("ServiceThingDescription.json");
    json thing_description = json::parse(in2);
    //place first thing description at the end of this function's output
    //std::cout << thing_description["thing-descriptions"] << std::endl;
    adapter_td["thing-descriptions"].emplace_back(std::move(thing_description["thing-descriptions"][0]));
    std::cout << adapter_td << std::endl;
    in2.close();
    //adapter_td["thing-descriptions"].emplace_back();

// TODO: handle adapters asynchronously
        // ... and handle the responses when the arrive.
/*        .then([=](http_response response) {
            printf("Received response status code:%u\n", response.status_code());

            json objects = json::parse(response.extract_string().get());

            for (auto& td : objects["thing-descriptions"]) {
                td["oid"] = td["oid"].get<std::string>() + std::string("_enc");
                td["name"] = td["name"].get<std::string>() + std::string(" (encrypted)");

                // change property name as well
                //for (auto& prop : td["properties"])
                //  {
                    //prop["pid"] = prop["pid"].get<std::string>() + std::string("_enc");
                //  }

                //json result = json_value["thing-descriptions"][0];
                // And use emplace_back+move to detach the object from `json_value`
                // and move it to the back of `td_file["b"]`.
                //adapter_td["thing-descriptions"].emplace_back(std::move(td));
                //std::cout << "Obtained result:" << result.dump(4) << std::endl;
            }
        });

        // Wait for all the outstanding I/O to complete and handle any exceptions
        try {
            requestTask.wait();
        }
        catch (const std::exception &e) {
            printf("Error exception:%s\n", e.what());
        }
    }

    */

    return adapter_td.dump();
}

string vicinity_handler::readProperty(string oid, string pid) {
    // for convenience
    using json = nlohmann::json;

    if(oid == ownOid) { //read property of the service itself
        if(pid == "publickey") {
            std::cout << "Publickey requested and returned" << std::endl;
            return std::string("{\"publickey\":\"testpublickey\"}");
            //return std::string("{\"publickey\":\"" + m_pController->getHE_handler()->getPublicKey() + "\"}");
        }
    }
    
    // oid is compount of <adapterid>:<objectid>
    string adapterid, objectid;
    cout << oid << endl;
    try {
        std::regex re("(.+):(.+)");
        std::smatch match;
        if (std::regex_search(oid, match, re) && match.size() == 3) {
            adapterid = match.str(1);
            objectid = match.str(2);

            string adapterendpoint = m_endpoints[adapterid];
            http_client client(adapterendpoint);

            pplx::task<web::http::http_response> requestTask = client.request(methods::GET, uri_builder(U("/objects/" + objectid + "/properties/" + pid)).to_string());

            web::http::http_response response = requestTask.get();
            string rawresult = response.extract_string().get();

            json result = json::parse(rawresult);

            result["value"] = m_pController->getHE_handler()->encrypt_as_string(result["value"].get<long>());
            result["encrypted"] = "True";

            return result.dump();
        } else {
            cout << "no match: " << match.size() << endl;
        }
    } catch (std::regex_error& e) {
    // Syntax error in the regular expression
    }

    return std::string();
}

//TODO this can propably be removed, right?
string vicinity_handler::postAction(string oid, string aid, string payload, string sender) {
    //currently, there are only actions for this he service
    if(oid == ownOid) { //he service
        //which action?
        if(aid == "decrypt") {
            //check, if sender is owner of this service (saved in database)
            
            //decrypt
            
            //return decrypted value
            
        }
        else if(aid == "randomshare") {
            //check if sender is participant
        }
        else if(aid == "aggregate") {
            //check if owner is sender
            //extract participants from payload
            
            //do checks -> as in python....
            
            //send random shares
            //wait for all shares maybe better check if all were received in /randomshare
            //idk yet
        }
        return "";
    }
}

string vicinity_handler::writeProperty(string oid, string pid, string payload) {
    using json = nlohmann::json; // for convenience

    if(oid == ownOid) { //he service
        //which property?
        if(pid == "hasaccess") {
            //get requester id from payload
            std::cout << "payloadXX: " + payload << std::endl;
            json content = json::parse(payload);
            std::cout << "payloadXX: " + std::string(content["requester-id"]) << std::endl;
            //ask db, if requester has access and send result back
            if(m_pController->getDB_access()->hasAccessToData(std::string(content["requester-id"]).c_str(), std::string(content["destination-id"]).c_str())) {
                return "{\"granted\":true}";
            }
            return "{\"granted\":false}";
        }
        return "";
    }
}

string vicinity_handler::getOwnOid() {
    return ownOid;
}

string vicinity_handler::getAdapterId() {
    return adapterId;
}

string vicinity_handler::getAgentPort() {
    return agentPort;
}

string vicinity_handler::getOwnPort() {
    return port;
}

string vicinity_handler::decrypt(string oid, string sourceOid, string payload) { //TODO doesn't need to return something, right?
    using json = nlohmann::json; // for convenience
    
    if(ownOid == oid) { //is the action for this service?
        if(m_pController->getDB_access()->hasAccessToDecrypt(sourceOid.c_str())) { //is own device or we permitted decryption
            //everything ok, let's get value and decrypt it
            json content = json::parse(payload);
            std::string value = string(content["value"]);
            int decryptedValue = m_pController->getHE_handler()->decrypt(value); //TODO might get stuck here, when input is wrong!
            std::cout << "DECRYPT VIC HANDL: " + std::to_string(decryptedValue) << std::endl;           
            updateTaskStatus(string("decrypt"), string("finished"), string("{\"value\": " + std::to_string(decryptedValue) + "}"));
            //return std::to_string(decryptedValue);
            return "";
        }
    }
    
    //update status again: finished or failed?
    updateTaskStatus(string("decrypt"), string("failed"), string("{\"value\":0}"));
    return "";
}

string vicinity_handler::enterAggregation(string oid, string sourceOid, string payload) { //TODO doesn't need to return something, right?
    //check if oid is he oid
    return "";
}

string vicinity_handler::sendShareAction(string oid, string sourceOid, string payload) { //TODO doesn't need to return something, right?
    //check if oid is he oid
    return "";
}

//takes action id, status (failed, running or finished)
bool vicinity_handler::updateTaskStatus(string aid, string status, string payload) {
    std::string address = "http://127.0.0.1:" + agentPort + "/agent/actions/" + aid;
        
    http_client client(address.c_str());
    //client.request(methods::PUT, "", status);

    http_request request(methods::PUT);
    //request.headers().add("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    request.headers().add("adapter-id", adapterId);
    request.headers().add("infrastructure-id", ownOid);
    request.headers().add("status", status.c_str());
    request.set_body(payload.c_str());
    //std::cout << request.to_string() << std::endl;
    //std::cout << request.absolute_uri().to_string() << std::endl;
    
    client.request(request);
    
    std::cout << "sent status update" << std::endl;
    
    //std::string output = response.extract_string().get();
    return true;
}

//passed object id can be the one of a he service or the oid of a data source (encrypted device)
//TODO test this method!!!
string vicinity_handler::getPublicKey(string oid) {
    using json = nlohmann::json; // for convenience
    //get corresponding he service oid
    std::string heOid = m_pController->getDB_access()->getPrivacyService(oid.c_str());
    std::string key = m_pController->getDB_access()->get_public_key(heOid.c_str());
    if(key == "") { //if key empty -> key of he service is not in database yet
        //request public key from heOid
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + heOid + "/properties/publickey";
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        http_client client(address.c_str());
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        http_request request(methods::GET);
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        request.headers().add("adapter-id", adapterId);
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        request.headers().add("infrastructure-id", ownOid);
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        http_response response = client.request(request).get();
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        std::string output = response.extract_string().get();
        std::cout << "output: " + output << std::endl;
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        json jOutput = json::parse(output);
        std::cout << "9999999999999999999999999999999999"<< std::endl;
        std::string receivedKey = jOutput["publickey"];
        std::cout << "9999999999999999999999999999999999"<< std::endl; //hier kommts nicht hin!!
        
        std::cout << "Received KEY: " + receivedKey << std::endl;
        
        //put key in database
        m_pController->getDB_access()->insert_public_key(heOid.c_str(), receivedKey.c_str()); //insert retrieved key into database
        //return new key
        return receivedKey;
    } else {
        return key;
    }
}
