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
}

string vicinity_handler::generateThingDescription() {
    // for convenience
    using json = nlohmann::json;

    //encrypted thing description structure to add json objects from adapters
    auto adapter_td = json::parse(R"(
        {
            "adapter-id": "encryption_adapter",
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
    ifstream in2("ThingDescription.json");
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

            string encValue = m_pController->getHE_handler()->encrypt_as_string(result["value"].get<long>());

            return encValue;
        } else {
            cout << "no match: " << match.size() << endl;
        }
    } catch (std::regex_error& e) {
    // Syntax error in the regular expression
    }

    return std::string();
}

string vicinity_handler::postAction(string oid, string aid, string payload, string sender) {
    //currently, there are only actions for this he service
    getOwnOid();
    if(oid == getOwnOid()) { //he service
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

    getOwnOid();
    if(oid == getOwnOid()) { //he service
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
    // for convenience
    using json = nlohmann::json;
    ifstream in("ThingDescription.json");
    json thing_description = json::parse(in);
    in.close();
    return thing_description["thing-descriptions"][0]["oid"];
}
