#include "../include/vicinity_handler.h"
#include "../include/rest_handler.h"
#include "../include/he_handler.h"

#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include <sstream>
#include <utility>
#include <regex>

#include <unistd.h> //usleep

#include <boost/chrono.hpp>

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
    
    srand (time(NULL)); //initialize random seed with current time
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
    std::cout << string("Read Property; Oid: ") + oid + string(", Pid: ") + pid << std::endl;
    if(oid == ownOid) { //read property of the service itself
        if(pid == "publickey") {
            std::cout << "Publickey requested and returned" << std::endl;
            json keyJson = json::parse(std::string("{}"));
            keyJson["publickey"] = m_pController->getHE_handler()->getPublicKey();
            //std::cout << std::string("My public key: ") + keyJson.dump() << std::endl;
            return keyJson.dump();
        }
        else if(pid == "trustedparties") {
            std::vector<std::string> trustedParties = m_pController->getDB_access()->getTrustedParties();
            json parties = json::parse("{\"parties\":[]}");
            for(std::string p : trustedParties) {
                parties["parties"].push_back(p);
                std::cout << p << '\n';
            }
            //std::cout << "Trusted parties requested: " + parties.dump() << '\n';
            std::cout << "Trusted parties requested and returned" << std::endl;
            return parties.dump();
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

//when an action request is received, this redirects it to the correct function
void vicinity_handler::postAction(string oid, string aid, string payload, string sourceOid) {
    //currently, there are only actions for this he service
    if(oid == ownOid) { //he service
        //which action?
        if(aid == "decryptaction") {
            m_pController->getVICINITY_handler()->decrypt(oid, sourceOid, payload);
        }
        else if(aid == "randomshareaction") {
            m_pController->getVICINITY_handler()->sendShareAction(oid, sourceOid, payload);
        }
        else if(aid == "aggregationaction") {
            m_pController->getVICINITY_handler()->startAggregation(oid, sourceOid, payload);
        }
        else if(aid == "participateinaggregationaction") {
            m_pController->getVICINITY_handler()->participateInAggregation(oid, sourceOid, payload);
        }
        return;
    }
}

string vicinity_handler::writeProperty(string oid, string pid, string payload) {
    using json = nlohmann::json; // for convenience

    if(oid == ownOid) { //he service
        //which property?
        if(pid == "hasaccess") {
            //get requester id from payload
            std::cout << "payloadXX: " + payload << std::endl;
            json content;
            std::string requesterId = "";
            std::string destinationOid = "";
            try {
                content = json::parse(payload);
                requesterId = std::string(content["requester-id"]);
                destinationOid = std::string(content["destination-id"]);
                std::cout << "payloadXX: " + std::string(content["requester-id"]) << std::endl;
            } catch (...) {
                std::cout << "Error in writeProperty input payload!" << std::endl;
                return "";
            }
            //ask db, if requester has access and send result back
            if(m_pController->getDB_access()->hasAccessToData(requesterId.c_str(), destinationOid.c_str())) {
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

void vicinity_handler::decrypt(string oid, string sourceOid, string payload) {
    using json = nlohmann::json; // for convenience
    std::cout << "Decrypt start" << std::endl;
    if(ownOid == oid) { //is the action for this service?
        if(m_pController->getDB_access()->hasAccessToDecrypt(sourceOid.c_str())) { //is own device or we permitted decryption
            //everything ok, let's get value and decrypt it
            json content;
            std::string value;
            try {
                content = json::parse(payload);
                value = string(content["value"]);
            } catch (...) {
                std::cout << "Error in input payload of decrypt action!" << std::endl;
                updateTaskStatus(string("decrypt"), string("failed"), string("{\"value\":0}"));
                return;
            }
            //std::cout << "Value to decrypt: " + value << std::endl;
            try {
                int decryptedValue = m_pController->getHE_handler()->decrypt(value);
                std::cout << "Decrypted value: " + std::to_string(decryptedValue) << std::endl;
                //std::cout << "Payload update: " + string("{\"value\": " + std::to_string(decryptedValue) + "}") << std::endl;
                updateTaskStatus(string("decrypt"), string("finished"), string("{\"value\": " + std::to_string(decryptedValue) + "}"));
                return;
            }
            catch(...) {
                std::cout << "Error while decrypting!" << std::endl;
            }
        }
    }
    //update status
    updateTaskStatus(string("decrypt"), string("failed"), string("{\"value\":0}"));
    std::cout << "Decrypt end" << std::endl;
    return;
}

void vicinity_handler::startAggregation(string oid, string sourceOid, string payload) {
    using json = nlohmann::json; //for convenience
    
    m_pController->getDB_access()->resetBlindedMeasurements();
    
    if(ownOid == oid) { //is the action for this service?
        if(m_pController->getDB_access()->isOwnDevice(sourceOid.c_str())) { //only has to be called from owner devices (check sourceOid)
            //get participant he services, devices and properties
            std::cout << "Start Aggregation Payload: " + payload << std::endl;
            json jPayload;
            try {
                jPayload = json::parse(payload);
            } catch (...) {
                std::cout << "Error in input payload of start aggregation action!" << std::endl;
                updateTaskStatus(std::string("aggregation"), std::string("failed"), std::string("{\"value\":0}"));
                return;
            }
            if(jPayload.find("devices") != jPayload.end() && jPayload.find("properties") != jPayload.end() && jPayload.find("participants") != jPayload.end()) {
                auto devices = jPayload["devices"].get<std::vector<std::string>>();
                auto properties = jPayload["properties"].get<std::vector<std::string>>();
                auto participants = jPayload["participants"].get<std::vector<std::string>>();
                //has every vector the same length?
                if(static_cast<int>(devices.size()) == static_cast<int>(properties.size()) && static_cast<int>(properties.size()) == static_cast<int>(participants.size())) {
                    //are there at least three different he services? minimum for safe aggregation
                    std::set<std::string> participantsSet;
                    for (auto i : participants)
                    {
                        participantsSet.insert(i);
                    }
                    if(participantsSet.size() >= 3) {
                        std::cout << "There are at least 3 participants" << std::endl;
                        std::vector<std::string> taskIds;
                        std::vector<std::string> taskExecutors;
                        //insert all in database so we can receive the blinded measurements later
                        for (auto &p : participantsSet) {
                            m_pController->getDB_access()->insertParticipantBlindedMeasurements(p.c_str());
                            //call aggregation action of participant (every participant once)
                            //oid of destination has to be replaced with "you", makes setup easier (no need to save the oid manually)
                            std::cout << "Ask participant to participate in aggregation." << std::endl;
                            std::string taskId = askToParticipateInAggregation(p, replaceAll(payload, p, std::string("you")));
                            std::cout << std::string("Task ID: ") + taskId << std::endl; 
                            taskIds.push_back(taskId);
                            taskExecutors.push_back(p);
                        }
                        //wait until all blined measurements were received. meanwhile, if any task failed -> return; timeout of 60 seconds
                        time_t startWaitingForTasks = time(NULL); //current time
                        boost::chrono::system_clock::time_point startWaitingForTasks2 = boost::chrono::system_clock::now();
                        int pollCount = 0;
                        while(static_cast<int>(taskIds.size()) > 0 && time(NULL) < startWaitingForTasks + 60) { //or while !allBlindedMeasurementsReceived()
                            //taskIds and taskExecutor (should) have the same size, however, we better check it
                            if(static_cast<int>(taskIds.size()) == static_cast<int>(taskExecutors.size()) && static_cast<int>(taskIds.size()) > 0) {
                                for(int i = 0; i < static_cast<int>(taskIds.size()); i++) {
                                    //get status of task i
                                    pollCount++;
                                    std::string status = getStatusOfAction(taskExecutors.at(i), std::string("participateInAggregation"), taskIds.at(i));
                                    //std::cout << "Status of a task: " + status << std::endl;
                                    if(status == "failed") { //then, this task (aggregation itself) failed also
                                        std::cout << "Task of a participant failed." << std::endl;
                                        updateTaskStatus(std::string("aggregation"), std::string("failed"), std::string("{\"value\":0}"));
                                        return;
                                    }
                                    else if(status == "finished") {
                                        //get result
                                        std::cout << "One of the participateInAggregation tasks is finished." << std::endl;
                                        std::string recPayload = getReturnValueOfAction(taskExecutors.at(i), std::string("participateInAggregation"), taskIds.at(i));
                                        json jRecPayload = json::parse(recPayload);
                                        
                                        //std::cout << std::string("Received payload: ") + recPayload << std::endl;
                                        std::cout << std::string("Received value: ") + jRecPayload["value"].dump() << std::endl;
                                        int blindedMeasurement = atoi(std::string(jRecPayload["value"].dump()).c_str());
                                        m_pController->getDB_access()->updateBlindedMeasurement(taskExecutors.at(i).c_str(), blindedMeasurement);
                                        taskIds.erase(taskIds.begin()+i);
                                        taskExecutors.erase(taskExecutors.begin()+i);
                                        break;
                                    }
                                }
                            }
                            else {
                                //should never be here, so update task status to failed, just in case
                                std::cout << "StartAggregation failed! Should not reach this!" << std::endl;
                                updateTaskStatus(std::string("aggregation"), std::string("failed"), std::string("{\"value\":0}"));
                                return;
                            }
                            //sleep(1); //1 second, lower in real world usage
                            usleep(200 * 1000); //200 miliseconds
                        }
                        boost::chrono::milliseconds waitingTime = boost::chrono::duration_cast<boost::chrono::milliseconds> (boost::chrono::system_clock::now() - startWaitingForTasks2);
                        std::cout << "Polling messages: " + std::to_string(pollCount) << std::endl; 
                        std::cout << "Waiting time for shares: " << waitingTime.count() << " ms" << std::endl;
                        if(m_pController->getDB_access()->allBlindedMeasurementsReceived()) { //all shares should have been received, but better check it
                            std::cout << "All blinded measurements received." << std::endl;
                            //add blinded measurements -> calculate result
                            int aggregationResult = m_pController->getDB_access()->getBlindedMeasurementSum();
                            //update status to finished and return result
                            std::cout << "Aggregation result: " + std::to_string(aggregationResult) << std::endl;
                            updateTaskStatus(std::string("aggregation"), std::string("finished"), std::string("{\"value\":") + std::to_string(aggregationResult) + std::string("}"));
                            return;
                        }
                    }
                }
            }
        }
    } 
    //update status failed
    std::cout << "StartAggregation failed!" << std::endl;
    updateTaskStatus(std::string("aggregation"), std::string("failed"), std::string("{\"value\":0}"));
    return;
}

void vicinity_handler::participateInAggregation(string oid, string sourceOid, string payload) {
    using json = nlohmann::json; //for convenience
    
    //following actually not needed, since they are deleted at the end. just to be safe.
    m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator

    std::cout << "//////////////////PIA//////////////////" << std::endl;
    std::cout << "Aggregation action payload: " + payload << std::endl;
    if(ownOid == oid) { //is the action for this service?
        if(m_pController->getDB_access()->isTrustedInitiator(sourceOid.c_str())) { //is initiator allowed to initiate aggregation? (do we trust?)
            //get participant he services, devices and properties
            json jPayload;
            try {
                jPayload = json::parse(payload);
            } catch (...) {
                std::cout << "Error in input payload of participate in aggregation action!" << std::endl;
                updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
                return;
            }
            if(jPayload.find("devices") != jPayload.end() && jPayload.find("properties") != jPayload.end() && jPayload.find("participants") != jPayload.end()) {
                auto devices = jPayload["devices"].get<std::vector<std::string>>();
                auto properties = jPayload["properties"].get<std::vector<std::string>>();
                auto participants = jPayload["participants"].get<std::vector<std::string>>();
                //have devices, properties and participants the same length?
                if(static_cast<int>(devices.size()) == static_cast<int>(properties.size()) && static_cast<int>(properties.size()) == static_cast<int>(participants.size())) {
                    //are there at least three different participants? minimum for safe aggregation
                    std::set<std::string> participantsSet;
                    for (auto i : participants)
                    {
                        participantsSet.insert(i);
                    }
                    if(participantsSet.size() >= 3) {
                        std::cout << "There are at least 3 participants" << std::endl;
                        //determine, which participants are part of my trusted parties
                        std::set<std::string> participatingTrustedPartiesSet;
                        for(auto i : participantsSet) {
                            if(i != "you") { //cannot be myself
                                if(m_pController->getDB_access()->trustsMe(i.c_str())) { //only if participant trusts me
                                    participatingTrustedPartiesSet.insert(i);
                                }
                            }
                        }
                        //is there at least one participant, who i trust?
                        if(participatingTrustedPartiesSet.size() >= 1) {                        
                            std::cout << "There is at least 1 trusted party" << std::endl;
                            //am i really a part of this aggregation?
                            if(participantsSet.find("you") != participantsSet.end()) {
                                std::cout << "I am part of this aggregation." << std::endl;
                                
                                //get sum of requested own device properties
                                long sumOfOwnDeviceProperties = 0;
                                for(int i = 0; i < devices.size(); i++) {
                                    if(participants.at(i) == "you") {
                                        //are the as mine marked devices really mine?
                                        if(m_pController->getDB_access()->isOwnDevice(devices.at(i).c_str())) {
                                            std::cout << std::string("Get plain value of: ") + devices.at(i) + std::string(", ") + properties.at(i) << std::endl;
                                            sumOfOwnDeviceProperties += getPropertyOfEncryptedDevice(devices.at(i), properties.at(i));
                                        }
                                        else {
                                            std::cout << "One of the requested devices is not mine! Aggregation failed!" << std::endl;
                                            m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
                                            updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
                                            return;
                                        }
                                    }
                                }
                                std::cout << "Sum of own device properties: " + std::to_string(sumOfOwnDeviceProperties) << std::endl;
                                
                                //insert participants who trust me in database (random shares)
                                for(auto &tp : participatingTrustedPartiesSet) {
                                    std::cout << "Insert participant who trusts me in database for aggregation." << std::endl;
                                    m_pController->getDB_access()->insertParticipantRandomShares(sourceOid.c_str(), tp.c_str());
                                }
                                long sentSharesSum = 0;
                                //send to all trusted participants a share and save their sum
                                std::vector<std::string> taskIds;
                                std::vector<std::string> taskExecutors;
                                for(auto &tp : participatingTrustedPartiesSet) {
                                    //generate share and add to all shares
                                    long share = rand() % 100000; //random share value: here, 0-99999 possible
                                    sentSharesSum += share;
                                    //send share
                                    std::cout << "Send random share to participating trusted party." << std::endl;
                                    std::string taskId = "";
                                    for(int i = 0; i < 3 && taskId == std::string(""); i++) { //try to send share max 3 times
                                        taskId = sendRandomShare(tp, sourceOid, share);
                                    }
                                    if(taskId == std::string("")) {
                                        std::cout << "Even after retries, was not able to send share. Task failed." << std::endl;
                                        m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
                                        updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
                                        return;
                                    }
                                    std::cout << "Sent Random Share Task ID: " + taskId << std::endl;
                                    //save task id and task executor
                                    taskIds.push_back(taskId);
                                    taskExecutors.push_back(tp);
                                }

                                //wait until all tasks were finished or at least one failed (the send share ones).
                                time_t startWaitingForTasks = time(NULL); //current time
                                //time_t startRunning = time(NULL); //current time
                                
                                
                                //it is not necessary, to check if shares were successful. however,we cannot react, if they were not
                                /*
                                std::cout << "XXXBefore While loop" << std::endl;
                                int pollCount = 0;
                                while(static_cast<int>(taskIds.size()) > 0 && time(NULL) < startWaitingForTasks + 30) { // && time(NULL) < startRunning + 5) { //30 second total timeout
                                    std::cout << "XXXWhile loop" << std::endl;
                                    //taskIds and taskExecutor have the same size, however, we better check it
                                    if(static_cast<int>(taskExecutors.size()) == static_cast<int>(taskExecutors.size())) {
                                        //std::cout << "XXXSame size" << std::endl;
                                        for(int i = 0; i < static_cast<int>(taskExecutors.size()); i++) {
                                            std::cout << "XXXFor loop" << std::endl;
                                            //get status of task i
                                            pollCount++;
                                            std::string status = getStatusOfAction(taskExecutors.at(i), std::string("randomshare"), taskIds.at(i));
                                            //std::cout << "Task Status: " + status << std::endl;
                                            if(status == "failed") { //then, this task (aggregation itself) failed also
                                                std::cout << "Randomshare task of a trusted participant failed." << std::endl;
                                                m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
                                                updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
                                                return;
                                            }
                                            else if(status == "finished") {
                                                std::cout << std::string("Randomshare task of a trusted participant is finished.") + taskIds.at(i) << std::endl;
                                                taskIds.erase(taskIds.begin()+i);
                                                taskExecutors.erase(taskExecutors.begin()+i);
                                                break;
                                            }
                                            
                                            //it can happen, that updating task to "running" was received after "finished"
                                            else if(status == "running") {
                                                pollCount++;
                                                std::string returnValue = getReturnValueOfAction(taskExecutors.at(i), std::string("randomshare"), taskIds.at(i));
                                                json jReturnValue;
                                                try {
                                                    jReturnValue = json::parse(returnValue);
                                                } catch (...) {
                                                    std::cout << "Error when parsing return value in participate in aggregation." << std::endl;
                                                    updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
                                                    return;
                                                }
                                                if(jReturnValue.find("declined") != jReturnValue.end()) { //this task was actually finished, since the payload is there. can happen if gateway receives messages out of order
                                                    //delete task
                                                    deleteTask(taskExecutors.at(i), std::string("randomshare"), taskIds.at(i));
                                                    if(jReturnValue["declined"].get<bool>() == 0) {
                                                        //remove this task
                                                        taskIds.erase(taskIds.begin()+i);
                                                        taskExecutors.erase(taskExecutors.begin()+i);
                                                        break;
                                                    }
                                                    else {
                                                        std::cout << "Randomshare task of a trusted participant failed. (Wrong update as well)" << std::endl;
                                                        m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
                                                        updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
                                                        return;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    usleep(200 * 1000); //200 miliseconds
                                }
                                std::cout << "Polling messages: " + std::to_string(pollCount) << std::endl;
                                if(static_cast<int>(taskIds.size()) > 0) {
                                    std::cout << "Timeout when waiting for our random share tasks to be finished. Stop." << std::endl;
                                    //Delete tasks, that are still running (the ones in the vectors)
                                    if(static_cast<int>(taskExecutors.size()) == static_cast<int>(taskExecutors.size())) {
                                        for(int i = 0; i < static_cast<int>(taskExecutors.size()); i++) {
                                            deleteTask(taskExecutors.at(i), std::string("randomshare"), taskIds.at(i));
                                        }
                                    }
                                    m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
                                    updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
                                    return;
                                }
                                */
                                
                                time_t start = time(NULL); //current time
                                std::cout << "Waiting for shares..." << std::endl;
                                while(!m_pController->getDB_access()->allRandomSharesReceived(sourceOid.c_str()) && time(NULL) < start + 10) {
                                    //std::cout << "Waiting for shares..." << std::endl;
                                    //sleep(1);
                                    usleep(50 * 1000); //50 miliseconds
                                }
                                //all shares were received? Now, we either received all, or we do not want to wait any longer for them
                                if(m_pController->getDB_access()->allRandomSharesReceived(sourceOid.c_str())) {
                                    std::cout << "All random shares received." << std::endl;
                                    //calculate blinded measurement
                                    int receivedShares = m_pController->getDB_access()->getRandomShareSum(sourceOid.c_str());
                                    long blindedMeasurement = sentSharesSum - receivedShares + sumOfOwnDeviceProperties;
                                    //return result
                                    m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
                                    std::cout << "Participate In Aggregation Output: " + std::to_string(blindedMeasurement) << std::endl;
                                    updateTaskStatus(std::string("participateInAggregation"), std::string("finished"), std::string("{\"value\":") + std::to_string(blindedMeasurement) + std::string("}"));
                                    return;
                                } else {
                                    std::cout << "Timeout when waiting for shares." << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "Participate in Aggregation task failed." << std::endl;
    m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
    updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
    return;
}

void vicinity_handler::sendShareAction(string oid, string sourceOid, string payload) {
    std::cout << "Randomshare action called, try to insert share in database." << std::endl;
    using json = nlohmann::json; // for convenience
    if(ownOid == oid) { //is the action for this service?
        if(m_pController->getDB_access()->trustsMe(sourceOid.c_str())) { //does this service trust me?
            //get share value and aggregation origin
            json jPayload;
            string initiator;
            int share;
            try {
                jPayload = json::parse(payload);
                initiator = jPayload["initiator"];
                std::cout << "share: " + jPayload["share"].dump() << std::endl;
                share = atoi(std::string(jPayload["share"].dump()).c_str());
            } catch (...) {
                std::cout << "Error in input payload of send share action!" << std::endl;
                updateTaskStatus(string("randomshare"), string("failed"), string("{\"declined\":true}"));
                return;
            }
            //is this participant inserted? did we even receive the request to aggregate yet?
            time_t startWaiting = time(NULL); //current time
            while(time(NULL) < startWaiting + 3) {
                if(m_pController->getDB_access()->isAlreadyInsertedInRandomShares(initiator.c_str(), sourceOid.c_str())) {
                    //insert data in database; uses UPDATE operator -> if sourceOid is no participant, nothing happens -> no need to check, if sourcOid is part of aggregation
                    m_pController->getDB_access()->updateShareRandomShares(initiator.c_str(), sourceOid.c_str(), share);
                    updateTaskStatus(string("randomshare"), string("finished"), string("{\"declined\":false}"));
                    std::cout << "Randomshare task finished." << std::endl;
                    return;
                } else {
                    usleep(50 * 1000); //50 miliseconds sleep. maybe it takes some more time, until we receive a request to aggregate
                }
            }
        }
    }
    std::cout << "Randomshare task failed." << std::endl;
    updateTaskStatus(string("randomshare"), string("failed"), string("{\"declined\":true}"));
    return;
}

//takes action id, status (failed, running or finished)
bool vicinity_handler::updateTaskStatus(string aid, string status, string payload) {
    using json = nlohmann::json; // for convenience
    //sleep(4);
    std::string address = "http://127.0.0.1:" + agentPort + "/agent/actions/" + aid;
    //std::cout << "Update Task Status Address: " + address << std::endl;
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
    pplx::task<web::http::http_response> requestTask = client.request(request);
    web::http::http_response response = requestTask.get();
    std::string responseStr = response.extract_string().get();
    std::cout << std::string("Updating Task Status Response: ") + responseStr << std::endl;
    try {
        json jResponseStr = json::parse(responseStr);
        if(jResponseStr["error"] == true) {
            std::cout << "There occured an error when updating the task status!" << std::endl;
            return false;
        }
        else {
            std::cout << "Updated task status." << std::endl;
        }
    } catch (...) {
        std::cout << "Error when updating task status!" << std::endl;
        return false;
    }
    //std::string output = response.extract_string().get();
    return true;
}

//passed object id can be the one of a he service or the oid of a data source (encrypted device)
string vicinity_handler::getPublicKey(string oid) {
    using json = nlohmann::json; // for convenience
    //get corresponding he service oid
    std::string heOid = m_pController->getDB_access()->getPrivacyService(oid.c_str());
    std::string key = m_pController->getDB_access()->get_public_key(heOid.c_str());
    if(key == "") { //if key empty -> key of he service is not in database yet
        //request public key from heOid
        std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + heOid + "/properties/publickey";
        http_client client(address.c_str());
        http_request request(methods::GET);
        request.headers().add("adapter-id", adapterId);
        request.headers().add("infrastructure-id", ownOid);
        http_response response = client.request(request).get();
        std::string output = response.extract_string().get();
        //std::cout << "output: " + output << std::endl;
        std::string receivedKey = "";
        try {
            json jOutput = json::parse(output);
            if(jOutput["error"].get<bool>() == 0) {
                if(jOutput["message"][0].find("publickey") != jOutput["message"][0].end()) {
                    receivedKey = jOutput["message"][0]["publickey"];
                } else {
                    std::cout << "Error with received public key." << std::endl;
                }
            }
        } catch (...) {
            std::cout << "Error with payload when getting public key!" << std::endl;
        }
        //put key in database
        m_pController->getDB_access()->insert_public_key(heOid.c_str(), receivedKey.c_str()); //insert retrieved key into database
        //return new key
        std::cout << "Requested key and returned it" << std::endl;
        //std::cout << "Received Key: " + receivedKey << std::endl;
        return receivedKey;
    } else {
        return key;
    }
}

//returns task status; payload  must contain properties, participants, devices
string vicinity_handler::askToParticipateInAggregation(string destinationOid, string payload) {
    using json = nlohmann::json; // for convenience
    std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + destinationOid + "/actions/participateInAggregation";
    http_client client(address.c_str());
    http_request request(methods::POST);
    request.headers().add("adapter-id", adapterId);
    request.headers().add("infrastructure-id", ownOid);
    request.set_body(payload);
    http_response response = client.request(request).get();
    std::string output = response.extract_string().get();
    //std::cout << "output: " + output << std::endl;
    std::string taskId = "";
    try {
        json jOutput = json::parse(output);
        if(jOutput["error"].get<bool>() == 0) {
            taskId = jOutput["message"][0]["taskId"];
        }
    } catch (...) {
        std::cout << "Error with return payload when asking to participate in aggregation!" << std::endl;
    }
    return taskId;
}

//returns status
string vicinity_handler::getStatusOfAction(string destinationOid, string action, string taskId) {
    using json = nlohmann::json; // for convenience
    std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + destinationOid + "/actions/" + action + "/tasks/" + taskId;
    
    //std::cout << "URL: " + address << std::endl;
    http_client client(address.c_str());
    http_request request(methods::GET);
    request.headers().add("adapter-id", adapterId);
    request.headers().add("infrastructure-id", ownOid);
    http_response response = client.request(request).get();
    std::string output = response.extract_string().get();
    //std::cout << "Output: " + output << std::endl;
    std::string status = "";
    try {
        json jOutput = json::parse(output);
        if(jOutput["error"].get<bool>() == 0) {
            status = jOutput["message"][0]["status"];
            //std::cout << TaskId + std::string(" ") + status << std::endl;
        }
    } catch (...) {
        std::cout << "Error with payload when getting status of an action!" << std::endl;
    }
    return status;
}

//returns return value
string vicinity_handler::getReturnValueOfAction(string destinationOid, string action, string taskId) {
    using json = nlohmann::json; // for convenience
    std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + destinationOid + "/actions/" + action + "/tasks/" + taskId;
    
    http_client client(address.c_str());
    http_request request(methods::GET);
    request.headers().add("adapter-id", adapterId);
    request.headers().add("infrastructure-id", ownOid);
    //request.set_body(payload);
    http_response response = client.request(request).get();
    std::string output = response.extract_string().get();
    //std::cout << "output: " + output << std::endl;
    std::string returnValue = "";
    try {
        json jOutput = json::parse(output);
        if(jOutput["error"].get<bool>() == 0) {
            returnValue = jOutput["message"][0]["returnValue"];
        }
    } catch (...) {
        std::cout << "Error when getting return value of an action!" << std::endl;
        return std::string("");
    }
    return returnValue;
}

//send random share value to destination oid; returns taskId
string vicinity_handler::sendRandomShare(string destinationOid, string initiatorOid, int randomShare) {
    std::cout << std::string("Send Random Share Destination: ") + destinationOid << std::endl;
    
    using json = nlohmann::json; // for convenience
    std::string address = std::string("http://127.0.0.1:") + agentPort + std::string("/agent/remote/objects/") + destinationOid + std::string("/actions/randomshare");
    //std::cout << "URL: " + address << std::endl;
    http_client client(address.c_str());
    http_request request(methods::POST);
    request.headers().add("adapter-id", adapterId);
    request.headers().add("infrastructure-id", ownOid);
    request.set_body(std::string("{\"initiator\":\"") + initiatorOid + std::string("\",\"share\":") + std::to_string(randomShare) + std::string("}"));
    http_response response = client.request(request).get();
    std::string output = response.extract_string().get();
    std::cout << "OUTPUT: " + output << std::endl;
    try {
        json jOutput = json::parse(output);
        if(jOutput["error"].get<bool>() == 1) {
            std::cout << "There was an error, when sending a share to a trusted party!" << std::endl;
            return std::string("");
        }
        std::string taskId = jOutput["message"][0]["taskId"];     
        std::cout << "Send Random Share TaskID: " + taskId << std::endl;
        return taskId;
    } catch (...) {
        std::cout << "Error when sending a share to a trusted party!" << std::endl;
        return std::string("");
    }
}

//needed for distributed aggregation -> get plain property of encrypted device
long vicinity_handler::getPropertyOfEncryptedDevice(string encryptedDeviceOid, string pid) {    
    using json = nlohmann::json; // for convenience
    std::string plainDeviceOid = m_pController->getDB_access()->getPlainDeviceOfOwnEncryptedDevice(encryptedDeviceOid.c_str());
    std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + plainDeviceOid + "/properties/" + pid;
    std::cout << "address: " + address << std::endl;
    http_client client(address.c_str());
    http_request request(methods::GET);
    request.headers().add("adapter-id", adapterId);
    request.headers().add("infrastructure-id", ownOid);
    http_response response = client.request(request).get();
    std::string output = response.extract_string().get();
    //std::cout << "output: " + output << std::endl;
    long returnValue;
    try {
        json jOutput = json::parse(output);
        if(jOutput["error"].get<bool>() == 0) {
            returnValue = atoi(std::string(jOutput["message"][0]["value"].dump()).c_str());
        }
    } catch (...) {
        std::cout << "Error, received payload is wrong" << std::endl; 
    }
    return returnValue;
}

//deltes a task, returns true if successful, else false
bool vicinity_handler::deleteTask(string destinationOid, string action, string taskId) {
    using json = nlohmann::json; // for convenience
    std::string address = std::string("http://127.0.0.1:") + agentPort + std::string("/agent/remote/objects/") + destinationOid + std::string("/actions/") + action + std::string("/tasks/") + taskId;
    //std::cout << "URL: " + address << std::endl;
    http_client client(address.c_str());
    http_request request(methods::DEL);
    request.headers().add("adapter-id", adapterId);
    request.headers().add("infrastructure-id", ownOid);
    http_response response = client.request(request).get();
    std::string output = response.extract_string().get();
    try {
        json jOutput = json::parse(output);
        if(jOutput["error"].get<bool>() == 0) {
            return true;
        }
    } catch (...) {
    }
    return false;
}

//Source: https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
string vicinity_handler::replaceAll(std::string str, const std::string from, const std::string to) {
    if(from.empty())
        return str;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}
