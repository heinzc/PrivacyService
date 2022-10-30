#include "vicinity_handler.h"
#include "rest_handler.h"
#include "he_handler.h"


#include <sstream>
#include <utility>
#include <regex>

//#include <unistd.h> //usleep

#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>



using namespace std;
//using namespace utility;
//using namespace web::http;
//using namespace web::http::client;
//using namespace concurrency::streams;


vicinity_handler::vicinity_handler() :
    QObject()
{
    //ctor
    m_configFile = QJsonObject();

    m_ownServiceOid = QString();
    m_agentPort = 0;
    m_ownAdapterId = QString();
}

vicinity_handler::~vicinity_handler()
{
    //dtor
}

/**
 * @brief Initialize the handler
 * @param configfile to fetch Config Infos from
*/
void vicinity_handler::initialize(QString configfile) {
    // read the AdapterConfig
    QFile file(configfile);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray bytes = file.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError);
        if (jsonError.error != QJsonParseError::NoError)
        {
            qDebug() << "fromJson failed: " << jsonError.errorString();
            return;
        }
        if (document.isObject())
        {
            m_configFile = document.object();
        }
    }

    // parse array of all adapters and store them in a map
    for (auto adapter : m_configFile.value("adapters").toArray()) {
        // query the endpoint for /objects
        QString adapterid = adapter.toObject().value("adapter-id").toString();
        QString endpoint = adapter.toObject().value("endpoint").toString();

        m_endpoints.insert(adapterid, endpoint);
    }

    m_agentPort = 9997;
    //initialize random seed with current time
    srand (time(NULL));

    generateThingDescription();
}

/**
 * @brief Read the Service Thing Description of this Service. Iterate over all Adapters and add their Objects as well.
 * @return final Thing Description of this Privacy Service
*/
void vicinity_handler::generateThingDescription() {
    //load Service TD with endpoints for VAS
    QFile file("ServiceThingDescription.json");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray bytes = file.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError);
        if (jsonError.error != QJsonParseError::NoError)
        {
            qDebug() << "fromJson failed: " << jsonError.errorString();
            return;
        }
        if (document.isObject())
        {
            m_ownTD = document.object();

            m_ownServiceOid = m_ownTD.value("thing-descriptions").toArray().at(0).toObject().value("oid").toString();
            m_ownAdapterId = m_ownTD.value("adapter-id").toString();
        }
    }


    // iterate over all adapters in config file and request their TD
    for (auto adapter : m_configFile.value("adapters").toArray()) {
        // query the endpoint for /objects
        QString endpoint = adapter.toObject().value("endpoint").toString();

        // send request to adapters /objects
        m_pController->getREST_handler()->get(QUrl(endpoint + "/objects"), this);
    }
}

/**
 * @brief This Slot is called, when a reply from one of the requested adapters is returned
 * @param reply returned by the adapter
*/
void vicinity_handler::getAdapterTDFinished(QNetworkReply* reply) {
    // read the reply. has to be JSON, so parse it
    QJsonDocument replyJson = QJsonDocument::fromJson(reply->readAll());
    qDebug() << "reply in slot method: " << replyJson;

    // if the reply could not be parse, skip to next adapter
    if (replyJson.isNull()) {
        qDebug() << "error parsing adapter reply";
    }

    // read the original TD of the adapter
    QJsonObject rootNode = replyJson.object();

    QString adapterid = rootNode.value("adapter-id").toString();
    QJsonArray adapterObjects = rootNode.value("thing-descriptions").toArray();

    // load the current TD, so we can add things
    QJsonArray currentTD = m_ownTD.value("thing-descriptions").toArray();

    // go through all objects in the foreign adapters TD, rename them and add to our own TD
    for (auto td : adapterObjects) {
        QJsonObject tdNode = td.toObject();

        tdNode.insert("oid", adapterid + ":" + tdNode.value("oid").toString() + "_enc");
        tdNode.insert("name", tdNode.value("name").toString() + " (encrypted)");

        // remove write links.
        /*
        for (auto& prop : td["properties"]) {
            prop["read_link"]["href"] = std::string("/objects/{oid}/properties/{pid}");
            try {
                prop.erase("write_link");
            }
            catch (const std::exception& e) {
                cout << "info: remove write link failed" << endl;
            }
        }
        */

        // add the edited Object to our Adapters TD
        currentTD.append(tdNode);
    }
    // once all objects are parsed, replace the old TD with our new one
    m_ownTD["thing-descriptions"] = currentTD;

    qDebug() << "reply parsed. " << adapterObjects.size() << " object(s) added.";
}


QJsonObject vicinity_handler::getThingDescription() {
    return m_ownTD;
}


