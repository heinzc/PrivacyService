#include "db_access.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include <stdlib.h>

#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlDatabase>

using namespace std;

//constructor connects to database
//if there is no such database, create it
db_access::db_access(QString dbName)
{
    m_dbName = dbName;
    m_db = nullptr;

    if ( open() ) {
        qDebug() << "Database open";
    }

    if (createTables()) {
        qDebug() << "Database Tables created";
    }

    
    resetRandomShares();
    resetBlindedMeasurements();
}

//destructor
db_access::~db_access()
{
    m_db->close();
}


bool db_access::open() {
    /* Open database */
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
    m_db->setDatabaseName(m_dbName);

    if (!m_db->open())
    {
        qDebug() << "Can't open database: " << m_dbName;
        return false;
    }
    else
    {
        qDebug() << "Using SqLite DB: " << m_dbName;
    }

    return true;
}


bool db_access::createTables() {
    QStringList createQueries;
    createQueries << "CREATE TABLE IF NOT EXISTS PUBLIC_KEYS(ID TEXT PRIMARY KEY, PUBLICKEY TEXT NOT NULL);"
        << "CREATE TABLE IF NOT EXISTS OWN_KEYS(KEYTYPE TEXT PRIMARY KEY, VALUE TEXT NOT NULL)"
        << "CREATE TABLE IF NOT EXISTS DATA_ACCESS(OWN_OID TEXT, ALLOWED_REQUESTER_OID TEXT)"
        << "CREATE TABLE IF NOT EXISTS OWN_DEVICES(OID TEXT PRIMARY KEY, OID_PLAIN_DEVICE TEXT)"
        << "CREATE TABLE IF NOT EXISTS FOREIGN_DEVICES_ACCESS_DECRYPT(OID TEXT PRIMARY KEY)"
        << "CREATE TABLE IF NOT EXISTS PRIVACY_SERVICE(DEVICE_OID TEXT PRIMARY KEY, HE_SERVICE_OID TEXT)"
        << "CREATE TABLE IF NOT EXISTS AGGREGATION_TRUSTED_PARTIES(OID TEXT PRIMARY KEY)"
        << "CREATE TABLE IF NOT EXISTS AGGREGATION_PARTIES_WHO_TRUST_ME(OID TEXT PRIMARY KEY)" //do not insert own oid!
        << "CREATE TABLE IF NOT EXISTS AGGREGATION_RANDOM_SHARES(INITIATOR_OID TEXT, PARTICIPANT_OID TEXT, RECEIVED REAL, SHARE REAL, PRIMARY KEY(INITIATOR_OID, PARTICIPANT_OID))"
        << "CREATE TABLE IF NOT EXISTS AGGREGATION_TRUSTED_INITIATORS(OID TEXT PRIMARY KEY)"
        << "CREATE TABLE IF NOT EXISTS AGGREGATION_BLINDED_MEASUREMENTS(PARTICIPANT_OID TEXT PRIMARY KEY, RECEIVED REAL, BLINDED_MEASUREMENT REAL)";

    if (!m_db->isOpen()) {
        m_db->open();
    }

    for (QString queryString : qAsConst(createQueries)) {
        QSqlQuery query = m_db->exec(queryString);
        if (!query.isActive()) {
            qDebug() << "query not active: " << query.lastError().text();
            return false;
        }
    }

    return true;
}


//to save own keys (public and private key) in database
bool db_access::insert_own_key(const QString& keytype, const QString& value) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("INSERT OR REPLACE INTO OWN_KEYS (KEYTYPE,VALUE) VALUES (:keytype, :value);");
    query.bindValue(":keytype", keytype);
    query.bindValue(":value", value);
    query.exec();

    if (!query.isActive()) {
        qDebug() << query.lastError().text();
        return false;
    }

    qDebug() << "inserting " << keytype << " successful";
    return true;
}


//used to retrieve own keys (including private key) from the database
QString db_access::get_own_key(const QString& keytype)
{
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT VALUE from OWN_KEYS where KEYTYPE = \'" + keytype + "\';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.first()) {
        qDebug() << query.lastError().text();
        return QString();
    }

    QSqlRecord rec = query.record();

    return rec.value("VALUE").toString();
}


//insert public key
bool db_access::insert_public_key(const QString& id, const QString& pkJson) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("INSERT OR REPLACE INTO PUBLIC_KEYS (ID,PUBLICKEY) VALUES (:id, :pubkey);");
    query.bindValue(":id", id);
    query.bindValue(":pubkey", pkJson);
    query.exec();

    if (!query.isActive()) {
        qDebug() << query.lastError().text();
        return false;
    }

    qDebug() << "inserting pubkey " << id << " successful";
    return true;
}


//get public key, returns empty string if key is not in database!
QString db_access::get_public_key(const QString& id) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT PUBLICKEY from PUBLIC_KEYS where ID = \'" + id + "\';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.first()) {
        qDebug() << query.lastError().text();
        return QString();
    }

    QSqlRecord rec = query.record();

    return rec.value("PUBLICKEY").toString();
}


