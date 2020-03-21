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
    const char* sql3 = "CREATE TABLE ACCESS(OID TEXT PRIMARY KEY)";
    
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql2, callback, 0, &zErrMsg);
    //cout << zErrMsg << endl; //TODO doesn't work when there is no db anymore
    
    rc = sqlite3_exec(db, sql3, callback, 0, &zErrMsg);
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
