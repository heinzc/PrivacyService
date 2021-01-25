#include "rest_handler.h"

//#include "FHE.h"
#include "he_handler.h"
#include "vicinity_handler.h"

#include <QNetworkRequest>



rest_handler::rest_handler() :
    QObject()
{
    //ctor
    m_pNetManager = new QNetworkAccessManager(nullptr);
    m_pHttpServer = new QHttpServer();
    m_pController = nullptr;

    this->setupRoutes();
}

rest_handler::~rest_handler()
{
    //dtor
    delete m_pNetManager;
    delete m_pHttpServer;
}

void rest_handler::setController(he_controller * controller) {
    m_pController = controller;
}

int rest_handler::setupRoutes() {
    m_pHttpServer->route("/vicinity/objects", QHttpServerRequest::Method::GET, [=](const QHttpServerRequest& request) {
        return handle_VICINITY_GET_objects(request);
        });

    m_pHttpServer->route("/vicinity/objects/<arg>/properties/<arg>", QHttpServerRequest::Method::GET, [=](QString oid, QString pid, const QHttpServerRequest& request) {
        return handle_VICINITY_GET_properties(oid, pid, request);
        });

    m_pHttpServer->route("/vicinity/objects/<arg>/properties/<arg>", QHttpServerRequest::Method::PUT, [=](QString oid, QString pid, const QHttpServerRequest& request) {
        return handle_VICINITY_PUT_properties(oid, pid, request);
        });

    m_pHttpServer->route("/vicinity/objects/<arg>/actions/<arg>", QHttpServerRequest::Method::PUT, [=](QString oid, QString aid, const QHttpServerRequest& request) {
        return handle_VICINITY_POST_action(oid, aid, request);
        });

    m_pHttpServer->route("/encrypt", QHttpServerRequest::Method::POST, [=](const QHttpServerRequest& request) {
        return handle_local_encrypt(request);
        });

    m_pHttpServer->route("/aggregate", QHttpServerRequest::Method::POST, [=](const QHttpServerRequest& request) {
        return handle_local_aggregate(request);
        });

    m_pHttpServer->route("/decrypt", QHttpServerRequest::Method::POST, [=](const QHttpServerRequest& request) {
        return handle_local_decrypt(request);
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

/**
 * @brief Blocking! Send get request to given Endpoint
 * @param endpoint the URL the request is sent to
 * @return the response
*/
void rest_handler::get(const QUrl& endpoint, QObject * caller) {
    // connect the reply to the slot
    connect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), caller, SLOT(getAdapterTDFinished(QNetworkReply*)));
    QNetworkRequest request(endpoint);

    m_pNetManager->get(request); 
}


QJsonDocument rest_handler::get_blocking(const QUrl& endpoint) {
    // disconnect first, so the reply buffer is not read by the slot
    disconnect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), nullptr, nullptr);

    QNetworkRequest req(endpoint);
    QScopedPointer<QNetworkReply> reply(m_pNetManager->get(req));

    QTime timeout = QTime::currentTime().addSecs(10);
    while (QTime::currentTime() < timeout && !reply->isFinished()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Failure" << reply->errorString();
    }

    QByteArray replyPayload = reply->readAll();

    return QJsonDocument::fromJson(replyPayload);
}


//// A POST request
////
//void rest_handler::handle_post(http_request message) {
//    std::cout << "POST!" << std::endl;
//    try{
//        else if(std::find(paths.begin(), paths.end(), "hasaccess") != paths.end()) { //DEPRECATED
//            string stvalue = message.extract_string().get(); //oid
//            if(m_pController->getDB_access()->hasAccessToDecrypt(stvalue.c_str())) {
//                message.reply(status_codes::OK, "true");
//            }
//            else {
//                message.reply(status_codes::OK, "false");
//            }
//        }
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
double rest_handler::extract_double_from_request(const QHttpServerRequest& request) {
    QString strValue;
    double value;

    // check content type and extract value accordingly
    QVariant contentType = request.headers().value("Content-Type");
    if (contentType.toString() == "application/json") {
        // read body as JSON
        QJsonDocument body_json = QJsonDocument::fromJson(request.body());

        // check if parsing succeeded
        if (body_json.isNull()) {
            throw std::invalid_argument("JSON parse error");
        }
        if (!body_json.isObject()) {
            throw std::invalid_argument("invalid JSON provided");
        }
        if (!body_json.object().contains("value")) {
            throw std::invalid_argument("JSON does not contain a value");
        }

        // at this point jsonValue contains the actual value
        QJsonValue jsonValue = body_json.object().value("value");
        // return a double value
        switch (jsonValue.type()) {
        case (QJsonValue::Double):
            return jsonValue.toDouble();
        case (QJsonValue::String):
            strValue = jsonValue.toString();
            bool convertOK;
            value = strValue.toDouble(&convertOK);
            if (!convertOK) {
                throw std::invalid_argument("JSON does not contain a value");
            }
            return value;
        default:
            throw std::invalid_argument("JSON does not contain a value");
        }
    }
    // if no Content-Type is given, assume text/plain
    else if (contentType.isNull() || contentType.toString() == "text/plain") {
        strValue = QTextCodec::codecForUtfText(request.body())->toUnicode(request.body());
        bool convertOK;
        value = strValue.toDouble(&convertOK);
        if (!convertOK) {
            throw std::invalid_argument("Request Body does not contain a value");
        }
        return value;
    }
    else {
        throw std::invalid_argument("no value provided");
    }
}