////device of owner of this service
//bool db_access::isOwnDevice(const char * id)
//{
//    sqlite3* db;
//    char* zErrMsg = 0;
//    int rc = sqlite3_open(databaseName, &db);
//    sqlite3_busy_timeout(db, 2000);
//    
//    string sql = "select OID from OWN_DEVICES where OID = \'";
//    sql.append(id);
//    sql.append("\';");
//    
//    char** results = NULL;
//    int rows, columns;
//    char* error;
//    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
//    sqlite3_close(db);
//    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry //=> id found
//        return true;
//    }
//    return false;
//}
//
////unencrypted device of encrypted device of owner of this service
//std::string db_access::getPlainDeviceOfOwnEncryptedDevice(const char * id)
//{
//    sqlite3* db;
//    char* zErrMsg = 0;
//    int rc = sqlite3_open(databaseName, &db);
//    sqlite3_busy_timeout(db, 2000);
//    
//    string sql = "select OID_PLAIN_DEVICE from OWN_DEVICES where OID = \'";
//    sql.append(id);
//    sql.append("\';");
//    
//    char** results = NULL;
//    int rows, columns;
//    char* error;
//    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
//    sqlite3_close(db);
//    std::string output = "";
//    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
//        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
//            output = results[i];
//        }
//    }
//    if (rows * columns == 0) { //key not in database
//        cout << "Not found in database" << endl;
//        return std::string("");
//    }
//    return output;
//}


//owners device, or trusted device
bool db_access::hasAccessToDecrypt(const QString& id) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT OID from FOREIGN_DEVICES_ACCESS_DECRYPT where OID = \'" + id + "\';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there is no result (no first record), deny access
    if (!query.first()) {
        qDebug() << query.lastError().text();
        return false;
    }

    // if id was found, return true
    return true;
}


//check if data requester is allowed to data
//only used so other parties can know, with whom we want to share data!
bool db_access::hasAccessToData(const QString& data_requester_oid, const QString& destination_oid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT OID from DATA_ACCESS where OWN_OID = \'" + destination_oid + "\'and ALLOWED_REQUESTER_OID = \'" + data_requester_oid + "\';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there is no result (no first record), deny access
    if (!query.first()) {
        qDebug() << query.lastError().text();
        return false;
    }

    // if id was found, return true
    return true;
}


//get the corresponding privacy service oid of input oid
QString db_access::getPrivacyService(const QString& id) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT HE_SERVICE_OID from PRIVACY_SERVICE where DEVICE_OID = \'" + id + "\';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.first()) {
        qDebug() << query.lastError().text();
        return QString();
    }

    QSqlRecord rec = query.record();

    return rec.value("PUBLICKEY").toString();
}


//has to be called after finished or aborted computation with the requester id (initiator oid)
void db_access::deleteInitiatorRandomShares(const QString& initiatorOid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("delete from AGGREGATION_RANDOM_SHARES where INITIATOR_OID = \'" + initiatorOid + "\';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.isActive()) {
        qDebug() << "query not active: " << query.lastError().text();
    }
}


//update (insert) share
void db_access::updateShareRandomShares(const QString& initiatorOid, const QString& participantOid, double value) {
    qDebug() << "Update Random Share Begin";

    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("UPDATE AGGREGATION_RANDOM_SHARES SET RECEIVED = 1, SHARE = " + QString::number(value) + " WHERE INITIATOR_OID = '" + initiatorOid + "' AND PARTICIPANT_OID = '" + participantOid + "';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.isActive()) {
        qDebug() << "query not active: " << query.lastError().text();
    }

    qDebug() << "Update Random Share End";
}


//returns true, if we received the request for the computation and the participant was inserted
bool db_access::isAlreadyInsertedInRandomShares(const QString& initiatorOid, const QString& participantOid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT * from AGGREGATION_RANDOM_SHARES where PARTICIPANT_OID = '" + participantOid + "' and INITIATOR_OID = '" + initiatorOid + "';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there is no result (no first record), deny
    if (!query.first()) {
        qDebug() << query.lastError().text();
        return false;
    }

    // if id was found, return true
    return true;
}


//returns true if all shares received
bool db_access::allRandomSharesReceived(const QString& initiatorOid) {

    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT PARTICIPANT_OID from AGGREGATION_RANDOM_SHARES where RECEIVED = 0 and INITIATOR_OID = '" + initiatorOid + "';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there is no result, there is no missing share
    if (!query.first()) {
        qDebug() << query.lastError().text();
        return true;
    }
    // if something is still missing, return false
    return false;
}


//if oid trusts this service (-> would send share in aggregation
bool db_access::trustsMe(const QString& oid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT OID from AGGREGATION_PARTIES_WHO_TRUST_ME where OID = '" + oid + "';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there is no result, this oid does not trust us
    if (!query.first()) {
        qDebug() << query.lastError().text();
        return false;
    }
    // if something is still missing, return false
    return true;
}


