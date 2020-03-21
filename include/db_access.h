#pragma once

#include "../third-party/sqlite/sqlite3.h"
#include <stdio.h>
#include <string>

class db_access
{
	public:
        //constructor connects to database
        //if there is no such database, create it
        db_access(const char* database);

        //destructor
        ~db_access();

        //to save own keys (public and private key) in database
        bool insert_own_key(const char* keytype, const char* value);
        
        //used to retrieve own keys (including private key) from the database
        std::string get_own_key(const char* keytype);
        
        //insert public key
        bool insert_public_key(const char* id, const char* key);
        
        //get public key, returns empty string if key is not in database!
        std::string get_public_key(const char* id);
        
        //check if id is in Access table (check if id has access to decrypt and data)
        bool hasAccess(const char* id);
        
        //check if id is found in ACCESS table
        bool hasAccess2(const char * id);

        //prints returned values of database //www.sqlite.org/quickstart.html
        static int callback(void* data, int argc, char** argv, char** azColName) {
            int i;
            fprintf(stderr, "%s: ", (const char*)data);

            for (i = 0; i < argc; i++) {
                printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
            }

            printf("\n");
            return 0;
        }
        
    private:
        sqlite3* db;
        char* zErrMsg;
        int rc;
        const char* data;
};