QJsonObject vicinity_handler::readProperty(const QString& oid, const QString& pid) {
    qDebug() << "Read Property; Oid: " + oid + ", Pid: " + pid;
    QJsonObject obj; // return object

    //read property of the service itself
    if(oid == m_ownServiceOid) {
        if(pid == "publickey") {
            qDebug() << "Publickey requested and returned";
            obj.insert("publickey", m_pController->getHE_handler()->getPublicKey());
            return obj;
        }
        else if(pid == "relinkeys") {
            qDebug() << "Reliniarization Keys requested and returned";
            QString relinKeys;
            m_pController->getHE_handler()->getRelinKeys(relinKeys);
            obj.insert("relinkeys", relinKeys);
            return obj;
        }
        else if (pid == "galoiskeys") {
            qDebug() << "Galois Keys requested and returned";
            QString galoisKeys;
            m_pController->getHE_handler()->getGaloisKeys(galoisKeys);
            obj.insert("galoiskeys", galoisKeys);
            qDebug() << "json object size:" << obj.size();
            QJsonDocument doc(obj);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            qDebug() << "json as string size: " << strJson.size();
            return obj;
        }
        else if(pid == "trustedparties") {
            std::cout << "Trusted parties requested and returned" << std::endl;
            //std::vector<std::string> trustedParties = m_pController->getDB_access()->getTrustedParties();
            //json parties = json::parse("{\"parties\":[]}");
            //for(std::string p : trustedParties) {
            //    parties["parties"].push_back(p);
            //    std::cout << p << '\n';
            //}
            return obj;
        }
    }

    // read property of attached adapter
    // oid is compount of <adapterid>:<objectid>
    QString adapterid, objectid;
    QRegularExpression rx("(.+):(.+)_enc");
    QRegularExpressionMatch rx_match = rx.match(oid);

    if (rx_match.hasMatch()) {
        QStringList matches = rx_match.capturedTexts(); // first entry is complete string
        if (matches.size() == 3) {
            adapterid = matches.at(1);
            objectid = matches.at(2);
        }
        else {
            obj.insert("error", "invalid oid");
            return obj;
        }
    }
    else {
        obj.insert("error", "invalid oid");
        return obj;
    }

    // at this point adapterid and objectid are valid
    // retrieve cleartext data

    if (m_endpoints.contains(adapterid)) {
        QString adapterendpoint = m_endpoints.value(adapterid);

        // send request to adapters /objects
        QJsonDocument replyJson = m_pController->getREST_handler()->get_blocking(QUrl(adapterendpoint + "/objects/" + objectid + "/properties/" + pid));

        // if the reply could not be parse, skip to next adapter
        if (replyJson.isNull()) {
            obj.insert("error", "error retrieving original property value");
            return obj;
        }

        double cleartextValue = replyJson.object().value("value").toDouble();
        QString encryptedValue = m_pController->getHE_handler()->encrypt_as_QString(cleartextValue);

        obj.insert("encrypted", "True");
        obj.insert("value", encryptedValue);

        return obj;
    }

    return obj;
}
//
////when an action request is received, this redirects it to the correct function
//void vicinity_handler::postAction(string oid, string aid, string payload, string sourceOid) {
//    //currently, there are only actions for this he service
//    if(oid == m_ownOid) { //he service
//        //which action?
//        if(aid == "decryptaction") {
//            m_pController->getVICINITY_handler()->decrypt(oid, sourceOid, payload);
//        }
//        else if(aid == "randomshareaction") {
//            m_pController->getVICINITY_handler()->sendShareAction(oid, sourceOid, payload);
//        }
//        else if(aid == "aggregationaction") {
//            m_pController->getVICINITY_handler()->startAggregation(oid, sourceOid, payload);
//        }
//        else if(aid == "participateinaggregationaction") {
//            m_pController->getVICINITY_handler()->participateInAggregation(oid, sourceOid, payload);
//        }
//        return;
//    }
//}


QJsonObject vicinity_handler::writeProperty(const QString& oid, const QString& pid, const QJsonDocument& payload) {
    qDebug() << "Write Property; Oid: " + oid + ", Pid: " + pid;
    QJsonObject jsonBody = payload.object();

    QJsonObject obj; // return object

    if(oid == m_ownServiceOid) { //he service
        if (pid == "recrypt") {
            // decrypt payload data cleartext data
            QString recrypted = m_pController->getHE_handler()->recrypt(jsonBody.value("ciphertext").toString());

            obj.insert("value", recrypted);

            return obj;
        }
        else if (pid == "recrypt_svm") {
            m_pController->getHE_handler()->recrypt_for_svm(jsonBody.value("ciphertext").toString(), jsonBody.value("dimension").toInt(), obj);

            return obj;
        }
        else if (pid == "hasaccess") {
            // get requester id from payload
            QString requesterId = jsonBody.value("requester-id").toString();
            QString destinationOid = jsonBody.value("destination-id").toString();

            //ask db, if requester has access and send result back
            bool access_granted = m_pController->getDB_access()->hasAccessToData(requesterId, destinationOid);

            obj.insert("granted", access_granted);

            return obj;
        }

        obj.insert("error", true);
        obj.insert("error-msg", "matching Property not found");

        return obj;
    }

    // read property of attached adapter
    // oid is compount of <adapterid>:<objectid>
    QString adapterid, objectid;
    QRegularExpression rx("(.+):(.+)_enc");
    QRegularExpressionMatch rx_match = rx.match(oid);

    if (rx_match.hasMatch()) {
        QStringList matches = rx_match.capturedTexts(); // first entry is complete string
        if (matches.size() == 3) {
            adapterid = matches.at(1);
            objectid = matches.at(2);
        }
        else {
            obj.insert("error", "invalid oid");
            return obj;
        }
    }
    else {
        obj.insert("error", "invalid oid");
        return obj;
    }

    // at this point adapterid and objectid are valid
    if (m_endpoints.contains(adapterid)) {
        // decrypt payload data cleartext data
        double cleartextValue = m_pController->getHE_handler()->decrypt(jsonBody.value("value").toString());

        jsonBody["value"] = cleartextValue;

        // we know the endpoint and where to send the data. extract and decrypt it before sending
        QString adapterendpoint = m_endpoints.value(adapterid);

        // send request to adapters /objects
        QJsonDocument replyJson = m_pController->getREST_handler()->put_blocking(QUrl(adapterendpoint + "/objects/" + objectid + "/properties/" + pid), QJsonDocument(jsonBody));

        // if the reply could not be parse, skip to next adapter
        if (replyJson.isNull()) {
            obj.insert("error", "error retrieving original property value");
            return obj;
        }

        obj.insert("adapter-reply", replyJson.object());
        return obj;
    }

    return obj;
}


