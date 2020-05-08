#include "../include/db_access.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;
#include "../third-party/sqlite/sqlite3.h"
#include <stdlib.h>

//constructor connects to database
//if there is no such database, create it
db_access::db_access(const char* database)
{
    databaseName = database; 
    
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);

    if (rc)
    {
        cout << "Can't open database: ";
        cout << sqlite3_errmsg(db) << endl;
        //fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }
    else {
        fprintf(stderr, "Opened database successfully\n");
    }

    const char* sql = "CREATE TABLE PUBLIC_KEYS(ID TEXT PRIMARY KEY, PUBLICKEY TEXT NOT NULL);";
    const char* sql2 = "CREATE TABLE OWN_KEYS(KEYTYPE TEXT PRIMARY KEY, VALUE TEXT NOT NULL)";
    const char* sql3 = "CREATE TABLE DATA_ACCESS(OWN_OID, ALLOWED_REQUESTER_OID TEXT)";
    const char* sql4 = "CREATE TABLE OWN_DEVICES(OID TEXT PRIMARY KEY, OID_PLAIN_DEVICE TEXT)";
    const char* sql5 = "CREATE TABLE FOREIGN_DEVICES_ACCESS_DECRYPT(OID TEXT PRIMARY KEY)";
    const char* sql6 = "CREATE TABLE PRIVACY_SERVICE(DEVICE_OID TEXT PRIMARY KEY, HE_SERVICE_OID TEXT)"; //is this really needed? TODO
    const char* sql7 = "CREATE TABLE AGGREGATION_TRUSTED_PARTIES(OID TEXT PRIMARY KEY)";
    const char* sql8 = "CREATE TABLE AGGREGATION_PARTIES_WHO_TRUST_ME(OID TEXT PRIMARY KEY)"; //do not insert own oid!
    const char* sql9 = "CREATE TABLE AGGREGATION_RANDOM_SHARES(INITIATOR_OID TEXT, PARTICIPANT_OID TEXT, RECEIVED INTEGER, SHARE INTEGER, PRIMARY KEY(INITIATOR_OID, PARTICIPANT_OID))";
    const char* sql10 = "CREATE TABLE AGGREGATION_TRUSTED_INITIATORS(OID TEXT PRIMARY KEY)";
    const char* sql11 = "CREATE TABLE AGGREGATION_BLINDED_MEASUREMENTS(PARTICIPANT_OID TEXT PRIMARY KEY, RECEIVED INTEGER, BLINDED_MEASUREMENT INTEGER)";
    
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql2, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql3, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql4, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql5, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql6, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql7, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql8, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql9, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql10, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql11, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    sqlite3_close(db);
    
    resetRandomShares(); //TODO decomment!
    resetBlindedMeasurements(); //TODO decomment!
    
    std::cout << std::string("THREADSAFE?: ") + std::to_string(sqlite3_threadsafe());
    //if(sqlite3_db_mutex(sqlite3*) == NULL) {
        //std::cout << "NUUUUULLLL" << std::endl;
    //}
    
    /*
    updateShareRandomShares("oid 1", "oid 2", 13373);
    if(trustsMe("oid 1")) {
        std::cout << "YEAAH" << std::endl;
    }
    std::cout << "YEAAH22" << std::endl;
    
    insertParticipantRandomShares("SOURZ", "PARTIZ");
    */
}

//destructor
db_access::~db_access()
{
    
}

//to save own keys (public and private key) in database
bool db_access:: insert_own_key(const char* keytype, const char* value)
{
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "INSERT OR REPLACE INTO OWN_KEYS";
    sql.append(" (KEYTYPE,VALUE) VALUES (\'");
    sql.append(keytype);
    sql.append("\',\'");
    sql.append(value);
    sql.append("\');");  
    
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    sqlite3_close(db);
    printf(zErrMsg);
    cout << "Values posted" << endl;
    
	return true;
}

//to save own keys (public and private key) in database
bool db_access:: insert_own_key(const char* keytype, std::string & value)
{
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    sqlite3_stmt *stmt;
    sqlite3_prepare(db, "INSERT OR REPLACE INTO OWN_KEYS (KEYTYPE,VALUE) VALUES (?,?);",
                    -1,
                    &stmt,
                    0
    );
    
    std::string keyy = keytype;
    //bind params
    sqlite3_bind_text(stmt,
                      1,  //1st param
                      keyy.data(),
                      keyy.size(),
                      0
    );
    sqlite3_bind_text(stmt,
                      2, //2nd param
                      value.data(),
                      value.size(),
                      0
    );
    //execute statement
    sqlite3_step(stmt);
    //free
    sqlite3_finalize(stmt);
    
    sqlite3_close(db);
	return true;
}

