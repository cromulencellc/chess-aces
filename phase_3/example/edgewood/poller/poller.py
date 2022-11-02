#!/bin/python3

import requests
import datetime
from datetime import datetime as dt
import socket
import json
import time
import random
import sys
import os
import signal
import statistics


# The tempest weather plugin listing on udp port 50222
HOST = os.getenv("HOST", "127.0.0.1")
WEATHER_PORT = int(os.getenv("WEATHER_PORT", 50222))

# The main web server tcp port
HTTP_PORT = int(os.getenv("HTTP_PORT", 8080))

BASE_URI = 'http://{}:{}'.format(HOST, HTTP_PORT)

TIMEOUT = int(os.getenv('TIMEOUT', 10))

EXPECTED_TOKEN = os.getenv('EXPECTED_TOKEN', None)

def load_admin_data():
    try:
        ADMIN_DATA_PATH = os.getenv("ADMIN_DATA_PATH", "/data/admin.json")
        admin_file = open(ADMIN_DATA_PATH)
        admin_data = json.load(admin_file)
        admin_file.close()
        return admin_data
    except Exception as e:
        print("[ERROR] couldn't load admin data:")
        print(e)
        sys.exit(1)

ADMIN_DATA = load_admin_data()

def sig_handler( signum, frame):
    print('[FAIL] sigalrm')
    sys.exit(1)

def assert_equal(expected, got, message=None):
    if expected == got:
        return

    if not message:
        message = "assert_equal failed"
    print("expected {}".format(expected))
    print("got {}".format(got))
    raise AssertionError(message)

def get(url):
    try:
        return requests.get(BASE_URI + url)
    except:
        print("[FAIL] couldn't get {}".format(url))
        raise

def get_json(url):
    resp = get(url)
    try:
        return json.loads(resp.text)
    except:
        print("[FAIL] couldn't parse json from {}".format(url))
        print(resp.text)
        raise

def clean_database():

    # for doing things that require authentication
    admin_session = requests.Session()

    #first connect and clean out the database

    try:
        result = admin_session.post(BASE_URI+'/admin/login',
                            data={'userid': ADMIN_DATA["userid"],
                                  'password': ADMIN_DATA["password"]})
    except Exception as e:
        print(e)
        print("[FAIL] Could not connect to server")
        raise

    if 'login successful' not in result.text:

        print("[FAIL] Login as adminuser failed")
        return False

    result = admin_session.post(BASE_URI+'/admin/delete_all')

    if 'weather data deleted' not in result.text:

        print("[FAIL] Cleaning of database failed")
        return False

    return True
    # print(result.status_code, result.text)

def send_random_readings(starting_timestamp):

    clientSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    addr = (HOST, WEATHER_PORT)

    timestamp = starting_timestamp

    results = []

    for _ in range(0, 120):

        temperature = random.uniform(-40.0, 110.0)

        wind_speed = random.uniform(0.0, 35.0)

        wind_gust = wind_speed + random.uniform(0.0, 10.0)

        wind_direction = random.uniform(0.0, 359.99)

        humidity = random.uniform(10.0, 100.0)

        pressure = random.uniform(10.0, 100.0)

        readings = [timestamp, 0.0, wind_speed, wind_gust, wind_direction, 3.0, pressure,(temperature-32.0) *5/9, humidity, 0, 0, 0, .123]

        results += [readings]

        observation = {"serial_number":"ST-00099999",
                            "type":"obs_st",
                            "hub_sn":"HB-000000000",
                            "obs":[readings],
                            "firmware_revision":156}


        clientSocket.sendto(json.dumps(observation).encode(), addr)

        timestamp += 60

    time.sleep(2)
    return(results)


def update_minmax(readings, new_readings):
    max_temp = readings['_max_temp']
    max_wind = readings['_max_wind']

    for reading in new_readings:
        max_temp = max(max_temp, reading[7]*9/5 + 32.0)
        max_wind = max(max_wind, reading[2])

    readings['_max_temp'] = max_temp
    readings['_max_wind'] = max_wind

def pick_past_day(readings):
    now = dt.now()
    today = dt(now.year, now.month, now.day,
               now.hour % 12, 7, 0)
    days_past = random.randrange(2, 4 * LENGTH) # arbitrary
    past_day = today - datetime.timedelta(days = days_past)

    while past_day in readings:
        days_past = random.randrange(2, 10 * LENGTH) # bigger range
        past_day = today - datetime.timedelta(days = days_past)

    return past_day

def add_today(readings):
    now = dt.now()
    today = dt(now.year, now.month, now.day,
               now.hour % 12, 7, 0)
    if today in readings:
        print("[ERROR] tried to add today twice, internal poller error")
        sys.exit(1)

    new_readings = send_random_readings(today.timestamp())

    update_minmax(readings, new_readings)
    readings[today] = new_readings

def add_random_day(readings):
    day = pick_past_day(readings)
    day_ts = day.timestamp()

    new_readings = send_random_readings(day_ts)

    update_minmax(readings, new_readings)

    readings[day] = new_readings

def check_random_day(readings):
    day = random.choice(list(readings.keys()))
    while type(day) == str:
        day = random.choice(list(readings.keys()))

    day_readings = readings[day]

    result_json = get_json('/weather?date={:04}{:02}{:02}'.
                           format(day.year, day.month, day.day))

    temps = None

    for element in result_json:
        if element['type'] == 'temperature':
            temps = element['data']
            break

    if temps is None:
        print("[FAIL] Did not receive temperature data from query")
        sys.exit(1)

    for i in range(0,len(temps)):
        assert_equal(day_readings[i][7]*9/5 + 32.0,
                     temps[i]['temperature'])

    winds = None

    for element in result_json:
        if element['type'] == 'wind':
            winds = element['data']
            break

    if winds is None:
        print("[FAIL] Did not receive wind data from query")
        sys.exit(1)

