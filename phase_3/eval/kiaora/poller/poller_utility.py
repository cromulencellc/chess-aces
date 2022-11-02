from operator import truediv
import string
import random

import poller_config

def getRandomNumber(start, end):
    return random.randint(start, end)

def hasNumbers(inputString):
    return any(char.isdigit() for char in inputString)

def getRandomString(asciiNum, length):
    letters=string.ascii_letters
    checkNumbers=True
    if asciiNum == 1:
        numbers=string.digits
        letters=letters+numbers
        checkNumbers=False
    result=''.join(random.choice(letters) for i in range(length))
    while hasNumbers(result)==checkNumbers:
        result=''.join(random.choice(letters) for i in range(length))
    return result

def findinmessage(haystack, needle):
    if isinstance(haystack, bytes):
        return(haystack.decode('utf-8').lower().find(needle))
    if isinstance(haystack, str):
        return(haystack.lower().find(needle))      
    return -1

def printMsg(message, fromwhom=0):
    if poller_config.PRINTSTATUS and fromwhom == 0:
        print(message, flush=True)
    if poller_config.PRINTUSER == True and fromwhom == 1:
        print(message, flush=True)
    if poller_config.PRINTRESPONSE == True and fromwhom == 2:
        print(message, flush=True)