//if we allowed this oid to start distributed aggregation
bool db_access::isTrustedInitiator(const QString& oid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT OID from AGGREGATION_TRUSTED_INITIATORS where OID = '" + oid + "';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there is no result, this oid does not trust us
    if (!query.first()) {
        qDebug() << query.lastError().text();
        return false;
    }
    // if something is still missing, return false
    return true;
}


//insert new participant of an aggregation. function automatically determines the needed participants (those who trust me apart from myself)
//participants can be inserted multiple times, will only be saves once in database (for this aggregation)
bool db_access::insertParticipantRandomShares(const QString& initiatorOid, const QString& participantOid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("INSERT OR REPLACE INTO AGGREGATION_RANDOM_SHARES (INITIATOR_OID, PARTICIPANT_OID, RECEIVED, SHARE) VALUES (\'" + initiatorOid + "\',\'" + participantOid + "\',0,0);");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.isActive()) {
        qDebug() << query.lastError().text();
        return false;
    }

    return true;
}


//insert participants of own aggregation into database
bool db_access::insertParticipantBlindedMeasurements(const QString& participantOid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("INSERT OR REPLACE INTO AGGREGATION_BLINDED_MEASUREMENTS (PARTICIPANT_OID, RECEIVED, BLINDED_MEASUREMENT) VALUES (\'" + participantOid + "\',0,0);");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.isActive()) {
        qDebug() << query.lastError().text();
        return false;
    }

    return true;
}


//update (insert) blinded measurement
bool db_access::updateBlindedMeasurement(const QString& participantOid, double value) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("UPDATE AGGREGATION_BLINDED_MEASUREMENTS SET RECEIVED = 1, BLINDED_MEASUREMENT = " + QString::number(value) + " WHERE PARTICIPANT_OID = '" + participantOid + "';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.isActive()) {
        qDebug() << query.lastError().text();
        return false;
    }

    return true;
}


//if all shares received of own aggregation (also true, if there are no entries at all)
bool db_access::allBlindedMeasurementsReceived() {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT PARTICIPANT_OID from AGGREGATION_BLINDED_MEASUREMENTS where RECEIVED = 0';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there is no result, this oid does not trust us
    if (!query.first()) {
        qDebug() << query.lastError().text();
        return true;
    }
    // if something is still missing, return false
    return false;
}



//returns all trusted parties for distributed aggregation
QStringList db_access::getTrustedParties() {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QStringList result;
    QSqlQuery query(*m_db);

    query.prepare("SELECT OID from AGGREGATION_TRUSTED_PARTIES;");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there was an error, show message and return empty list
    if (!query.isActive()) {
        qDebug() << query.lastError().text();
        return result;
    }

    // otherwise populate the result list.
    while (query.next()) {
        result.push_back(query.value("OID").toString());
    }

    return result;
}


//returns true, if oid is a trusted party
bool db_access::isTrustedParty(const QString& oid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("SELECT OID from AGGREGATION_TRUSTED_PARTIES where OID = '" + oid + "';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there is no result, we do not trust this oid
    if (!query.first()) {
        qDebug() << query.lastError().text();
        return false;
    }
    // if something is still missing, return false
    return true;
}


//returns the sum of the blinded measurements -> the result of the aggregation
double db_access::getBlindedMeasurementSum() {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    double result = 0;
    QSqlQuery query(*m_db);

    query.prepare("SELECT BLINDED_MEASUREMENT from AGGREGATION_BLINDED_MEASUREMENTS;");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there was an error, show message and return 0
    if (!query.isActive()) {
        qDebug() << query.lastError().text();
        return result;
    }

    // otherwise populate the result list.
    while (query.next()) {
        result += query.value("BLINDED_MEASUREMENT").toDouble();
    }

    return result;
}

//returns sum of received random shares of the aggregateion initiated by initiatorOid
double db_access::getRandomShareSum(const QString& initiatorOid) {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    double result = 0;
    QSqlQuery query(*m_db);

    query.prepare("SELECT SHARE from AGGREGATION_RANDOM_SHARES where INITIATOR_OID = '" + initiatorOid + "';");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    // if there was an error, show message and return 0
    if (!query.isActive()) {
        qDebug() << query.lastError().text();
        return result;
    }

    // otherwise populate the result list.
    while (query.next()) {
        result += query.value("SHARE").toDouble();
    }

    return result;
}


//clears the table
bool db_access::resetRandomShares() {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("DELETE from AGGREGATION_RANDOM_SHARES;");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.isActive()) {
        qDebug() << "query not active: " << query.lastError().text();
        return false;
    }

    return true;
}


//resets the complete table
bool db_access::resetBlindedMeasurements() {
    if (!m_db->isOpen()) {
        m_db->open();
    }

    QSqlQuery query(*m_db);

    query.prepare("DELETE from AGGREGATION_BLINDED_MEASUREMENTS;");
    //query.bindValue(":keytype", keytype); // TODO: see why binding Values is not working...
    query.exec();

    if (!query.isActive()) {
        qDebug() << "query not active: " << query.lastError().text();
        return false;
    }

    return true;
}