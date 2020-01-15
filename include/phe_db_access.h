#include "db_access.h"
#include "../third-party/sqlite/sqlite3.h"

class phe_db_access : db_access
{
	public:
        //constructor connects to database
        //if there is no such database, create it
        phe_db_access(const char* database);

        //destructor
        ~phe_db_access();

        //insert public key
        bool insert_public_key(const char* table, int id, int data);

        //get public key
        int get_public_key(const char* table, int id);
        
    private:
        sqlite3* db;
        char* zErrMsg;
        int rc;
        const char* data;
};
