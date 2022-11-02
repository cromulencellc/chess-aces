import string
import random
import json
from bs4 import BeautifulSoup

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

def getCookie(response):
    rh=json.dumps(response.headers.__dict__['_store'])
    rhj=json.loads(rh)
    sc_rhj=rhj['set-cookie']
    cookie=sc_rhj[1].split(';')[0]
    return cookie

def getCRSFToken(response):
    configJson=json.loads(response.text)
    return configJson['csrf_token']

def getMessage(response):
    configJson=json.loads(response.text)
    status=configJson['status']
    return status['message']

def getCRSFTokenSoup(response):
    soup = BeautifulSoup(response.text, "html.parser")
    return soup.find('input',attrs = {'name':'_csrf'})['value']

# def checkResult(response, expected_result, extrastring=""):
#     if response.status_code != expected_result:
#         print("Status Code:", str(response.status_code), 'vs expected', str(expected_result), '. Failure in', extrastring, flush=True)
#         print('[FAILED] POLLER Failed', flush=True)
#         exit(0)