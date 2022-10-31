from dis import dis
import time
import poller_config
import poller_utility

def recvdata(conn, delay=2):
    if conn is None:
        return
    footerfound = False
    recvd = conn.recvuntil(poller_config.go_ahead, timeout=delay)
    if not recvd:
        return
    if recvd.find(poller_config.header) >= 0:
        recvd = recvd[recvd.find(poller_config.header) + 5:]
    if recvd.find(poller_config.footer) >= 0:
        footerfound = True
        recvd = recvd[:recvd.find(poller_config.footer)]
    if recvd.find(poller_config.go_ahead) >= 0 and not footerfound:
        recvd = recvd[:recvd.find(poller_config.go_ahead)]
    return recvd.decode('utf-8')

def senddata(conn, data, display=1):
    if data is not None:
        if display == 1:
            message='User('+data.replace('\n', ' ').replace('\r', '')+ ')'
            poller_utility.printMsg(message, poller_config.USER)
        conn.sendline(data.encode('utf-8'))

def sendSingleMessage(conn, message):
    time.sleep(1)
    response=recvdata(conn)
    if response is not None:
        response=response.replace('\n', ' ').replace('\r', '')
        poller_utility.printMsg(response, poller_config.RESPONSE)
    senddata(conn, message)
    response=recvdata(conn)
    if response is not None:
        response=response.replace('\n', ' ').replace('\r', '')
        poller_utility.printMsg(response, poller_config.RESPONSE)

def sendSingleMessageNoResp(conn, message):
    time.sleep(1)
    response=recvdata(conn)
    if response is not None:
        response=response.replace('\n', ' ').replace('\r', '')
        poller_utility.printMsg(response, poller_config.RESPONSE)
    senddata(conn, message)

def getMessage(conn,clean=1):
    tries=10
    while tries > 0:
        time.sleep(1)
        response=recvdata(conn)
        if response is not None:
            tries = 10
            if clean == 1:
                response=response.replace('\n', ' ').replace('\r', '')
                poller_utility.printMsg(response, poller_config.RESPONSE)
            return response
        tries-=1
    return -1