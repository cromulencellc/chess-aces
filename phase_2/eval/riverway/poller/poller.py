#!/usr/bin/python3

from websocket import create_connection
import json
import random
import os
import copy
import sys

baseconfig = {"cmd":"setconfig",
            "Kp":0.6,
            "Ki":0.1,
            "Kd":0.5,
            "ivelocity":-81,
            "fvelocity":-10,
            "ialtitude":1600,
            "faltitude":40,
            "fuel_max": 65000,
            "thrust_max": 3216,
            "mpg" : 1576,
            }

base_sim_config = {  "cmd":"setsimconfig",
                    "sim_alt": 9000,
                    "sim_vel": -500.0,
                    "planet": 'mars',
                    "logging": False,
                }


sc_config = ["Kp", "Ki", "Kd", "ivelocity", "fvelocity", "ialtitude", "faltitude"]
sim_config = [ "sim_alt", "sim_vel", "planet", "logging"]

def random_sim_config(a):

    copy2 = copy.deepcopy(a)

    for element in ["sim_alt","sim_vel"]:
        
        copy2[element] = random.randint(0, 35000)

    copy2['planet'] = random.choice(['mars', 'earth'])

    copy2['logging'] = random.choice([True, False])

    return copy2


def random_config(a):

    copy2 = copy.deepcopy(a)

    for element in sc_config:

        copy2[element] = random.randint(0, 35000)

    return copy2


def compare_config(a, b):

    for element in sc_config:

        try:

            if a[element] != b[element]:

                return False
        except:

            return False


    return True

def compare_sim_config(a, b):

    if a['sim_alt'] != b['sim_alt']:
        return False

    if a['sim_vel'] != b['sim_vel']:
        return False

    if a['logging'] != b['logging']:
        print("[ERROR] bad logging value")
        return False

    if a['selected_planet'] != b['planet']:
        print("bad planet value")
        return False

    return True
    
def send_and_check_sim_config(ws):

    print("[TEST] Checking for sim config setting and getting")

    print("[INFO] ...Generating and sending a random configuration")
    sentconfig = random_sim_config(base_sim_config)

    ws.send(json.dumps(sentconfig))

    print("[INFO] ...Requesting the current configuration")
    ws.send('{"cmd": "getsimconfig"}')

    data = ws.recv()

    response = json.loads(data)

    print("[INFO] ...Comparing what was sent with what was received")

    if compare_sim_config(response, sentconfig):

        print("[INFO] ...Configs are the same")
        return True

    else:

        print("[ERROR] ...The configs are not the same")
        return False

def send_and_check_sc_config(ws):

    print("[TEST] Checking for config setting and getting")

    print("[INFO] ...Generating and sending a random configuration")
    sentconfig = random_sim_config(baseconfig)
    ws.send(json.dumps(sentconfig))

    print("[INFO]...Requesting the current configuration")
    ws.send('{"cmd": "getconfig"}')

    data = ws.recv()

    response = json.loads(data)

    print("[INFO]...Comparing what was sent with what was received")

    if compare_config(sentconfig, response):

        print("[INFO] ...Configs are the same")
        return True

    else:

        print("[ERROR] ...The configs are not the same")
        return False



def check_for_logs(ws):

    print("[TEST] Checking for a list of logs")

    ws.send('{"cmd":"listlogs"}')

    data = ws.recv()

    response = json.loads(data)

    if response['type'] == 'loglist':

        print("[INFO] ...Got the list")
        return True

    else:

        return False


def get_a_log(ws):

    print("[TEST] Retrieving a random log")

    ws.send('{"cmd":"listlogs"}')

    data = ws.recv()

    response = json.loads(data)

    logcount = len(response['logs'])

    if logcount == 0:

        print("[INFO] ...There currently aren't any and this is ok")
        return

    choice = random.randint(0, logcount -1)

    logname = response['logs'][choice]

    command = {"cmd":"sendlog", "name": logname}

    ws.send(json.dumps(command))

    data = ws.recv()

    response = json.loads(data)

    if response['type'] == 'logdata':

        print("[INFO] ...Received a log")
        return True
    
    else:

        return False


