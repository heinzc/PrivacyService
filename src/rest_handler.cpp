#include "rest_handler.h"

#include "he_handler.h"
#include "vicinity_handler.h"

#include <QCoreApplication>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QHttpServer>

#include <QStringConverter>
#include <QJsonArray>


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
    addRoute("/vicinity/objects", QHttpServerRequest::Method::Get, [=](const QHttpServerRequest& request, QHttpServerResponder &&responder) {
        return handle_VICINITY_GET_objects(request, responder);
        });

    addRoute("/vicinity/objects/<arg>/properties/<arg>", QHttpServerRequest::Method::Get, [=](QString oid, QString pid, const QHttpServerRequest& request, QHttpServerResponder &&responder) {
        return handle_VICINITY_GET_properties(oid, pid, request, responder);
        });

    addRoute("/vicinity/objects/<arg>/properties/<arg>", QHttpServerRequest::Method::Put, [=](QString oid, QString pid, const QHttpServerRequest& request, QHttpServerResponder &&responder) {
        return handle_VICINITY_PUT_properties(oid, pid, request, responder);
        });

    addRoute("/vicinity/objects/<arg>/actions/<arg>", QHttpServerRequest::Method::Post, [=](QString oid, QString aid, const QHttpServerRequest& request, QHttpServerResponder &&responder) {
        return handle_VICINITY_POST_action(oid, aid, request, responder);
        });

    addRoute(QString("/encrypt"), QHttpServerRequest::Method::Post, [=](const QHttpServerRequest& request, QHttpServerResponder &&responder) {
        return handle_local_encrypt(request, responder);
        });

    addRoute("/aggregate", QHttpServerRequest::Method::Post, [=](const QHttpServerRequest& request, QHttpServerResponder &&responder) {
        return handle_local_aggregate(request, responder);
        });

    addRoute("/decrypt", QHttpServerRequest::Method::Post, [=](const QHttpServerRequest& request, QHttpServerResponder &&responder) {
        return handle_local_decrypt(request, responder);
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


QJsonDocument rest_handler::get_blocking(const QUrl& endpoint, const QList<QPair<QByteArray, QByteArray>>& headers) {
    // disconnect first, so the reply buffer is not read by the slot
    disconnect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), nullptr, nullptr);

    QNetworkRequest req(endpoint);
    // add given headers
    for(auto header : headers)
    {
        req.setRawHeader(header.first, header.second);
    }
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


QJsonDocument rest_handler::put_blocking(const QUrl& endpoint, const QJsonDocument& payload) {
    // disconnect first, so the reply buffer is not read by the slot
    disconnect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), nullptr, nullptr);

    qDebug() << "received new call to put. Sending out request";

    QNetworkRequest req(endpoint);
    QScopedPointer<QNetworkReply> reply(m_pNetManager->put(req, payload.toJson()));

    qDebug() << "Waiting for request to finish...";
    QTime timeout = QTime::currentTime().addSecs(10);
    while (QTime::currentTime() < timeout && !reply->isFinished()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    qDebug() << "finished";
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Failure" << reply->errorString();
    }

    QByteArray replyPayload = reply->readAll();

    return QJsonDocument::fromJson(replyPayload);
}


QJsonDocument rest_handler::post_blocking(const QUrl& endpoint, const QJsonDocument& payload) {
    // disconnect first, so the reply buffer is not read by the slot
    disconnect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), nullptr, nullptr);

    QNetworkRequest req(endpoint);
    QScopedPointer<QNetworkReply> reply(m_pNetManager->post(req, payload.toJson()));

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


QJsonDocument rest_handler::extractJsonFromRequest(const QHttpServerRequest& request) {
        // read body as JSON
        QJsonDocument body_json = QJsonDocument::fromJson(request.body());

        // check if parsing succeeded
        if (body_json.isNull()) {
            throw std::invalid_argument("JSON parse error");
        }
        if (!body_json.isObject()) {
            throw std::invalid_argument("invalid JSON provided");
        }
        //if (!body_json.object().contains("value")) {
        //    throw std::invalid_argument("JSON does not contain a value");
        //}

        return body_json;
}