QString vicinity_handler::getOwnOid() {
    return m_ownServiceOid;
}

QString vicinity_handler::getAdapterId() {
    return m_ownAdapterId;
}

uint8_t vicinity_handler::getAgentPort() {
    return m_agentPort;
}

uint8_t vicinity_handler::getOwnPort() {
    return 4242;
}
//
//void vicinity_handler::decrypt(string oid, string sourceOid, string payload) {
//    using json = nlohmann::json; // for convenience
//    std::cout << "Decrypt start" << std::endl;
//    if(m_ownOid == oid) { //is the action for this service?
//        if(m_pController->getDB_access()->hasAccessToDecrypt(sourceOid.c_str())) { //is own device or we permitted decryption
//            //everything ok, let's get value and decrypt it
//            json content;
//            std::string value;
//            try {
//                content = json::parse(payload);
//                value = string(content["value"]);
//            } catch (...) {
//                std::cout << "Error in input payload of decrypt action!" << std::endl;
//                updateTaskStatus(string("decrypt"), string("failed"), string("{\"value\":0}"));
//                return;
//            }
//            //std::cout << "Value to decrypt: " + value << std::endl;
//            try {
//                int decryptedValue = m_pController->getHE_handler()->decrypt(QString::fromStdString(value));
//                std::cout << "Decrypted value: " + std::to_string(decryptedValue) << std::endl;
//                //std::cout << "Payload update: " + string("{\"value\": " + std::to_string(decryptedValue) + "}") << std::endl;
//                updateTaskStatus(string("decrypt"), string("finished"), string("{\"value\": " + std::to_string(decryptedValue) + "}"));
//                return;
//            }
//            catch(...) {
//                std::cout << "Error while decrypting!" << std::endl;
//            }
//        }
//    }
//    //update status
//    updateTaskStatus(string("decrypt"), string("failed"), string("{\"value\":0}"));
//    std::cout << "Decrypt end" << std::endl;
//    return;
//}
//
//void vicinity_handler::startAggregation(string oid, string sourceOid, string payload) {
//    using json = nlohmann::json; //for convenience
//
//    m_pController->getDB_access()->resetBlindedMeasurements();
//
//    if(m_ownOid == oid) { //is the action for this service?
//        if(m_pController->getDB_access()->isOwnDevice(sourceOid.c_str())) { //only has to be called from owner devices (check sourceOid)
//            //get participant he services, devices and properties
//            std::cout << "Start Aggregation Payload: " + payload << std::endl;
//            json jPayload;
//            try {
//                jPayload = json::parse(payload);
//            } catch (...) {
//                std::cout << "Error in input payload of start aggregation action!" << std::endl;
//                updateTaskStatus(std::string("aggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                return;
//            }
//            if(jPayload.find("devices") != jPayload.end() && jPayload.find("properties") != jPayload.end() && jPayload.find("participants") != jPayload.end()) {
//                auto devices = jPayload["devices"].get<std::vector<std::string>>();
//                auto properties = jPayload["properties"].get<std::vector<std::string>>();
//                auto participants = jPayload["participants"].get<std::vector<std::string>>();
//                //has every vector the same length?
//                if(static_cast<int>(devices.size()) == static_cast<int>(properties.size()) && static_cast<int>(properties.size()) == static_cast<int>(participants.size())) {
//                    //are there at least three different he services? minimum for safe aggregation
//                    std::set<std::string> participantsSet;
//                    for (auto i : participants)
//                    {
//                        participantsSet.insert(i);
//                    }
//                    if(participantsSet.size() >= 3) {
//                        std::cout << "There are at least 3 participants" << std::endl;
//                        std::vector<std::string> taskIds;
//                        std::vector<std::string> taskExecutors;
//                        //insert all in database so we can receive the blinded measurements later
//                        for (auto &p : participantsSet) {
//                            m_pController->getDB_access()->insertParticipantBlindedMeasurements(p.c_str());
//                            //call aggregation action of participant (every participant once)
//                            //oid of destination has to be replaced with "you", makes setup easier (no need to save the oid manually)
//                            std::cout << "Ask participant to participate in aggregation." << std::endl;
//                            std::string taskId = askToParticipateInAggregation(p, replaceAll(payload, p, std::string("you")));
//                            std::cout << std::string("Task ID: ") + taskId << std::endl;
//                            taskIds.push_back(taskId);
//                            taskExecutors.push_back(p);
//                        }
//                        //wait until all blined measurements were received. meanwhile, if any task failed -> return; timeout of 60 seconds
//                        time_t startWaitingForTasks = time(NULL); //current time
//                        boost::chrono::system_clock::time_point startWaitingForTasks2 = boost::chrono::system_clock::now();
//                        int pollCount = 0;
//                        while(static_cast<int>(taskIds.size()) > 0 && time(NULL) < startWaitingForTasks + 60) { //or while !allBlindedMeasurementsReceived()
//                            //taskIds and taskExecutor (should) have the same size, however, we better check it
//                            if(static_cast<int>(taskIds.size()) == static_cast<int>(taskExecutors.size()) && static_cast<int>(taskIds.size()) > 0) {
//                                for(int i = 0; i < static_cast<int>(taskIds.size()); i++) {
//                                    //get status of task i
//                                    pollCount++;
//                                    std::string status = getStatusOfAction(taskExecutors.at(i), std::string("participateInAggregation"), taskIds.at(i));
//                                    //std::cout << "Status of a task: " + status << std::endl;
//                                    if(status == "failed") { //then, this task (aggregation itself) failed also
//                                        std::cout << "Task of a participant failed." << std::endl;
//                                        updateTaskStatus(std::string("aggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                                        return;
//                                    }
//                                    else if(status == "finished") {
//                                        //get result
//                                        std::cout << "One of the participateInAggregation tasks is finished." << std::endl;
//                                        std::string recPayload = getReturnValueOfAction(taskExecutors.at(i), std::string("participateInAggregation"), taskIds.at(i));
//                                        json jRecPayload = json::parse(recPayload);
//
//                                        //std::cout << std::string("Received payload: ") + recPayload << std::endl;
//                                        std::cout << std::string("Received value: ") + jRecPayload["value"].dump() << std::endl;
//                                        int blindedMeasurement = atoi(std::string(jRecPayload["value"].dump()).c_str());
//                                        m_pController->getDB_access()->updateBlindedMeasurement(taskExecutors.at(i).c_str(), blindedMeasurement);
//                                        taskIds.erase(taskIds.begin()+i);
//                                        taskExecutors.erase(taskExecutors.begin()+i);
//                                        break;
//                                    }
//                                }
//                            }
//                            else {
//                                //should never be here, so update task status to failed, just in case
//                                std::cout << "StartAggregation failed! Should not reach this!" << std::endl;
//                                updateTaskStatus(std::string("aggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                                return;
//                            }
//                            //sleep(1); //1 second, lower in real world usage
//                            //usleep(200 * 1000); //200 miliseconds
//                        }
//                        boost::chrono::milliseconds waitingTime = boost::chrono::duration_cast<boost::chrono::milliseconds> (boost::chrono::system_clock::now() - startWaitingForTasks2);
//                        std::cout << "Polling messages: " + std::to_string(pollCount) << std::endl;
//                        std::cout << "Waiting time for shares: " << waitingTime.count() << " ms" << std::endl;
//                        if(m_pController->getDB_access()->allBlindedMeasurementsReceived()) { //all shares should have been received, but better check it
//                            std::cout << "All blinded measurements received." << std::endl;
//                            //add blinded measurements -> calculate result
//                            int aggregationResult = m_pController->getDB_access()->getBlindedMeasurementSum();
//                            //update status to finished and return result
//                            std::cout << "Aggregation result: " + std::to_string(aggregationResult) << std::endl;
//                            updateTaskStatus(std::string("aggregation"), std::string("finished"), std::string("{\"value\":") + std::to_string(aggregationResult) + std::string("}"));
//                            return;
//                        }
//                    }
//                }
//            }
//        }
//    }
//    //update status failed
//    std::cout << "StartAggregation failed!" << std::endl;
//    updateTaskStatus(std::string("aggregation"), std::string("failed"), std::string("{\"value\":0}"));
//    return;
//}
//
//void vicinity_handler::participateInAggregation(string oid, string sourceOid, string payload) {
//    using json = nlohmann::json; //for convenience
//
//    //following actually not needed, since they are deleted at the end. just to be safe.
//    m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
//
//    std::cout << "//////////////////PIA//////////////////" << std::endl;
//    std::cout << "Aggregation action payload: " + payload << std::endl;
//    if(m_ownOid == oid) { //is the action for this service?
//        if(m_pController->getDB_access()->isTrustedInitiator(sourceOid.c_str()) | m_pController->getDB_access()->isOwnDevice(sourceOid.c_str())) { //is initiator allowed to initiate aggregation? (do we trust?)
//            //get participant he services, devices and properties
//            json jPayload;
//            try {
//                jPayload = json::parse(payload);
//            } catch (...) {
//                std::cout << "Error in input payload of participate in aggregation action!" << std::endl;
//                updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                return;
//            }
//            if(jPayload.find("devices") != jPayload.end() && jPayload.find("properties") != jPayload.end() && jPayload.find("participants") != jPayload.end()) {
//                auto devices = jPayload["devices"].get<std::vector<std::string>>();
//                auto properties = jPayload["properties"].get<std::vector<std::string>>();
//                auto participants = jPayload["participants"].get<std::vector<std::string>>();
//                //have devices, properties and participants the same length?
//                if(static_cast<int>(devices.size()) == static_cast<int>(properties.size()) && static_cast<int>(properties.size()) == static_cast<int>(participants.size())) {
//                    //are there at least three different participants? minimum for safe aggregation
//                    std::set<std::string> participantsSet;
//                    for (auto i : participants)
//                    {
//                        participantsSet.insert(i);
//                    }
//                    if(participantsSet.size() >= 3) {
//                        std::cout << "There are at least 3 participants" << std::endl;
//                        //determine, which participants are part of my trusted parties and which trust me
//                        std::set<std::string> participatingPartiesWhoTrustMeSet;
//                        std::set<std::string> participatingTrustedPartiesSet;
//                        for(auto i : participantsSet) {
//                            if(i != "you") { //cannot be myself
//                                if(m_pController->getDB_access()->trustsMe(i.c_str())) { //only if participant trusts me
//                                    participatingPartiesWhoTrustMeSet.insert(i);
//                                }
//                                if(m_pController->getDB_access()->isTrustedParty(i.c_str())) { //only if i trust this party
//                                    participatingTrustedPartiesSet.insert(i);
//                                }
//                            }
//                        }
//                        //is there at least one participant, who i trust?
//                        if(participatingTrustedPartiesSet.size() >= 1) {
//                            std::cout << "There is at least 1 trusted party" << std::endl;
//                            //am i really a part of this aggregation?
//                            if(participantsSet.find("you") != participantsSet.end()) {
//                                std::cout << "I am part of this aggregation." << std::endl;
//
//                                //get sum of requested own device properties
//                                long sumOfOwnDeviceProperties = 0;
//                                for(int i = 0; i < devices.size(); i++) {
//                                    if(participants.at(i) == "you") {
//                                        //are the as mine marked devices really mine?
//                                        if(m_pController->getDB_access()->isOwnDevice(devices.at(i).c_str())) {
//                                            std::cout << std::string("Get plain value of: ") + devices.at(i) + std::string(", ") + properties.at(i) << std::endl;
//                                            sumOfOwnDeviceProperties += getPropertyOfEncryptedDevice(devices.at(i), properties.at(i));
//                                        }
//                                        else {
//                                            std::cout << "One of the requested devices is not mine! Aggregation failed!" << std::endl;
//                                            m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
//                                            updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                                            return;
//                                        }
//                                    }
//                                }
//                                std::cout << "Sum of own device properties: " + std::to_string(sumOfOwnDeviceProperties) << std::endl;
//
//                                //insert participants who trust me in database (random shares)
//                                for(auto &tp : participatingPartiesWhoTrustMeSet) {
//                                    std::cout << "Insert participant who trusts me in database for aggregation." << std::endl;
//                                    m_pController->getDB_access()->insertParticipantRandomShares(sourceOid.c_str(), tp.c_str());
//                                }
//                                long sentSharesSum = 0;
//                                //send to all trusted participants a share and save their sum
//                                std::vector<std::string> taskIds;
//                                std::vector<std::string> taskExecutors;
//                                for(auto &tp : participatingTrustedPartiesSet) {
//                                    //generate share and add to all shares
//                                    long share = rand() % 100000; //random share value: here, 0-99999 possible
//                                    sentSharesSum += share;
//                                    //send share
//                                    std::cout << "Send random share to participating trusted party." << std::endl;
//                                    std::string taskId = "";
//                                    for(int i = 0; i < 3 && taskId == std::string(""); i++) { //try to send share max 3 times
//                                        taskId = sendRandomShare(tp, sourceOid, share);
//                                    }
//                                    if(taskId == std::string("")) {
//                                        std::cout << "Even after retries, was not able to send share. Task failed." << std::endl;
//                                        m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
//                                        updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                                        return;
//                                    }
//                                    std::cout << "Sent Random Share Task ID: " + taskId << std::endl;
//                                    //save task id and task executor
//                                    taskIds.push_back(taskId);
//                                    taskExecutors.push_back(tp);
//                                }
//
//                                //wait until all tasks were finished or at least one failed (the send share ones).
//                                time_t startWaitingForTasks = time(NULL); //current time
//                                //time_t startRunning = time(NULL); //current time
//
//
//                                //it is not necessary, to check if shares were successful. however,we cannot react, if they were not
//                                /*
//                                std::cout << "XXXBefore While loop" << std::endl;
//                                int pollCount = 0;
//                                while(static_cast<int>(taskIds.size()) > 0 && time(NULL) < startWaitingForTasks + 30) { // && time(NULL) < startRunning + 5) { //30 second total timeout
//                                    std::cout << "XXXWhile loop" << std::endl;
//                                    //taskIds and taskExecutor have the same size, however, we better check it
//                                    if(static_cast<int>(taskExecutors.size()) == static_cast<int>(taskExecutors.size())) {
//                                        //std::cout << "XXXSame size" << std::endl;
//                                        for(int i = 0; i < static_cast<int>(taskExecutors.size()); i++) {
//                                            std::cout << "XXXFor loop" << std::endl;
//                                            //get status of task i
//                                            pollCount++;
//                                            std::string status = getStatusOfAction(taskExecutors.at(i), std::string("randomshare"), taskIds.at(i));
//                                            //std::cout << "Task Status: " + status << std::endl;
//                                            if(status == "failed") { //then, this task (aggregation itself) failed also
//                                                std::cout << "Randomshare task of a trusted participant failed." << std::endl;
//                                                m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
//                                                updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                                                return;
//                                            }
//                                            else if(status == "finished") {
//                                                std::cout << std::string("Randomshare task of a trusted participant is finished.") + taskIds.at(i) << std::endl;
//                                                taskIds.erase(taskIds.begin()+i);
//                                                taskExecutors.erase(taskExecutors.begin()+i);
//                                                break;
//                                            }
//
//                                            //it can happen, that updating task to "running" was received after "finished"
//                                            else if(status == "running") {
//                                                pollCount++;
//                                                std::string returnValue = getReturnValueOfAction(taskExecutors.at(i), std::string("randomshare"), taskIds.at(i));
//                                                json jReturnValue;
//                                                try {
//                                                    jReturnValue = json::parse(returnValue);
//                                                } catch (...) {
//                                                    std::cout << "Error when parsing return value in participate in aggregation." << std::endl;
//                                                    updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                                                    return;
//                                                }
//                                                if(jReturnValue.find("declined") != jReturnValue.end()) { //this task was actually finished, since the payload is there. can happen if gateway receives messages out of order
//                                                    //delete task
//                                                    deleteTask(taskExecutors.at(i), std::string("randomshare"), taskIds.at(i));
//                                                    if(jReturnValue["declined"].get<bool>() == 0) {
//                                                        //remove this task
//                                                        taskIds.erase(taskIds.begin()+i);
//                                                        taskExecutors.erase(taskExecutors.begin()+i);
//                                                        break;
//                                                    }
//                                                    else {
//                                                        std::cout << "Randomshare task of a trusted participant failed. (Wrong update as well)" << std::endl;
//                                                        m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
//                                                        updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                                                        return;
//                                                    }
//                                                }
//                                            }
//                                        }
//                                    }
//                                    usleep(200 * 1000); //200 miliseconds
//                                }
//                                std::cout << "Polling messages: " + std::to_string(pollCount) << std::endl;
//                                if(static_cast<int>(taskIds.size()) > 0) {
//                                    std::cout << "Timeout when waiting for our random share tasks to be finished. Stop." << std::endl;
//                                    //Delete tasks, that are still running (the ones in the vectors)
//                                    if(static_cast<int>(taskExecutors.size()) == static_cast<int>(taskExecutors.size())) {
//                                        for(int i = 0; i < static_cast<int>(taskExecutors.size()); i++) {
//                                            deleteTask(taskExecutors.at(i), std::string("randomshare"), taskIds.at(i));
//                                        }
//                                    }
//                                    m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
//                                    updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
//                                    return;
//                                }
//                                */
//
//                                time_t start = time(NULL); //current time
//                                std::cout << "Waiting for shares..." << std::endl;
//                                while(!m_pController->getDB_access()->allRandomSharesReceived(sourceOid.c_str()) && time(NULL) < start + 10) {
//                                    //std::cout << "Waiting for shares..." << std::endl;
//                                    //sleep(1);
//                                    //usleep(50 * 1000); //50 miliseconds
//                                }
//                                //all shares were received? Now, we either received all, or we do not want to wait any longer for them
//                                if(m_pController->getDB_access()->allRandomSharesReceived(sourceOid.c_str())) {
//                                    std::cout << "All random shares received." << std::endl;
//                                    //calculate blinded measurement
//                                    int receivedShares = m_pController->getDB_access()->getRandomShareSum(sourceOid.c_str());
//                                    long blindedMeasurement = sentSharesSum - receivedShares + sumOfOwnDeviceProperties;
//                                    //return result
//                                    m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
//                                    std::cout << "Participate In Aggregation Output: " + std::to_string(blindedMeasurement) << std::endl;
//                                    updateTaskStatus(std::string("participateInAggregation"), std::string("finished"), std::string("{\"value\":") + std::to_string(blindedMeasurement) + std::string("}"));
//                                    return;
//                                } else {
//                                    std::cout << "Timeout when waiting for shares." << std::endl;
//                                }
//                            }
//                        } std::cout << "There is no trusted party participating." << std::endl;
//                    }
//                }
//            }
//        }
//    }
//    std::cout << "Participate in Aggregation task failed." << std::endl;
//    m_pController->getDB_access()->deleteInitiatorRandomShares(sourceOid.c_str()); //reset db entries of this initiator
//    updateTaskStatus(std::string("participateInAggregation"), std::string("failed"), std::string("{\"value\":0}"));
//    return;
//}
//
//void vicinity_handler::sendShareAction(string oid, string sourceOid, string payload) {
//    std::cout << "Randomshare action called, try to insert share in database." << std::endl;
//    using json = nlohmann::json; // for convenience
//    if(m_ownOid == oid) { //is the action for this service?
//        if(m_pController->getDB_access()->trustsMe(sourceOid.c_str()) | m_pController->getDB_access()->isOwnDevice(sourceOid.c_str())) { //does this service trust me?
//            //get share value and aggregation origin
//            json jPayload;
//            string initiator;
//            int share;
//            try {
//                jPayload = json::parse(payload);
//                initiator = jPayload["initiator"];
//                std::cout << "share: " + jPayload["share"].dump() << std::endl;
//                share = atoi(std::string(jPayload["share"].dump()).c_str());
//            } catch (...) {
//                std::cout << "Error in input payload of send share action!" << std::endl;
//                updateTaskStatus(string("randomshare"), string("failed"), string("{\"declined\":true}"));
//                return;
//            }
//            //is this participant inserted? did we even receive the request to aggregate yet?
//            time_t startWaiting = time(NULL); //current time
//            while(time(NULL) < startWaiting + 3) {
//                if(m_pController->getDB_access()->isAlreadyInsertedInRandomShares(initiator.c_str(), sourceOid.c_str())) {
//                    //insert data in database; uses UPDATE operator -> if sourceOid is no participant, nothing happens -> no need to check, if sourcOid is part of aggregation
//                    m_pController->getDB_access()->updateShareRandomShares(initiator.c_str(), sourceOid.c_str(), share);
//                    updateTaskStatus(string("randomshare"), string("finished"), string("{\"declined\":false}"));
//                    std::cout << "Randomshare task finished." << std::endl;
//                    return;
//                } else {
//                    //usleep(50 * 1000); //50 miliseconds sleep. maybe it takes some more time, until we receive a request to aggregate
//                }
//            }
//        }
//    }
//    std::cout << "Randomshare task failed." << std::endl;
//    updateTaskStatus(string("randomshare"), string("failed"), string("{\"declined\":true}"));
//    return;
//}
//
////takes action id, status (failed, running or finished)
//bool vicinity_handler::updateTaskStatus(string aid, string status, string payload) {
//    using json = nlohmann::json; // for convenience
//    //sleep(4);
//    std::string address = "http://127.0.0.1:" + agentPort + "/agent/actions/" + aid;
//    //std::cout << "Update Task Status Address: " + address << std::endl;
//    //http_client client(address.c_str());
//    ////client.request(methods::PUT, "", status);
//    //http_request request(methods::PUT);
//    ////request.headers().add("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
//    //request.headers().add("adapter-id", adapterId);
//    //request.headers().add("infrastructure-id", m_ownOid);
//    //request.headers().add("status", status.c_str());
//    //request.set_body(payload.c_str());
//    ////std::cout << request.to_string() << std::endl;
//    ////std::cout << request.absolute_uri().to_string() << std::endl;
//    //pplx::task<web::http::http_response> requestTask = client.request(request);
//    //web::http::http_response response = requestTask.get();
//    //std::string responseStr = response.extract_string().get();
//    std::string responseStr = "";
//    std::cout << std::string("Updating Task Status Response: ") + responseStr << std::endl;
//    try {
//        json jResponseStr = json::parse(responseStr);
//        if(jResponseStr["error"] == true) {
//            std::cout << "There occured an error when updating the task status!" << std::endl;
//            return false;
//        }
//        else {
//            std::cout << "Updated task status." << std::endl;
//        }
//    } catch (...) {
//        std::cout << "Error when updating task status!" << std::endl;
//        return false;
//    }
//    //std::string output = response.extract_string().get();
//    return true;
//}
//
//passed object id can be the one of a he service or the oid of a data source (encrypted device)
string vicinity_handler::getPublicKey(string oid)
{
    // get corresponding he service oid
    QString heOid = m_pController->getDB_access()->getPrivacyService(QString::fromStdString(oid));
    QString key = m_pController->getDB_access()->get_public_key(heOid);
    if (key.isEmpty())
    { // if key empty -> key of he service is not in database yet
        // request public key from heOid
        QString address = "http://127.0.0.1:" + QString::number(m_agentPort) + "/agent/remote/objects/" + heOid + "/properties/publickey";
        QList<QPair<QByteArray, QByteArray>> headers;
        headers.append(qMakePair<QByteArray, QByteArray>("adapter-id", m_ownAdapterId.toUtf8()));
        headers.append(qMakePair<QByteArray, QByteArray>("infrastructure-id", m_ownServiceOid.toUtf8()));
        QJsonDocument replyDoc = m_pController->getREST_handler()->get_blocking(address, headers);
        // std::cout << "output: " + output << std::endl;
        QString receivedKey = "";
        try
        {
            if (!replyDoc.object().contains("error"))
            {
                if (replyDoc.object().contains("publickey"))
                {
                    receivedKey = replyDoc.object().value("publickey").toString();
                }
                else
                {
                    std::cout << "Error with received public key." << std::endl;
                }
            }
        }
        catch (...)
        {
            std::cout << "Error with payload when getting public key!" << std::endl;
        }
        // put key in database
        m_pController->getDB_access()->insert_public_key(heOid, receivedKey); // insert retrieved key into database
        // return new key
        std::cout << "Requested key and returned it" << std::endl;
        // std::cout << "Received Key: " + receivedKey << std::endl;
        return receivedKey.toStdString();
    }
    else
    {
        return key.toStdString();
    }
}
//
////returns task status; payload  must contain properties, participants, devices
//string vicinity_handler::askToParticipateInAggregation(string destinationOid, string payload) {
//    using json = nlohmann::json; // for convenience
//    std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + destinationOid + "/actions/participateInAggregation";
//    //http_client client(address.c_str());
//    //http_request request(methods::POST);
//    //request.headers().add("adapter-id", adapterId);
//    //request.headers().add("infrastructure-id", m_ownOid);
//    //request.set_body(payload);
//    //http_response response = client.request(request).get();
//    //std::string output = response.extract_string().get();
//    std::string output = "";
//    //std::cout << "output: " + output << std::endl;
//    std::string taskId = "";
//    try {
//        json jOutput = json::parse(output);
//        if(jOutput["error"].get<bool>() == 0) {
//            taskId = jOutput["message"][0]["taskId"];
//        }
//    } catch (...) {
//        std::cout << "Error with return payload when asking to participate in aggregation!" << std::endl;
//    }
//    return taskId;
//}
//
////returns status
//string vicinity_handler::getStatusOfAction(string destinationOid, string action, string taskId) {
//    using json = nlohmann::json; // for convenience
//    std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + destinationOid + "/actions/" + action + "/tasks/" + taskId;
//
//    //std::cout << "URL: " + address << std::endl;
//    //http_client client(address.c_str());
//    //http_request request(methods::GET);
//    //request.headers().add("adapter-id", adapterId);
//    //request.headers().add("infrastructure-id", m_ownOid);
//    //http_response response = client.request(request).get();
//    //std::string output = response.extract_string().get();
//    std::string output = "";;
//    //std::cout << "Output: " + output << std::endl;
//    std::string status = "";
//    try {
//        json jOutput = json::parse(output);
//        if(jOutput["error"].get<bool>() == 0) {
//            status = jOutput["message"][0]["status"];
//            //std::cout << TaskId + std::string(" ") + status << std::endl;
//        }
//    } catch (...) {
//        std::cout << "Error with payload when getting status of an action!" << std::endl;
//    }
//    return status;
//}
//
////returns return value
//string vicinity_handler::getReturnValueOfAction(string destinationOid, string action, string taskId) {
//    using json = nlohmann::json; // for convenience
//    std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + destinationOid + "/actions/" + action + "/tasks/" + taskId;
//
//    //http_client client(address.c_str());
//    //http_request request(methods::GET);
//    //request.headers().add("adapter-id", adapterId);
//    //request.headers().add("infrastructure-id", m_ownOid);
//    ////request.set_body(payload);
//    //http_response response = client.request(request).get();
//    //std::string output = response.extract_string().get();
//    std::string output = "";
//    //std::cout << "output: " + output << std::endl;
//    std::string returnValue = "";
//    try {
//        json jOutput = json::parse(output);
//        if(jOutput["error"].get<bool>() == 0) {
//            returnValue = jOutput["message"][0]["returnValue"];
//        }
//    } catch (...) {
//        std::cout << "Error when getting return value of an action!" << std::endl;
//        return std::string("");
//    }
//    return returnValue;
//}
//
////send random share value to destination oid; returns taskId
//string vicinity_handler::sendRandomShare(string destinationOid, string initiatorOid, int randomShare) {
//    std::cout << std::string("Send Random Share Destination: ") + destinationOid << std::endl;
//
//    using json = nlohmann::json; // for convenience
//    std::string address = std::string("http://127.0.0.1:") + agentPort + std::string("/agent/remote/objects/") + destinationOid + std::string("/actions/randomshare");
//    //std::cout << "URL: " + address << std::endl;
//    //http_client client(address.c_str());
//    //http_request request(methods::POST);
//    //request.headers().add("adapter-id", adapterId);
//    //request.headers().add("infrastructure-id", m_ownOid);
//    //request.set_body(std::string("{\"initiator\":\"") + initiatorOid + std::string("\",\"share\":") + std::to_string(randomShare) + std::string("}"));
//    //http_response response = client.request(request).get();
//    //std::string output = response.extract_string().get();
//    std::string output = "";
//    std::cout << "OUTPUT: " + output << std::endl;
//    try {
//        json jOutput = json::parse(output);
//        if(jOutput["error"].get<bool>() == 1) {
//            std::cout << "There was an error, when sending a share to a trusted party!" << std::endl;
//            return std::string("");
//        }
//        std::string taskId = jOutput["message"][0]["taskId"];
//        std::cout << "Send Random Share TaskID: " + taskId << std::endl;
//        return taskId;
//    } catch (...) {
//        std::cout << "Error when sending a share to a trusted party!" << std::endl;
//        return std::string("");
//    }
//}
//
////needed for distributed aggregation -> get plain property of encrypted device
//long vicinity_handler::getPropertyOfEncryptedDevice(string encryptedDeviceOid, string pid) {
//    using json = nlohmann::json; // for convenience
//    std::string plainDeviceOid = m_pController->getDB_access()->getPlainDeviceOfOwnEncryptedDevice(encryptedDeviceOid.c_str());
//    std::string address = "http://127.0.0.1:" + agentPort + "/agent/remote/objects/" + plainDeviceOid + "/properties/" + pid;
//    std::cout << "address: " + address << std::endl;
//    //http_client client(address.c_str());
//    //http_request request(methods::GET);
//    //request.headers().add("adapter-id", adapterId);
//    //request.headers().add("infrastructure-id", m_ownOid);
//    //http_response response = client.request(request).get();
//    //std::string output = response.extract_string().get();
//    std::string output = "";
//    //std::cout << "output: " + output << std::endl;
//    long returnValue;
//    try {
//        json jOutput = json::parse(output);
//        if(jOutput["error"].get<bool>() == 0) {
//            returnValue = atoi(std::string(jOutput["message"][0]["value"].dump()).c_str());
//        }
//    } catch (...) {
//        std::cout << "Error, received payload is wrong" << std::endl;
//    }
//    return returnValue;
//}
//
////deltes a task, returns true if successful, else false
//bool vicinity_handler::deleteTask(string destinationOid, string action, string taskId) {
//    using json = nlohmann::json; // for convenience
//    std::string address = std::string("http://127.0.0.1:") + agentPort + std::string("/agent/remote/objects/") + destinationOid + std::string("/actions/") + action + std::string("/tasks/") + taskId;
//    //std::cout << "URL: " + address << std::endl;
//    //http_client client(address.c_str());
//    //http_request request(methods::DEL);
//    //request.headers().add("adapter-id", adapterId);
//    //request.headers().add("infrastructure-id", m_ownOid);
//    //http_response response = client.request(request).get();
//    //std::string output = response.extract_string().get();
//    std::string output = "";
//    try {
//        json jOutput = json::parse(output);
//        if(jOutput["error"].get<bool>() == 0) {
//            return true;
//        }
//    } catch (...) {
//    }
//    return false;
//}

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