def do_good_sim(ws):

    print("[TEST] Doing a good sim")

    newconfig = copy.deepcopy(baseconfig)

    newconfig['ivelocity'] = random.randint(-85, -78)

    ws.send(json.dumps(newconfig))
    ws.send(json.dumps(base_sim_config))


    ws.send('{"cmd":"start"}')

    while True:

        try:
            data = ws.recv()
        except:
            break
        
        message = json.loads(data)

        if message['type'] == 'status':

            if message['status'] == 'success':

                print("[INFO] ...This was a success, as planned")
                return True

            else:

                print("[ERROR] ...This was a failure, and wasn't supposed to be")
                return False
            break

def do_bad_sim(ws):

    print("[TEST] Doing a bad sim")

    # set normal values for the simulation settings
    ws.send(json.dumps(base_sim_config))

    # send some bad-ish values for the spacecraft settings
    newconfig = copy.deepcopy(baseconfig)
    newconfig['Kp'] = random.randint(1, 9)
    newconfig['ivelocity'] = random.randint(70, 75)

    ws.send(json.dumps(newconfig))

    ws.send('{"cmd":"start"}')

    while True:

        try:
            data = ws.recv()
        except:
            break
        
        message = json.loads(data)
        
        if message['type'] == 'status':

            if message['status'] == 'success':

                print("[ERROR] ...This was a success and shouldn't be")
                return False

            else:

                print("[INFO] ...This was a failure, as planned")
                return True


def check_logging(ws):

    print("[TEST] Checking on logging config")

    ws.send('{"cmd":"listlogs"}')

    data = ws.recv()

    response = json.loads(data)

    original_logcount = len(response['logs'])

    new_sim_config = copy.deepcopy(base_sim_config)

    new_sim_config['logging'] = False

    ws.send(json.dumps(new_sim_config))

    ws.send(json.dumps(baseconfig))

    ws.send('{"cmd":"start"}')

    while True:

        try:
            data = ws.recv()
        except:
            break
        
        message = json.loads(data)
        
        if message['type'] == 'status':

            break

    ws.send('{"cmd":"listlogs"}')

    data = ws.recv()

    response = json.loads(data)

    new_logcount = len(response['logs'])

    if new_logcount != original_logcount:

        print("[ERROR] Logging wasn't supposed to happen!")
        return False

    else:

        print("[INFO] Logging was suppresed as planned")


    new_sim_config['logging'] = True

    ws.send(json.dumps(new_sim_config))

    ws.send('{"cmd":"start"}')

    while True:

        try:
            data = ws.recv()
        except:
            break
        
        message = json.loads(data)
       
        if message['type'] == 'status':

            break

    ws.send('{"cmd":"listlogs"}')

    data = ws.recv()

    response = json.loads(data)

    new_logcount = len(response['logs'])

    if new_logcount != original_logcount + 1:

        print("[ERROR] Logging was supposed to happen!")
        return False

    else:
        print("[INFO] Logging worked as planned")

    return True

def main():

    if 'HOST' in os.environ and 'PORT' in os.environ:
        host = os.environ.get('HOST')
        port = os.environ.get('PORT')
    else:
        print('[ERROR] HOST and PORT environment variables are required')
        sys.exit(-1)

    seed = os.environ.get('SEED', random.randint(0, 2**64- 1))

    print('[INFO] SEED: {}'.format(seed))

    length = int(os.environ.get('LENGTH', '500'))

    #900 iterations takes roughly a minute
    length = min(900, length)

    random.seed(seed)

    connect_string = "ws://{}:{}".format(host, port)

    # print(connect_string)

    try:
        ws = create_connection(connect_string)
    except:
        print('[ERROR] Unable to open websocket to {}'.format(connect_string))
        sys.exit(-1)

    functions = [ send_and_check_sc_config,
                    send_and_check_sim_config,
                    check_logging,
                    get_a_log,
                    check_for_logs,
                    do_good_sim,
                    do_bad_sim,
                    get_a_log ]

    for _ in range(0, length):

        function = random.choice(functions)

        if function(ws) == False:

            print("[FAIL] Failed poll!")
            exit(-1)

    print("[SUCCESS] Successful poll!")


if __name__ == "__main__":
    main()

