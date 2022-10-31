HOST=''
PORT=0
PWNLIB_SILENT=1
POLLER_SUCCESS=True
POLLER_MODE = 2
    # Automated Tutorial = 1
    # Automated Players  = 2
    # User Controlled    = 3, ensure docker-compose.yaml is updated
NUMBER_OF_THREADS = 3 # maximum 10 due to VME limiting to 10 connections
DIRECTION_OPERATIONS = 7
PRINTSTATUS = True
PRINTUSER = False
PRINTRESPONSE = False
STATUS=0
USER=1
RESPONSE=2
go_ahead = b'\xff\xf9'
header = b'\xff\xfc\x01'
footer = b'\xff\xfb\x01\xff'
maxusers = 10
users = [
    "Alpha", "Bravo", "Charlie", "Delta",
    "Echo", "Foxtrot", "Golf", "Hotel",
    "India", "Juliet", "Kilo", "Lima",
    "Mike", "November", "Oscar", "Papa"
]
badpassword = "wontwork"
USER_STATUS = {
    "Alpha":0, "Bravo":0, "Charlie":0, "Delta":0, 
    "Echo":0, "Foxtrot":0, "Golf":0, "Hotel":0,
    "India":0, "Juliet":0, "Kilo":0, "Lima":0,
    "Mike":0, "November":0, "Oscar":0, "Papa":0
}
TIMESTOCHECKFORCREATURES=1