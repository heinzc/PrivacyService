import datetime
from flask import Flask, jsonify, request
from random import random
import random
import json
import requests

app = Flask(__name__)

date_time = datetime.datetime.now()
pid = 'heartrate'
@app.route('/properties/heartrate', methods = ['GET'])
def getvalue():
    hr_num = random.randint(57, 89)
    return jsonify({'heart rate': hr_num,'time': date_time})

@app.route('/objects/<oid>/properties/<pid>',methods = ['GET'])
def getvalue_bp(oid, pid):
    bp_num = random.uniform(96.3, 100.8)
    print(bp_num);
    return jsonify({'value': bp_num,'time': date_time})

@app.route('/objects/<oid>/properties/<pid>',methods = ['PUT'])
def putvalue_bp(oid, pid):
    record = json.loads(request.data)
    print(record);
    return jsonify(record)

@app.route('/objects',methods = ['GET'])
def thing_descriptor():
    td = {
        "adapter-id": "debugging-adapter",
        "thing-descriptions": [{
            "oid": "debug",
            "name": "Debug_Dummy_device",
            "type": "adapters:ActivityTracker",
            "properties": [{
                "pid": "test",
                "monitors": "adapters:HeartRate",
                "read_link": {
                    "href": "/objects/{oid}/properties/{pid}",
                    "output": {
                        "type": "object",
                        "field": [{
                            "name": "value",
                            "schema": {
                                "type": "integer"
                            }
                        }]
                    }
                }
            }
            ],

            "actions": [],
            "events": []
        }]
     }
    return json.dumps(td)
#    return jsonify(data=td, status=200)

if __name__ == '__main__':

    app.run(host = '0.0.0.0', debug =True)
	#adapter port = 5000
    #active discovery
    #requests.post(
    #    'http://localhost:9997/objects',
    #    data=thing_descriptor()
    #)