# homomorphic-enc / privacy service

This homomorphic encryption prototype or privacy service can be used to securely aggregate data of different people, without publishing someones private data. You an also aggregate homomorphic encrypted data locally, share public keys automatically and wrap devices so you also get an encrypted device in VICINITY. You can encrypt and decrypt your data homomorphically and let other people decrypt data encrypted with your key with your service, if you granted access. Other people can also use this service to check if other people can have access to your shared, encrypted data.
Thanks to the homomorphic encryption, it is possible to e.g. store and process your data in an untrusted cloud.

Using SQlite (included; public domain).

## Table of contents
- Required Debian packages
- Building
- Workin examples
- Usage and features
- Program parts
- Configuration and setup
- Remarks

## Required Debian packages:
cmake
libgmp-dev
libboost-all-dev
libssl-dev
libcpprest-dev
libsqlite3-dev

## Building
To build this, the packages above need to be installed and everything in the third-party folder also needs to be installed respectively available.

Then, cd into the root folder of he project. Execute the following commands.

	cmake .
	make
The executable should now be found in the root of the project folder.

## Working examples
Two working examples using the privacy service (homomorphic encryption prototype) can be found [here](https://cpsgit.informatik.uni-kl.de/students/thesis_dennis). The first example shows, how your data can be processed in an untrusted cloud and how you can share it with other people. The second one shows how to use the distributed aggregation.

## Usage and features
The privacy service acts as an adapter for the VICINITY network and has to be called like every other adapter. There are several features that you can also see in the Neighbourhood Manager, but some of them are not meant to be used manually and are only meant to be used by the service itself. This is described below. More specific details than here can be found in the *ServiceThingDescription.json* or in the program code itself. In addition, the working examples can also help you.
### Properties
It is discussed [here](https://vicinityh2020.github.io/vicinity-gateway-api/#/actions/getObjectsOidActionsAidTasksTid), how to get properties, and [here](https://vicinityh2020.github.io/vicinity-gateway-api/#/actions/getObjectsOidActionsAidTasksTid) how to put properties.
#### trustedparties
Returns a list of all parties, which are trusted parties for an aggregation. If you want to start an aggregation with another party, you need to make sure at least one of a participants trusted parties is also part of the aggregation.

#### publickey
Returns the service's public key. You do not need to call this yourself, as the privacy service does this automatically for you. The public key is returned in the following format.

	{
		"publickey" : "qwertzuiop"
	}
For *seal_handler*, the public key also contains the parameters. This is important, if you need to work with data encrypted with different parameters. 

#### hasaccess
You can ask someone's privacy service, if someone has access to their encrypted data of a specific object. If you want to share someones data with somebody else, you should call this property, to check, if you are allowed to do so. This property cannot be requested using GET, you have to use PUT and provide the payload in the following format.

	{
		"requester-id" : "whoWantsTheData",
		"destination-id" : "whichIsTheOriginOfTheData"
	}
The *requester-id* is the oid of the object, that requests data or with which oid you want to share encrypted data.  Enter the oid of the object, where the data is from (its origin) in *destination-id*.

### Actions
To get the return value of the action or check the status, check [this](https://vicinityh2020.github.io/vicinity-gateway-api/#/actions/getObjectsOidActionsAidTasksTid) guide. To execute an action, see [this](https://vicinityh2020.github.io/vicinity-gateway-api/#/actions/postObjectsOidActionsAid).
#### Aggregation
To start an aggregation, start this action. This can only be called from your own devices. Returns the aggregate, if successful.

Input:

	{
		"participants": ["privacyOid1","privacyOid2","privacyOid3"],
		"devices" : ["deviceOid1","deviceOid2","deviceOid3"],
		"properties" : ["property1","property2","property3"]
	}
In *participants*, add the oids of the privacy services of the people, who shall be part of your aggregation. In *devices*, the oids of encrypted devices of the corresponding participant and in *properties*, the property of that object. You can have the same property, device or participant twice by duplicating the column.

Output:

	{
		"value" : 1337
	}
The value *value* represents the final aggregate.

#### participateInAggregation
This is called automatically by a privacy service, which started an aggregation. This action is used to ask a privacy service to participate in it. Returns the blinded measurement, if input is all right and requester is allowed to aggregate. Only trusted initiators and own devices are allowed to call this.

Input:

	{
		"participants": ["you","privacyOid2","privacyOid3"],
		"devices" : ["deviceOid1","deviceOid2","deviceOid3"],
		"properties" : ["property1","property2","property3"]
	}
This is pretty similar to the last action, however, the oid of the receiver of this request has to be replaced in *participants* by *you*. The reason of this is, that the privacy services do not know their own oid.

Output:

	{
		"value" : 1337
	}
The value *value* represents the *Blinded Measurement*.

#### Randomshare
This action is called automatically by the privacy service and is used to send a trusted party a share. There is no need to call this action yourself. Only the devices of the tables *AGGREGATION_PARTIES_WHO_TRUST_ME* and *OWN_DEVICES* are allowed to send you a share.
The share is sent in the following format:

	{
		"initiator" : "123",
		"share": 1337
	}
The *initiator* is the oid of the privacy service, which started the aggregation. The second value is used to store the share. The action's output is shown below, which is not needed at the moment, but is required.

	{
		"declined" : false
	}


#### Decrypt
If the requester is allowed to decrypt, the decrypted result is returned. In the input, insert the encrypted value, which you want to be decrypted. In the output, you can find the decrypted value.

Input:

	{
		"value" : "encryptedValue1337"
	}
Output:

	{
		"value" : 1337
	}


### Local features
#### aggregate
The local aggregation can only be called locally, which means you need to send a POST request to e.g. the following endpoint. The result is found in the response payload directly. 

	http://localhost:4242/aggregate
The payload must contain the values in the following format. You can add any amount of values.

    "values" : [
        "encryptedValue1",
        "encryptedValue2"
    ]
If you want to aggregate data, which was encrypted using the keys of this (your) privacy service, you did configure this request correctly.
However, if you want to aggregate data of someone else, you need to provide the oid of the object, where the data is from. You provide this oid in the *sourceOid* header of the request.

#### encrypt
The payload must directly contain the plain value you want to encrypt. The result is found in the response payload directly. Send to this endpoint, but with your port, of course.
	
	http://localhost:4242/encrypt
Sample payload:

	1337

#### decrypt
The payload must directly contain the cyphertext to be decrypted. The result is found in the response payload directly. Call the following endpoint.

	http://localhost:4242/decrypt
Sample payload:

	cyphertext1337


## Program parts
The program is divided into several parts, most of which are discussed here.
### Main
This is called when the program starts. It initializes all the needed classes, and outputs some test outputs. One of the things you should see, would be the sum of $71 + 71$, which should be printed like this:

	decrypted test result: 142
If it is not this result, then of course something is wrong.

### DB_Access
All database requests are handled by this class. There are functions for everything needed, e.g. to check if a given oid is a trusted party. It connects to the database *service.de*. It uses SQLite and is written for SQLite built with the multithreading option. When the program runs for the first time, the database including all tables is created. All functions in the code are commented, please look there for all details.
### Rest handler
All incoming requests are handled here using [Microsoft's cpprest](https://github.com/microsoft/cpprestsdk). It separates between different kinds of requests (e.g. POST, GET, ...) and then checks the path to know, what is requested. Local requests are directly processed and the result is returned. In case it was a request via VICINITY, information is extracted and then forwarded to the VICINITY_handler, which handles those requests. The rest_handler directly returns property request responses (the value is calculated by the vicinity_handler, however), but actions are just set to *running*. 
### VICINITY_handler
Generates a thing description of all adapters in *adapter_config.json* and adds its own. However, it does modify all devices, so they are then in encrypted form in the new one. The privacy service acts as a wrapper for the encryption.
Also, all VICINITY related requests are in some kind handled here. For properties, the correct return value is estimated. If a property of an encrypted device was requested, the plain value is fetched, encrypted and returned. Actions return here in the end the status *finished* or *failed*.
The public key of other privacy services is requested here. It is also useful to get the status of a task of an action or its result value.
### he_handler
Provides functions for encryption, decrypting, aggregation and others for a homomorphic encryption. The header is implemented by *phe_handler*, *fhe_handler* and *seal_handler*, which all use a different library for homomorphic encryption. The privacy service was written in a way, so you can choose which he_handler implementation you want. Just change it in *main.cpp*.
### he_controller
It saves references to different other classes, like *db_access*, *vicinity_hander* or an *he_handler*. These references can be provided, if needed.


## Configuration and setup
### Add privacy service in agent
For the agent, you usually add the adapters in a config data. To add this privacy service, you need to get the address right. The endpoint must contain the */vicinity* part, because all VICINITY related requests must have this in their path.

	{
        "adapter-id": "privacy-service-adapter",
	    "endpoint": "http://localhost:4242/vicinity"
    }

### Service.db
This database is created using SQLite. You can alter or add entries in the tables to set the privacy service up. In the following table, it is described what you can (or should) modify. Tables which should not be modified are maintained by the program itself. Changing them can lead to errors or wrong outputs.

| Table name | Modify? | Content |
|--|--|--|
| AGGREGATION_BLINDED_MEASUREMENTS | NO | Blinded measurements are saved here when aggregating
| AGGREGATION_PARTIES_WHO_TRUST_ME | **YES** | Insert oids of all privacy service, which were shared with your own privacy service. They can send you shares (Own Devices can as well)
| AGGREGATION_RADOM_SHARES | NO | Random shares are saved here when aggregating
| AGGREGATION_TRUSTED_INITIATORS | **YES** | Insert all oids of objects, which are allowed to start the aggregation. Own Devices are allowed to do this, no need to enter them.
| AGGREGATION_TRUSTED_PARTIES | **YES** | Add all oids of foreign privacy services, who you trust. You must be subscribed to those services!
| DATA_ACCESS | **YES** | Insert oids who are allowed to get your data e.g. from a foreign Value Added Service which can share your data. This service has to check these oids. 
| FOREIGN_DEVICES_ACCESS_DECRYPT | **YES** | Insert oids of devices, who can decrypt data encrypted with your public key. No need to insert your own devices. 
| OWN_DEVICES | **YES** | Insert all of your own devices (which are fully trusted, e.g. the devices of your organisation). Needed to check if aggregation and participateInAggregation request is legal. They have access to decrypt as well and can send you random shares.
| OWN_KEYS | NO | Here are your public and private key saved. Not encrypted! 
| PRIVACY_SERVICE | **YES** | Insert first oid of an object and second the oid of the corresponding privacy service. Needed to map both. This way we know, which privacy service created which encrypted device.
| PUBLIC_KEYS | NO | Here are the public keys of other privacy services saved. You could modify this, but there is no need, as the keys are fetched automatically. |

### ServiceThingDescription.json
This file is in the needed format to make the privacy service available in VICINITY. All properties and actions are described in here and if you don't want to offer some of them, you can just delete them out of this file.
But there are three settings you should consider to change:

| Item | Description |
| -- | -- |
| port | Port of the adapter where the privacy service will listen for requests. The agent will communicate over this port. Useful when several privacy services on one machine are needed. |
| agent-port | Port of your locally running agent. |
| adapter-id | Name of the adapter of this privacy service. |

### config_adapters.json
In this JSON file, add all adapters, where there are objects, you want to be duplicated in encrypted form by your privacy service. You can add more in *adapters*. 

    {
		"adapters":  [
			{
				"adapter-id": "debugging-adapter",
				"endpoint": "http://localhost:5000"
			}
		]
	}

## Remarks
With the current settings you cannot add arbitrarily big numbers, because of two reasons. Using *seal_handler*, the maximal representable value is limited by the *plain modulus*, which you can increase in the *initialize()* method in *seal_handler*, if needed. Also, for every random share r holds $r \in [0,100000)$. Because of this, too big numbers cannot be fully hidden. This can also be altered in *seal_handler* as well, if needed.

At the moment, the current implementation of the *seal_handler* requires the BFV scheme. Other schemes might work, but are not tested.
