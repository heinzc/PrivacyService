#include <cpr/cpr.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <json/json.h>
#include <nlohmann/json.hpp>

// To get individual adapter data
void get_data()
{
    
    using nlohmann::json;
    //std::cout << "Get data" << std::endl;
    auto r = cpr::Get(cpr::Url{ "http://localhost:5000/adapter/objects/oid/properties/heartrate" }
                      );
    //std::cout << "Returned Text:" << r.text << std::endl;
    json j = json::parse(r.text);
    std::cout << "Returned json:" << j << std::endl;
    auto result = j["Heartrate_value"];
    std::cout << "Heartrate_value: " << result << std::endl;
    // []() operator[]()'s should be chainable, since they return a reference to the basic_json<> object.
    
}

int main()
{
    using nlohmann::json;
    std::ifstream in("config_adapters.json");  // list of adapter endpoints
    json config_file = json::parse(in);
    std::ofstream output("Encrypted_adapter_TD.json"); // create a Thing-Description file
    
    //check file existance to append json objects
    if (std::ifstream("Encrypted_adapter_TD.json"))
    {

    std::remove("Encrypted_adapter_TD.json");
    std::cout << "File content removed for adding new json objects" << std::endl;
    
        //encrypted thing description structure to add json objects from adapters
    auto adapter_td = R"(
        {
            "adapter-id": "enc_adapter",
            "thing-descriptions": []
           }
           )";

    json td_file = json::parse(adapter_td);

    for (auto& endpoint : config_file["adapters"])
        {
          auto adap_endpoint = endpoint["endpoint"].get<std::string>();
            
          std::string url = adap_endpoint+"/objects";
          auto response = cpr::Get(cpr::Url{ url });
          json json_value = json::parse(response.text);
          //std::cout << "Returned json:" << json_value.dump(4) << std::endl;
        
        for (auto& td : json_value["thing-descriptions"])
            {
              td["oid"] = td["oid"].get<std::string>() + std::string("_enc");
              td["name"] = td["name"].get<std::string>() + std::string("_enc");
            for (auto& prop : td["properties"])
              {
                //prop["pid"] = prop["pid"].get<std::string>() + std::string("_enc");
              }
              json result = json_value["thing-descriptions"][0];
              // And use emplace_back+move to detach the object from `json_value`
            // and move it to the back of `td_file["b"]`.
              td_file["thing-descriptions"].emplace_back(std::move(td));
              std::cout << "Obtained result:" << result.dump(4) << std::endl;
       }
    }
    std::ofstream output_file;  // write the obtained json TD to a file.
    output_file.open("Encrypted_adapter_TD.json",std::ios_base::app );
    output_file << std::setw(4) << td_file.dump(4) << std::endl;
    }
    else
    {
    std::cout << "No file found to add json objects" << std::endl;
    }
    get_data();
    std::cout << "Press ENTER to exit." << std::endl;
    getchar(); //wait for the user to see the results and press a key to end
    
}
