#include "../third-party/sqlite/sqlite3.h"
#include <stdio.h>

class db_access
{
	public:
        //constructor connects to database
        //if there is no such database, create it
        db_access() {
            
        } ;

        //destructor
        virtual ~db_access() {
            
        };

        //insert public key
        virtual bool insert_public_key(const char* table, int id, int data) = 0;

        //get public key
        virtual int get_public_key(const char* table, int id) = 0;

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