//used to retrieve own keys (including private key) from the database
std::string db_access::get_own_key(const char* keytype)
{
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "select VALUE from OWN_KEYS where KEYTYPE = \'";
    sql.append(keytype);
    sql.append("\';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    std::string value = "";
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            value = results[i];
        }
    }
    if (rows * columns == 0) { //key not in database
        cout << "Keytype not found in database" << endl;
        return "";
    }
    return value;
}

//insert public key
bool db_access::insert_public_key(const char* id, const char* key)
{    
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "BEGIN IMMEDIATE TRANSACTION; INSERT OR REPLACE INTO PUBLIC_KEYS";
    sql.append(" (ID,PUBLICKEY) VALUES (\'");
    sql.append(id);
    sql.append("\',\'");
    sql.append(key);
    sql.append("\'); COMMIT TRANSACTION;");
    
    //std::cout << sql.c_str() << std::endl;

    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    sqlite3_close(db);
    printf(zErrMsg);
    cout << "Values posted" << endl;
    
	return true;
}

//insert public key
bool db_access::insert_public_key(const char* id, std::string & key)
{    
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "BEGIN IMMEDIATE TRANSACTION; INSERT OR REPLACE INTO PUBLIC_KEYS";
    sql.append(" (ID,PUBLICKEY) VALUES (\'");
    sql.append(id);
    sql.append("\',\'");
    sql.append(key);
    sql.append("\'); COMMIT TRANSACTION;");
    
    //std::cout << sql.c_str() << std::endl;

    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    sqlite3_close(db);
    printf(zErrMsg);
    cout << "Values posted" << endl;
    
	return true;
}

//get public key, returns empty string if key is not in database!
std::string db_access::get_public_key(const char * id)
{
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "select PUBLICKEY from PUBLIC_KEYS where ID = \'";
    sql.append(id);
    sql.append("\';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    std::string key = "";
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            key = results[i];
        }
        //key = "TEST";
    }
    if (rows * columns == 0) { //key not in database
        cout << "ID not found in database" << endl;
        return "";
    }
    
    return key;
}

/*
//check if id is in Access table (check if id has access to decrypt and data)
bool db_access::hasAccess(const char* id)
{
    string sql = "select OID from ACCESS where OID = \'";
    sql.append(id);
    sql.append("\'");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    std::string key = "";
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        return true;
    }
    else return false;
}
*/
/*
//check if id is found in ACCESS table
bool db_access::hasAccess2(const char * id)
{
    string sql = "select OID from ACCESS where OID = \'";
    sql.append(id);
    sql.append("\'");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry //=> id found
        return true;
    }
    return false;
}
*/

//device of owner of this service
bool db_access::isOwnDevice(const char * id)
{
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "select OID from OWN_DEVICES where OID = \'";
    sql.append(id);
    sql.append("\';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry //=> id found
        return true;
    }
    return false;
}

//unencrypted device of encrypted device of owner of this service
std::string db_access::getPlainDeviceOfOwnEncryptedDevice(const char * id)
{
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "select OID_PLAIN_DEVICE from OWN_DEVICES where OID = \'";
    sql.append(id);
    sql.append("\';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    std::string output = "";
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            output = results[i];
        }
    }
    if (rows * columns == 0) { //key not in database
        cout << "Not found in database" << endl;
        return std::string("");
    }
    return output;
}

//owners device, or trusted device
bool db_access::hasAccessToDecrypt(const char * id)
{
    if(isOwnDevice(id)) {
        return true;
    }
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    //else, check if it is a foreign device with access to decrypt
    string sql = "select OID from FOREIGN_DEVICES_ACCESS_DECRYPT where OID = \'";
    sql.append(id);
    sql.append("\';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry //=> id found
        return true;
    }
    return false;
}

