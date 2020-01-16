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
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }
    else {
        fprintf(stderr, "Opened database successfully\n");
    }

    const char* sql = "CREATE TABLE PUBLIC_KEYS(ID INT PRIMARY KEY, PUBLICKEY TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    
    printf(zErrMsg);
}

//destructor
db_access::~db_access()
{
    sqlite3_close(db);
}

//insert public key
bool db_access::insert_public_key(int id, const char* key)
{    
    string sql = "INSERT OR REPLACE INTO PUBLIC_KEYS";
    sql.append(" (ID,PUBLICKEY) VALUES (");
    sql.append(to_string(id));
    sql.append(",\'");
    sql.append(key);
    sql.append("\');");

    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    printf(zErrMsg);
    printf("values posted\n");
    
	return true;
}

//get public key
int db_access::get_public_key(int id)
{
    string sql = "select DATA from PUBLIC_KEYS where ID = ";
    sql.append(to_string(id));
    
    char** results = NULL;
    int rows, columns;
    char* error;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    int sum = 0;
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            sum += atoi(results[i]);
        }
    }

    return sum;
}