def check_no_wind(readings):
    day = random.choice(list(readings.keys()))
    while type(day) == str:
        day = random.choice(list(readings.keys()))

    day_readings = readings[day]

    result_json = get_json('/weather?temperatures=1&date={:4}{:02}{:02}'.
                           format(day.year, day.month, day.day))

    winds = None

    for element in result_json:
        if element['type'] == 'wind':
            winds = element['data']
            break

    assert_equal(None, winds)

    temps = None

    for element in result_json:
        if element['type'] == 'temperature':
            temps = element['data']
            break

    if temps is None:
        print("[FAIL] Did not receive temperature data from query")
        sys.exit(1)


    temps_matched = True

    for i in range(0,len(temps)):
        assert_equal(day_readings[i][7]*9/5 + 32.0, temps[i]['temperature'])

def check_current_temperature(readings):
    now = dt.now()
    today = dt(now.year, now.month, now.day,
               now.hour % 12, 7, 0)
    todays_readings = readings[today]

    result_json = get_json('/weather/temperature')

    assert_equal(todays_readings[-1][7]*9/5 + 32.0, result_json['temperature'])

def check_current_wind(readings):
    now = dt.now()
    today = dt(now.year, now.month, now.day,
               now.hour % 12, 7, 0)
    todays_readings = readings[today]

    result_json = get_json('/weather/wind')

    assert_equal(todays_readings[-1][2], result_json['avg'])
    assert_equal(todays_readings[-1][3], result_json['gust'])

def check_minmax(readings):

    result_json = get_json('/weather/max_temperature')

    the_temperature = result_json['temperature']

    assert_equal(readings['_max_temp'], the_temperature)

    result_json = get_json('/weather/max_wind')

    the_max_avg_wind = result_json['avg']
    assert_equal(readings['_max_wind'], the_max_avg_wind)

def check_admin(_readings):

    # for doing things that require authentication
    admin_session = requests.Session()

    #first connect and clean out the database

    
    result = admin_session.post(BASE_URI+'/admin/login',
                            data={'userid': ADMIN_DATA["userid"],
                                  'password': ADMIN_DATA["password"]})
    

    if 'login successful' not in result.text:

        print("[FAIL] Login as adminuser failed")
        return False

    result_j = json.loads(result.text)

    if EXPECTED_TOKEN:
        assert_equal(EXPECTED_TOKEN, result_j['token'])
    else:
        print("[INFO] no `EXPECTED_TOKEN` env, but got " + result_j['token'])

    return True
    # print(result.status_code, result.text)

def main():

    # setup signal handling for a timeout feature
    signal.signal(signal.SIGALRM, sig_handler)
    signal.alarm(60)



    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('SEED={0}'.format(SEED))

    random.seed( SEED )

    maybe_length = random.randrange(50, 100)
    LENGTH = int(os.getenv('LENGTH', maybe_length))
    print('LENGTH={0}'.format(LENGTH))

    # need to empty the database so that find max values will work
    if not clean_database():

        sys.exit(1)

    # janky, but seems like mongodb doesn't always get the database saved by the next operation
    #time.sleep(1)

    readings = dict()
    readings['_max_temp'] = -1000
    readings['_max_wind'] = -1000

    timings = dict()
    counts = dict()

    add_today(readings)
    check_minmax(readings)

    for _l in range(LENGTH):
        activity = random.choice([add_random_day,
                                  check_minmax,
                                  check_random_day,
                                  check_no_wind,
                                  check_current_temperature,
                                  check_current_wind,
                                  check_admin])
        n = activity.__name__
        print(n)
        t = timings.get(n, [])
        c = counts.get(n, 0)
        signal.alarm(TIMEOUT)
        start = time.perf_counter()
        activity(readings)
        ended = time.perf_counter()
        signal.alarm(0)

        t.append(ended - start)
        timings[n] = t
        counts[n] = c + 1

    fs= "{:30}\t{}\t{:7}\t{:7}\t{:7}"
    f = "{:30}\t{}\t{:0.3}\t{:0.3}\t{:0.3}"

    print()
    print(fs.format("activity", "count", "tmean", "tstdd", "tmax"))
    for k in timings.keys():
        try:
            print(f.
                  format(k,
                         counts[k],
                         statistics.mean(timings[k]),
                         statistics.stdev(timings[k]),
                         max(timings[k])))
        except:
            print(fs.format(k, counts[k], 'x', 'x', 'x'))

    print("success!")

maybe_length = random.randrange(50, 100)
LENGTH = int(os.getenv('LENGTH', maybe_length))

if __name__ == '__main__':

    main()

# rapid_wind_template = {"serial_number":"ST-00099999",
#                         "type":"rapid_wind",
#                         "hub_sn":"HB-00021396",
#                         "ob":[1639846290,2.03,129]}

# hub_status_template = {"serial_number":"HB-00099999",
#                         "type":"hub_status",
#                         "firmware_revision":"171",
#                         "uptime":72684,
#                         "rssi":-56,
#                         "timestamp":1639846293,
#                         "reset_flags":"BOR,PIN,POR",
#                         "seq":7262,
#                         "fs":[1,0,15675411,524288],
#                         "radio_stats":[22,1,0,7,46173],
#                         "mqtt_stats":[3,15]}
