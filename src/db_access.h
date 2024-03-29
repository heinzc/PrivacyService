#pragma once

#include <stdio.h>
#include <string>
#include <vector>

#include <QString>

class QSqlDatabase;

class db_access
{
	public:
        //constructor connects to database
        //if there is no such database, create it
        db_access(QString dbName);

        //destructor
        ~db_access();

        //to save own keys (public and private key) in database
        //bool insert_own_key(const char* keytype, const char* value);
        //bool insert_own_key(const char* keytype, std::string & key);
        bool insert_own_key(const QString& keytype, const QString& key);
        
        //used to retrieve own keys (including private key) from the database
        //std::string get_own_key(const char* keytype);
        QString get_own_key(const QString& keytype);
        
        ////insert public key
        //bool insert_public_key(const char* id, const char* key);
        //bool insert_public_key(const char* id, std::string & key);
        bool insert_public_key(const QString& id, const QString& pkJson);

        ////get public key, returns empty string if key is not in database!
        //std::string get_public_key(const char* id);
        QString get_public_key(const QString& id);

        ////check if id is in Access table (check if id has access to decrypt and data)
        ////bool hasAccess(const char* id);
        //
        ////check if id is found in ACCESS table
        ////bool hasAccess2(const char * id);
        //
        ////device of owner of this service
        //bool isOwnDevice(const char * id);
        //
        ////unencrypted device of encrypted device of owner of this service
        //std::string getPlainDeviceOfOwnEncryptedDevice(const char * id);
        //
        ////owners device, or trusted device
        //bool hasAccessToDecrypt(const char * id);
        bool hasAccessToDecrypt(const QString& id);
        
        ////check if data requester is allowed to data
        //bool hasAccessToData(const char * data_requester_oid, const char * destination_oid);
        bool hasAccessToData(const QString& data_requester_oid, const QString& destination_oid);
        
        ////get the corresponding privacy service oid of input oid
        //std::string getPrivacyService(const char * id);
        QString getPrivacyService(const QString& id);

        //has to be called after finished or aborted computation with the requester id (initiator oid)
        void deleteInitiatorRandomShares(const QString& initiatorOid);
        
        //update (insert) share
        void updateShareRandomShares(const QString& initiatorOid, const QString& participantOid, double value);
        
        //returns true, if we received the request for the computation and the participant was inserted
        bool isAlreadyInsertedInRandomShares(const QString& initiatorOid, const QString& participantOid);
        
        //returns true if all shares received (also true, if there are no entries at all)
        bool allRandomSharesReceived(const QString& initiatorOid);
        
        //if oid trusts this service (-> would send share in aggregation)
        bool trustsMe(const QString& oid);
        
        //if we allowed this oid to start distributed aggregation
        bool isTrustedInitiator(const QString& oid);

        //insert new participant of an aggregation. function automatically determines the needed participants (those who trust me apart from myself)
        //participants can be inserted multiple times, will only be saves once in database (for this aggregation)
        bool insertParticipantRandomShares(const QString& initiatorOid, const QString& participantOid);
        
        //insert participants of own aggregation into database
        bool insertParticipantBlindedMeasurements(const QString& participantOid);
        
        //update (insert) blinded measurement
        bool updateBlindedMeasurement(const QString& participantOid, double value);
        
        //if all shares received of own aggregation
        bool allBlindedMeasurementsReceived();
        
        //returns all trusted parties for distributed aggregation
        QStringList getTrustedParties();
        
        //returns true, if oid is a trusted party
        bool isTrustedParty(const QString& oid);
        
        //returns the sum of the blinded measurements -> the result of the aggregation
        double getBlindedMeasurementSum();
        
        //returns sum of received random shares of the aggregateion initiated by initiatorOid
        double getRandomShareSum(const QString& initiatorOid);

        
    private:
        QString m_dbName;
        QSqlDatabase* m_db;

        bool open();
        bool createTables();

        //clears the table
        bool resetRandomShares();

        //resets the complete table
        bool resetBlindedMeasurements();
};
