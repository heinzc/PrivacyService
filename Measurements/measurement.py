import datetime
import time
from random import random
import random
import json
import requests

date_time = datetime.datetime.now()

remoteUrlEncrypted = 'http://localhost:9997/agent/remote/objects/6be12282-af0e-4f4d-b66c-2b5285629ffb/properties/test'
remoteUrlPlaintext = 'http://localhost:9997/agent/remote/objects/7591d50e-57d5-4ef1-bc13-c2a608f5759f/properties/test'
remoteUrlDirect = 'http://192.168.10.114:5000/objects/debug/properties/test'
remoteUrlDirectExtern = 'http://131.246.211.131:5000/objects/debug/properties/test'
getLocalDataUrl = 'http://localhost:4242/vicinity/objects/debugging-vas:debug_enc/properties/test'
aggregateLocalUrl = 'http://localhost:4242/aggregate'
decryptLocalUrl = 'http://localhost:4242/decrypt'
requestHeaders = {'infrastructure-id': 'debug', 'adapter-id': 'debugging-vas', 'Content-Type' : 'application/json'}
iterations = 100
sleepTime = 1
runtimeExecutionTime = 0
mintime = 100
maxtime = 0

def get_data_request():
    response = requests.get(
        getLocalDataUrl,
        headers=requestHeaders
    )

    json_response = response.json()

    return json_response["value"]

def aggregate_request(values):
    response = requests.post(
        aggregateLocalUrl,
        headers=requestHeaders,
        json={"values" : values}
    )

    json_response = response.json()

    return json_response["result"]

def decrypt_request(value):
    response = requests.post(
        decryptLocalUrl,
        headers=requestHeaders,
        json={"value": value}
    )

    json_response = response.json()

    return json_response["value"]


if __name__ == '__main__':
    print("starting measurement against api: " + getLocalDataUrl)
    startTime = time.time()

    ciphertexts = []

    for x in range(iterations):
        lastExecutionTime = time.time()
        ciphertexts.append(get_data_request())
        currentRequestTime = time.time() - lastExecutionTime

        print("execution #" + str(x) + " took: " + str(currentRequestTime) + " seconds.")
        runtimeExecutionTime += currentRequestTime
        mintime = min(mintime, currentRequestTime)
        maxtime = max(maxtime, currentRequestTime)
        print(str(mintime) + ", " + str(maxtime))

        time.sleep(sleepTime)

    executionTime = (time.time() - startTime)
    # as we do not care about the sleep times, subtract it again
    correctedExecutionTime = executionTime - (iterations * sleepTime)

    print("execution time over " + str(iterations) + " iterations took:")
    print("corrected Execution time: " +  str(correctedExecutionTime))
    print("sum of execution times: " + str(runtimeExecutionTime))
    print("maxtime: " + str(maxtime))
    print("mintime: " + str(mintime))

    print("got " + str(len(ciphertexts)) + " elements")

    for x in range(10):
        aggregationStartTime = time.time()
        aggregatedResult = aggregate_request(ciphertexts)
        aggregationTime = (time.time() - aggregationStartTime)
        print("aggregation took: " + str(aggregationTime))

        decryptionStartTime = time.time()
        decryptedResult = decrypt_request(aggregatedResult)
        decryptionTime = (time.time() - decryptionStartTime)
        print("decryption took: " + str(decryptionTime))
        print("decryption result is: " + str(decryptedResult))