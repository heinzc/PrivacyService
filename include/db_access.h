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
        bool insert_own_key(const char* keytype, std::string & key);
        
        //used to retrieve own keys (including private key) from the database
        std::string get_own_key(const char* keytype);
        
        //insert public key
        bool insert_public_key(const char* id, const char* key);
        bool insert_public_key(const char* id, std::string & key);
        
        //get public key, returns empty string if key is not in database!
        std::string get_public_key(const char* id);
        
        //check if id is in Access table (check if id has access to decrypt and data)
        //bool hasAccess(const char* id);
        
        //check if id is found in ACCESS table
        //bool hasAccess2(const char * id);
        
        //device of owner of this service
        bool isOwnDevice(const char * id);
        
        //owners device, or trusted device
        bool hasAccessToDecrypt(const char * id);
        
        //check if data requester is allowed to data
        bool hasAccessToData(const char * data_requester_oid, const char * destination_oid);
        
        //get the corresponding privacy service oid of input oid
        std::string getPrivacyService(const char * id);
        
        //clears the table
        void resetAggregationComputation();
        
        //has to be called after finished or aborted computation with the requester id (source oid)
        void deleteSourceAggregationComputation(const char * sourceOid);
        
        //update (insert) share
        void updateShareAggregationComputation(const char * sourceOid, const char * participant, int value);
        
        //returns true if all shares received
        bool allSharesReceived(const char * sourceOid);
        
        //if oid trusts thisb service (-> would send share in aggregation)
        bool trustsMe(const char * oid);

        //to insert a participant for a new aggregation
        //participants can be inserted multiple times, will only be saves once in database (for this aggregation)
        void insertParticipantAggregateComputation(const char * sourceOid, const char * participant);
        
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
