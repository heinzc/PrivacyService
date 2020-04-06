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
    zErrMsg = 0;
    rc = sqlite3_open(database, &db);
    data = "Callback function called";

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
    const char* sql4 = "CREATE TABLE OWN_DEVICES(OID TEXT PRIMARY KEY)";
    const char* sql5 = "CREATE TABLE FOREIGN_DEVICES_ACCESS_DECRYPT(OID TEXT PRIMARY KEY)";
    const char* sql6 = "CREATE TABLE PRIVACY_SERVICE(DEVICE_OID TEXT PRIMARY KEY, HE_SERVICE_OID TEXT)"; //is this really needed? TODO
    //const char* sql7 = "CREATE TABLE AGGREGATE_TRUSTED_PARTIES(OID TEXT PRIMARY KEY)";
    //const char* sql8 = "CREATE TABLE AGGREGATE_PARTIES_WHO_TRUST_US(OID TEXT PRIMARY KEY)";
    
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
}

//destructor
db_access::~db_access()
{
    sqlite3_close(db);
}

//to save own keys (public and private key) in database
bool db_access:: insert_own_key(const char* keytype, const char* value)
{
    string sql = "INSERT OR REPLACE INTO OWN_KEYS";
    sql.append(" (KEYTYPE,VALUE) VALUES (\'");
    sql.append(keytype);
    sql.append("\',\'");
    sql.append(value);
    sql.append("\');");  
    

    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    printf(zErrMsg);
    cout << "Values posted" << endl;
    
	return true;
}

//to save own keys (public and private key) in database
bool db_access:: insert_own_key(const char* keytype, std::string & value)
{
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
	return true;
}

//used to retrieve own keys (including private key) from the database
std::string db_access::get_own_key(const char* keytype)
{
    string sql = "select VALUE from OWN_KEYS where KEYTYPE = \'";
    sql.append(keytype);
    sql.append("\'");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
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
    string sql = "INSERT OR REPLACE INTO PUBLIC_KEYS";
    sql.append(" (ID,PUBLICKEY) VALUES (\'");
    sql.append(id);
    sql.append("\',\'");
    sql.append(key);
    sql.append("\');");
    
    std::cout << sql.c_str() << std::endl;

    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    printf(zErrMsg);
    cout << "Values posted" << endl;
    
	return true;
}

//insert public key
bool db_access::insert_public_key(const char* id, std::string & key)
{    
    string sql = "INSERT OR REPLACE INTO PUBLIC_KEYS";
    sql.append(" (ID,PUBLICKEY) VALUES (\'");
    sql.append(id);
    sql.append("\',\'");
    sql.append(key);
    sql.append("\');");
    
    std::cout << sql.c_str() << std::endl;

    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    printf(zErrMsg);
    cout << "Values posted" << endl;
    
	return true;
}

//get public key, returns empty string if key is not in database!
std::string db_access::get_public_key(const char * id)
{
    string sql = "select PUBLICKEY from PUBLIC_KEYS where ID = \'";
    sql.append(id);
    sql.append("\'");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
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
    string sql = "select OID from OWN_DEVICES where OID = \'";
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

//owners device, or trusted device
bool db_access::hasAccessToDecrypt(const char * id)
{
    if(isOwnDevice(id)) {
        return true;
    }
    //else, check if it is a foreign device with access to decrypt
    string sql = "select OID from FOREIGN_DEVICES_ACCESS_DECRYPT where OID = \'";
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
    //as it is not the owner, check if requester oid has access to this device
    string sql = "select OID from DATA_ACCESS where OWN_OID = \'";
    sql.append(destination_oid);
    sql.append("\' and ALLOWED_REQUESTER_OID = \'");
    sql.append(data_requester_oid);
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

//get the corresponding privacy service oid of input oid
std::string db_access::getPrivacyService(const char * id)
{
    string sql = "select HE_SERVICE_OID from PRIVACY_SERVICE where DEVICE_OID = \'";
    sql.append(id);
    sql.append("\'");
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
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

