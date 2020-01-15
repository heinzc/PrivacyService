#include "../include/phe_db_access.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;
#include "../third-party/sqlite/sqlite3.h"
#include <stdlib.h>

//constructor connects to database
//if there is no such database, create it
phe_db_access::phe_db_access(const char* database) :
    db_access()
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

    const char* sql = "CREATE TABLE PHE_Public_Keys(ID INT NOT NULL, DATA INT NOT NULL);";
    //TODO create table for FHE
    
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
}

//destructor
phe_db_access::~phe_db_access()
{
    sqlite3_close(db);
}

//insert public key
bool phe_db_access::insert_public_key(const char* table, int id, int data)
{
    string sql = "INSERT INTO ";
    sql.append(table);
    sql.append(" (ID,DATA) VALUES (");
    sql.append(to_string(id));
    sql.append(",");
    sql.append(to_string(data));
    sql.append(");");

    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    printf("values posted\n");
	return true;
}

//get public key
int phe_db_access::get_public_key(const char* table, int id)
{
    string sql = "select DATA from ";
    sql.append(table);
    sql.append(" where ID = ");
    sql.append(to_string(id));
    
    char** results = NULL;
    int rows, columns;
    char* error;
    //std::vector<std::string> values;
    sqlite3_get_table(db, sql.c_str(), &results, &rows, &columns, &error);
    int sum = 0;
    //printf("\nrows");
    //printf(std::to_string(rows).c_str());
    //printf("col");
    //printf(std::to_string(columns).c_str());
    //printf("\n");
    if (rows > 0) { //because result[0] is column name, but rows does not include the column entry
        for (int i = 1; i <= rows * columns; i++) { //this is also why we need the offset of 1
            sum += atoi(results[i]);
        }
    }

    return sum;
}
