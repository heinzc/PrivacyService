import datetime
from flask import Flask, jsonify
from random import random
import random
import json
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
    return jsonify({'body temperature': bp_num,'time': date_time})

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

    app.run(host = 'localhost', debug =True)
	#adapter port = 5000