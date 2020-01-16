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

        //insert public key
        bool insert_public_key(int id, const char* key);

        //get public key
        int get_public_key(int id);

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
