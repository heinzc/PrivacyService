#pragma once

#include <iostream>
#include <map>
#include <set>

#include "stdafx.h"

#include <nlohmann/json.hpp>

#include "he_controller.h"

using namespace std;

class vicinity_handler
{
    public:
        vicinity_handler();
        virtual ~vicinity_handler();

        void setController(he_controller * controller) {
            m_pController = controller;
        }

        void initialize(std::string configfile);

        std::string generateThingDescription();

        std::string readProperty(std::string oid, std::string pid);
        //needed for distributed aggregation -> get plain property of encrypted device
        long readPlainProperty(std::string oid, std::string pid);
        std::string writeProperty(std::string oid, std::string aid, std::string payload);
        //when an action request is received, this redirects it to the correct function
        void postAction(std::string oid, std::string aid, std::string payload, std::string sourceOid);
        
        std::string getOwnOid();
        std::string getAdapterId();
        std::string getAgentPort();
        std::string getOwnPort();

        void decrypt(std::string oid, std::string sourceOid, std::string payload);
        void startAggregation(std::string oid, std::string sourceOid, std::string payload);
        void sendShareAction(std::string oid, std::string sourceOid, std::string payload);
        void participateInAggregation(std::string oid, std::string sourceOid, std::string payload);
        
        //returns task status; payload  must contain properties, participants, devices
        std::string askToParticipateInAggregation(std::string destinationOid, std::string payload);
        
        //returns status
        std::string getStatusOfAction(std::string destinationOid, std::string action, std::string taskId);
        //returns return value
        std::string getReturnValueOfAction(std::string destinationOid, std::string action, std::string taskId);
        
        std::string getPublicKey(std::string oid);
        
        std::string sendRandomShare(std::string destinationOid, std::string initiatorOid, int randomShare);
        
        std::string replaceAll(std::string str, const std::string from, const std::string to);
        
        long getPropertyOfEncryptedDevice(std::string encryptedDeviceOid, std::string pid);
        
        //deltes a task, returns true if successful, else false
        bool deleteTask(std::string destinationOid, std::string action, std::string taskId);

    protected:

    private:
        he_controller * m_pController;
        nlohmann::json m_configFile;
        std::map<std::string, std::string> m_endpoints;
        
        bool updateTaskStatus(std::string aid, std::string status, std::string payload);
        
        std::string port;
        std::string agentPort;
        std::string ownOid;
        std::string adapterId;
};