//check if data requester is allowed to data
//only used so other parties can know, with whom we want to share data!
bool db_access::hasAccessToData(const char * data_requester_oid, const char * destination_oid)
{
    if(!isOwnDevice(destination_oid)) {
        return false; //destination is not our device!
    }
    if(isOwnDevice(data_requester_oid)) {
        return true;
    }

    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    //as it is not the owner, check if requester oid has access to this device
    string sql = "select OID from DATA_ACCESS where OWN_OID = \'";
    sql.append(destination_oid);
    sql.append("\' and ALLOWED_REQUESTER_OID = \'");
    sql.append(data_requester_oid);
    sql.append("\';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry //=> id found
        return true;
    }
    return false;
}

//get the corresponding privacy service oid of input oid
std::string db_access::getPrivacyService(const char * id)
{
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "select HE_SERVICE_OID from PRIVACY_SERVICE where DEVICE_OID = \'";
    sql.append(id);
    sql.append("\';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    std::string service = "";
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            service = results[i];
        }
    }
    if (rows * columns == 0) { //key not in database
        cout << "ID not found in database" << endl;
        return "";
    }
    return service;
}

//clears the table
void db_access::resetRandomShares() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "BEGIN IMMEDIATE TRANSACTION; delete from AGGREGATION_RANDOM_SHARES; COMMIT TRANSACTION;";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    sqlite3_close(db);
}

//has to be called after finished or aborted computation with the requester id (initiator oid)
void db_access::deleteInitiatorRandomShares(const char * initiatorOid) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "BEGIN IMMEDIATE TRANSACTION; delete from AGGREGATION_RANDOM_SHARES where INITIATOR_OID = \'";
    sql.append(initiatorOid);
    sql.append("\'; COMMIT TRANSACTION;");
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    
    sqlite3_close(db);
}

//update (insert) share
void db_access::updateShareRandomShares(const char * initiatorOid, const char * participantOid, int value) {
    std::cout << "UUUUUUUPDATE RANDOM SHARE BEGIN" << std::endl;
    
    string sql = string("BEGIN IMMEDIATE TRANSACTION; UPDATE AGGREGATION_RANDOM_SHARES SET RECEIVED = 1, SHARE = ") + std::to_string(value) + string(" WHERE INITIATOR_OID = '") + initiatorOid + string("' AND PARTICIPANT_OID = '") + participantOid + string("'; COMMIT TRANSACTION;");
    int rc = 0;
    do {
        sqlite3* db;
        char* zErrMsg = 0;
        rc = sqlite3_open(databaseName, &db);
        sqlite3_busy_timeout(db, 2000);
        
        if(rc != SQLITE_OK) {
            std::cout << "PROBLEM WHEN OPENING DB" << std::endl;
        }
        std::cout << "5555555555555555555555 11" << std::endl;
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        std::cout << "5555555555555555555555" << std::endl;
        sqlite3_close(db);
        if(rc != SQLITE_OK){
            std::cout << std::string("SQLITE ERROR: ") + zErrMsg << std::endl;
        }
    } while(rc != SQLITE_OK);
    
    std::cout << "UUUUUUUPDATE RANDOM SHARE END" << std::endl;
}

//returns true, if we received the request for the computation and the participant was inserted
bool db_access::isAlreadyInsertedInRandomShares(const char * initiatorOid, const char * participantOid) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("select * from AGGREGATION_RANDOM_SHARES where PARTICIPANT_OID = '") + participantOid + string("' and INITIATOR_OID = '") + initiatorOid + string("';");
    //std::cout << "IS ALREADY INSERTED SQL: " + sql << std::endl;
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows == 0) { //0 means not inserted
        return false;
    }
    return true;
}

//returns true if all shares received
bool db_access::allRandomSharesReceived(const char * initiatorOid) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("select PARTICIPANT_OID from AGGREGATION_RANDOM_SHARES where RECEIVED = 0 and INITIATOR_OID = '") + initiatorOid + string("';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows == 0) { //0 means, we received all shares of this aggregation
        return true;
    }
    return false;
}

//if oid trusts this service (-> would send share in aggregation
bool db_access::trustsMe(const char * oid) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("select OID from AGGREGATION_PARTIES_WHO_TRUST_ME where OID = '") + oid + string("';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows == 0) { //0 means, this party does not trust us
        return false;
    }
    return true;
}