double rest_handler::extract_double_from_request(const QHttpServerRequest& request) {
    QString strValue;
    double value;

    // check content type and extract value accordingly
    QVariant contentType = request.value("Content-Type");
    if (contentType.toString() == "application/json") {
        QJsonDocument body_json = extractJsonFromRequest(request);

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
        auto fromUtf8 = QStringDecoder(QStringDecoder::Utf8);
        strValue = fromUtf8(request.body());
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
    QVariant contentType = request.value("Content-Type");
    if (contentType.toString() == "application/json") {
        // read body as JSON
        QJsonDocument body_json = extractJsonFromRequest(request);

        // at this point jsonValue contains the actual ctxt provided
        QJsonValue jsonValue = body_json.object().value("value");
        // return a double value
        return jsonValue.toString();
    }
    else {
        throw std::invalid_argument("no value provided or invalid Content-Type");
    }
}


QJsonObject rest_handler::handle_VICINITY_GET_objects(const QHttpServerRequest& request, QHttpServerResponder &responder) {
    qDebug() << "VICINITY Get Objects!";
    responder.write(QJsonDocument(m_pController->getVICINITY_handler()->getThingDescription()));
    return m_pController->getVICINITY_handler()->getThingDescription();
}


QJsonObject rest_handler::handle_VICINITY_GET_properties(QString oid, QString pid, const QHttpServerRequest& request, QHttpServerResponder &responder) {
    // TODO: sanitycheck for properties
    qDebug() << "VICINITY Get Parameters!";
    qDebug() << oid << pid;

    responder.write(QJsonDocument(m_pController->getVICINITY_handler()->readProperty(oid, pid)));
    return m_pController->getVICINITY_handler()->readProperty(oid, pid);
}


QJsonObject rest_handler::handle_VICINITY_PUT_properties(QString oid, QString pid, const QHttpServerRequest& request, QHttpServerResponder &responder) {
    qDebug() << "VICINITY Put Request!";
    QJsonDocument doc;
    try {
        doc = extractJsonFromRequest(request);
    }
    catch (std::invalid_argument& e) {
        QJsonObject error;
        error.insert("error", e.what());
        return error;
    }

    QJsonObject reply_payload = m_pController->getVICINITY_handler()->writeProperty(oid, pid, doc);
    responder.write(QJsonDocument(reply_payload));
    return reply_payload;
}


QJsonObject rest_handler::handle_VICINITY_POST_action(QString oid, QString aid, const QHttpServerRequest& request, QHttpServerResponder &responder) {
    qDebug() << "VICINITY Post Request!";
    qDebug() << "oid: " << oid << ", aid: " << aid;
    qDebug() << "headers: " << request.headers();
    qDebug() << "body: " << request.body();
    qDebug() << request.query().toString();
    qDebug() << request.url();

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


QJsonObject rest_handler::handle_local_encrypt(const QHttpServerRequest& request, QHttpServerResponder &responder) {
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

    responder.write(QJsonDocument(result));
    return result;
}


QJsonObject rest_handler::handle_local_aggregate(const QHttpServerRequest& request, QHttpServerResponder &responder) {
    std::cout << "Aggregate called locally." << std::endl;
    //string stvalue = message.extract_string().get();
    //std::cout << "Body: " + stvalue << std::endl;

    // check content type and extract value accordingly
    QString contentType = QString::fromUtf8(request.value("Content-Type"));
    if (contentType != "application/json") {
        responder.write(QHttpServerResponder::StatusCode::BadRequest);
        return QJsonObject();
    }

    // read body as JSON
    QJsonDocument body_json = extractJsonFromRequest(request);

    // at this point jsonValue contains the actual ctxt provided
    QJsonArray jsonValues = body_json.object().value("values").toArray();

    QStringList vec;
    for(auto it = jsonValues.begin(); it != jsonValues.end(); ++it) {
        //std::cout << it->as_string() << std::endl;
        try {
            vec.append(it->toString());
        } catch (...) {
            responder.write(QHttpServerResponder::StatusCode::BadRequest);
            return QJsonObject();
        }
    }

    std::string sourceOid = "";
    QString requestSourceOid = QString::fromUtf8(request.value("sourceOid"));
    if(!requestSourceOid.isEmpty()) {
        sourceOid = requestSourceOid.toStdString();
    }
    QString result = "";
    if(sourceOid == "") {
        result = m_pController->getHE_handler()->sum(vec, ""); //use own key
    } else {
        result = m_pController->getHE_handler()->sum(vec, (m_pController->getVICINITY_handler()->getPublicKey(sourceOid)).c_str());
    }
    //std::cout << "Result of aggregation: " + result << std::endl;
    std::cout << "Aggregate locally finished." << std::endl;
    QJsonObject resultObj
    {
        {"result", result}
    };
    responder.write(QJsonDocument(resultObj));
    return QJsonObject();
}

QJsonObject rest_handler::handle_local_decrypt(const QHttpServerRequest& request, QHttpServerResponder &responder) {
    QJsonObject result;
    QString value;
    try {
        value = extract_ctxt_from_request(request);
    }
    catch (std::invalid_argument& e) {
        result.insert("error", e.what());
        responder.write(QJsonDocument(result));
        return result;
    }

    double decrypted = m_pController->getHE_handler()->decrypt(value);

    result.insert("value", decrypted);

    responder.write(QJsonDocument(result));
    return result;
}