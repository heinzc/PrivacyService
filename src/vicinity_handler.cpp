#include "../include/vicinity_handler.h"
#include "../include/rest_handler.h"

#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include <nlohmann/json.hpp>

#include <sstream>

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

string vicinity_handler::generateThingDescription() {
    // for convenience
    using json = nlohmann::json;

    // read config. could be copied from agent or could be a separate one
    ifstream in("config_adapters.json");  // hard coded in working folder for now. config parameter in the future
    json config_file = json::parse(in);
    
    //encrypted thing description structure to add json objects from adapters
    auto adapter_td = json::parse(R"(
        {
            "adapter-id": "encryption_adapter",
            "thing-descriptions": []
        }
    )");

    // iterate over all adapters in config file
    for (auto& adapter : config_file["adapters"]) {
        // query the endpoint for /objects
        string endpoint = adapter["endpoint"].get<string>();
        http_client client(endpoint);

        // as the requests may take time, we will perform them asynchronous...
        pplx::task<web::http::http_response> requestTask = client.request(methods::GET, uri_builder(U("/objects")).to_string());

        web::http::http_response response = requestTask.get();
        json objects = json::parse(response.extract_string().get());

        for (auto& td : objects["thing-descriptions"]) {
            td["oid"] = td["oid"].get<std::string>() + std::string("_enc");
            td["name"] = td["name"].get<std::string>() + std::string(" (encrypted)");

            //json result = json_value["thing-descriptions"][0];
            // And use emplace_back+move to detach the object from `json_value`
            // and move it to the back of `td_file["b"]`.
            adapter_td["thing-descriptions"].emplace_back(std::move(td));
            //std::cout << "Obtained result:" << result.dump(4) << std::endl;
        }
    }

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

string vicinity_handler::readProperty(string endpoint) {
    std::cout << endpoint << std::endl;

    return std::string();
}