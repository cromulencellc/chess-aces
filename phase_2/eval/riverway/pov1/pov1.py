#!/usr/bin/python3

from websocket import create_connection
import os
import json

sc_config = {"cmd":"setconfig",
            "Kp":0.8,
            "Ki":0.1,
            "Kd":0.5,
            "ivelocity":-81,
            "fvelocity":-10,
            "ialtitude":1600,
            "faltitude":40,
            "fuel_max": 100,
            "thrust_max": 10000,
            "mpg" : 1000,

            }

sim_config = { "cmd":"setsimconfig",
                "sim_alt":9000,
                "sim_vel":-500,
                "planet":"mars",
                "logging": True,
            }

def do_sim(ws):

    ws.send(json.dumps(sc_config))
    ws.send(json.dumps(sim_config))

    ws.send('{"cmd":"start"}')

    while True:

        try:
            data = ws.recv()
            message = json.loads(data)

        except:
            return False
        

        if message['type'] == 'status':

            print("[INFO] Got the end of the simulation")
            break

def check_for_logs(ws):


    ws.send('{"cmd":"listlogs"}')

    try:
        data = ws.recv()
        response = json.loads(data)

    except:

        return False

    if response['type'] == 'loglist':

        print("[INFO] ...Got the list")

    else:

        return False

    if len(response['logs']) == 0:

        print("[ERROR] But the list is empty")
        return False

    return True

def main():

    if 'HOST' in os.environ and 'PORT' in os.environ:
        host = os.environ.get('HOST')
        port = os.environ.get('PORT')
    else:
        print('[ERROR] HOST and PORT environment variables are required')
        exit(-1)

    connect_string = "ws://{}:{}".format(host, port)

    try:
        ws = create_connection(connect_string, timeout = 15)

    except:
        print("[ERROR] Unable to open websocket to {}".format(connect_string))
        exit(-1)

    print("[INFO] Connected to the service")
    print("[INFO] Running a simulation")

    if do_sim(ws) == False:

        print("[ERROR] Simulation did not work")
        exit(-1)


    print("[INFO] Checking for a list of logs")

    if check_for_logs(ws) == False:
        print("[ERROR] Check for logs was unsuccessful")
        exit(-1)


    print("[INFO] Trying to retrieve the token file")
    ws.send('{"cmd":"sendlog", "name":"../../token"}')

    try:
        data = ws.recv()
        
    except:

        print("[FAIL] Did not get the token file")
        exit(-1)

    try:
        results = json.loads(data)['data']
        print('[SUCCESS] Token: ' + results)

    except:

        print("[FAIL] Unable to parse the response")
        exit(-1)



if __name__ == "__main__":
    main()