//if we allowed this oid to start distributed aggregation
bool db_access::isTrustedInitiator(const char * oid) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("select OID from AGGREGATION_TRUSTED_INITIATORS where OID = '") + oid + string("';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows == 0) { //0 means, this party does not trust us
        return false;
    }
    return true;
}

//insert participants of an aggregation. function automatically determines the needed participants (those who trust me apart from myself)
//participants can be inserted multiple times, will only be saves once in database (for this aggregation)
void db_access::insertParticipantRandomShares(const char * initiatorOid, const char * participantOid) {
    if(trustsMe(participantOid)) { //only participants who trust me send a share; in the trust me table, the own oid musn't be inserted!    
        sqlite3* db;
        char* zErrMsg = 0;
        int rc = sqlite3_open(databaseName, &db);
        sqlite3_busy_timeout(db, 2000);
        
        string sql = string("BEGIN IMMEDIATE TRANSACTION; INSERT OR REPLACE INTO AGGREGATION_RANDOM_SHARES (INITIATOR_OID, PARTICIPANT_OID, RECEIVED, SHARE) VALUES (\'") + initiatorOid + string("\',\'") + participantOid + string("\',0,0); COMMIT TRANSACTION;");
        rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
        sqlite3_close(db);
        //cout << std::string("ERR MSG INSERT RANDOM SHARE: ") + zErrMsg << endl;
    }
}

//insert participants of own aggregation into database
void db_access::insertParticipantBlindedMeasurements(const char * participantOid) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("BEGIN IMMEDIATE TRANSACTION; INSERT OR REPLACE INTO AGGREGATION_BLINDED_MEASUREMENTS (PARTICIPANT_OID, RECEIVED, BLINDED_MEASUREMENT) VALUES (\'") + participantOid + string("\',0,0); COMMIT TRANSACTION;");
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    
    sqlite3_close(db);
}

//update (insert) blinded measurement
void db_access::updateBlindedMeasurement(const char * participantOid, int value) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("BEGIN IMMEDIATE TRANSACTION; UPDATE AGGREGATION_BLINDED_MEASUREMENTS SET RECEIVED = 1, BLINDED_MEASUREMENT = ") + std::to_string(value) + string(" WHERE PARTICIPANT_OID = '") + participantOid + string("'; COMMIT TRANSACTION;");
    std::cout << std::string("SQL: ") + sql << std::endl;
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    
    sqlite3_close(db);
}

//if all shares received of own aggregation (also true, if there are no entries at all)
bool db_access::allBlindedMeasurementsReceived() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("select PARTICIPANT_OID from AGGREGATION_BLINDED_MEASUREMENTS where RECEIVED = 0';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows == 0) { //0 means, we received all shares of this aggregation
        return true;
    }
    return false;
}

//resets the complete table
void db_access::resetBlindedMeasurements() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "BEGIN IMMEDIATE TRANSACTION; delete from AGGREGATION_BLINDED_MEASUREMENTS; COMMIT TRANSACTION;";
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    
    sqlite3_close(db);
}

//returns all trusted parties for distributed aggregation
std::vector<std::string> db_access::getTrustedParties() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "select OID from AGGREGATION_TRUSTED_PARTIES;";
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    std::vector<std::string> trustedParties;
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            trustedParties.push_back(std::string(results[i]));
        }
    }
    return trustedParties;
}

//returns true, if oid is a trusted party
bool db_access::isTrustedParty(const char* oid) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("select OID from AGGREGATION_TRUSTED_PARTIES where OID = '") + oid + string("';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    if (rows == 0) { //0 means, we do not trust this party
        return false;
    }
    return true;
}


//returns the sum of the blinded measurements -> the result of the aggregation
int db_access::getBlindedMeasurementSum() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = "select BLINDED_MEASUREMENT from AGGREGATION_BLINDED_MEASUREMENTS;";
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    int sum = 0;
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            sum += atoi(results[i]);
        }
    }
    return sum;
}

//returns sum of received random shares of the aggregateion initiated by initiatorOid
int db_access::getRandomShareSum(const char* initiatorOid) {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(databaseName, &db);
    sqlite3_busy_timeout(db, 2000);
    
    string sql = string("select SHARE from AGGREGATION_RANDOM_SHARES where INITIATOR_OID = '") + initiatorOid + string("';");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    sqlite3_close(db);
    int sum = 0;
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            sum += atoi(results[i]);
        }
    }
    return sum;
}
