#pragma once

#include <iostream>
#include <map>

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
        std::string writeProperty(std::string oid, std::string aid, std::string payload);
        std::string postAction(std::string oid, std::string aid, std::string payload, std::string sender);
        
        std::string getOwnOid();
        std::string getAdapterId();
        std::string getAgentPort();
        std::string getOwnPort();

        std::string decrypt(std::string oid, std::string sourceOid, std::string payload);
        std::string enterAggregation(std::string oid, std::string sourceOid, std::string payload);
        std::string sendShareAction(std::string oid, std::string sourceOid, std::string payload);
        
        std::string getPublicKey(std::string oid);
        
        //std::string sendShare(std::string destinationOid, int shareValue);

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