QString rest_handler::extract_ctxt_from_request(const QHttpServerRequest& request) {
    QString strValue;
    QString value;

    // check content type and extract value accordingly
    QVariant contentType = request.headers().value("Content-Type");
    if (contentType.toString() == "application/json") {
        // read body as JSON
        QJsonDocument body_json = QJsonDocument::fromJson(request.body());

        // check if parsing succeeded
        if (body_json.isNull()) {
            throw std::invalid_argument("JSON parse error");
        }
        if (!body_json.isObject()) {
            throw std::invalid_argument("invalid JSON provided");
        }
        if (!body_json.object().contains("value")) {
            throw std::invalid_argument("JSON does not contain a value");
        }

        // at this point jsonValue contains the actual ctxt provided
        QJsonValue jsonValue = body_json.object().value("value");
        // return a double value
        return jsonValue.toString();
    }
    else {
        throw std::invalid_argument("no value provided or invalid Content-Type");
    }
}


QJsonObject rest_handler::handle_VICINITY_GET_objects(const QHttpServerRequest& request) {
    std::cout << "VICINITY Get Objects!" << std::endl;
    //string td = m_pController->getVICINITY_handler()->generateThingDescription();
    //message.reply(status_codes::OK, td);
    return m_pController->getVICINITY_handler()->getThingDescription();
}


QJsonObject rest_handler::handle_VICINITY_GET_properties(QString oid, QString pid, const QHttpServerRequest& request) {
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
    qDebug() << "VICINITY Get Parameters!";
    qDebug() << oid << pid;

    return m_pController->getVICINITY_handler()->readProperty(oid, pid);
        //string payload = m_pController->getVICINITY_handler()->readProperty(oid, pid);
        //std::cout << "Read Property. Sending answer back." << std::endl;
        //message.reply(status_codes::OK, payload);
}


QJsonObject rest_handler::handle_VICINITY_POST_action(QString oid, QString aid, const QHttpServerRequest& request) {
    std::cout << "VICINITY Post Request!" << std::endl;
        //string sourceOid = message.absolute_uri().query();
        ////remove "sourceOid=" from sourceOid (request parameter)
        //string toRemove = "sourceOid=";
        //std::string::size_type i = sourceOid.find(toRemove);
        //if (i != std::string::npos) sourceOid.erase(i, toRemove.length());
        //string oid = path[2];
        //string aid = path.at(1);
        //string payload = message.extract_string().get(); //can only be extracted once!
        //message.reply(status_codes::OK, "{\"status\":\"running\"}");
        //m_pController->getVICINITY_handler()->postAction(oid, aid, payload, sourceOid);
    return QJsonObject();
}


QJsonObject rest_handler::handle_VICINITY_PUT_properties(QString oid, QString pid, const QHttpServerRequest& request) {
    std::cout << "VICINITY Put Request!" << std::endl;
    QByteArray payload = request.body();
            
    //string payload2 = m_pController->getVICINITY_handler()->writeProperty(oid, pid, payload);
    return QJsonObject();
}

QJsonObject rest_handler::handle_local_encrypt(const QHttpServerRequest& request) {
    QJsonObject result;
    double value;
    try {
        value = extract_double_from_request(request);
    }
    catch (std::invalid_argument& e) {
        result.insert("error", e.what());
        return result;
    }

    QString encrypted = m_pController->getHE_handler()->encrypt_as_QString(value);

    result.insert("value", encrypted);
    return result;
}

QJsonObject rest_handler::handle_local_aggregate(const QHttpServerRequest& request) {
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
    return QJsonObject();
}

QJsonObject rest_handler::handle_local_decrypt(const QHttpServerRequest& request) {
    QJsonObject result;
    QString value;
    try {
        value = extract_ctxt_from_request(request);
    }
    catch (std::invalid_argument& e) {
        result.insert("error", e.what());
        return result;
    }

    double decrypted = m_pController->getHE_handler()->decrypt(value);

    result.insert("value", decrypted);
    return result;
}

#include "moc_rest_handler.cpp"