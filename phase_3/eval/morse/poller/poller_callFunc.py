from pwn import *
import requests

import poller_api
import poller_config
import poller_utility

from poller_socketio import *
import time
import json

def doPing():
    c_session = requests.Session()
    headers=poller_config.headers
    result=poller_api.getPing(c_session, headers)
    poller_utility.checkResult(result, 200)
    print("Ping Server Completed", flush=True)
    c_session.close()

def doFailLogin():
    c_session = requests.Session()
    headers=poller_config.headers
    ## GET FORUM CONFIGURATION
    #print("GET Config:", flush=True)
    result=poller_api.getConfig(c_session, headers)
    poller_utility.checkResult(result, 200)
    headers['x-csrf-token']=poller_utility.getCRSFToken(result)
    ## USER LOGIN
    user=poller_utility.getRandomUser()
    #print("POST Login (", user, ")", flush=True)
    result=poller_api.failLogin(c_session, user, headers)
    poller_utility.checkResult(result, 403, user)
    print("Successful Failed Login:", user, flush=True)
    c_session.close()

def doSuccessfulLogin():
    c_session = requests.Session()
    headers=poller_config.headers
    ## GET FORUM CONFIGURATION
    #print("GET Config:", flush=True)
    result=poller_api.getConfig(c_session, headers)
    poller_utility.checkResult(result, 200)
    headers['x-csrf-token']=poller_utility.getCRSFToken(result)
    ## SUCCESSFUL USER LOGIN
    user=poller_utility.getRandomUser()
    #print("POST Login (", user, ")", flush=True)
    result=poller_api.doLogin(c_session, user, poller_utility.getUserPassword(user), headers)
    poller_utility.checkResult(result, 200, user)
    print("Successful Login:", user, flush=True)
    c_session.close()

def getMaxNumberOfTopics():
    resulttext=""
    while resulttext != '{"status":{"code":"ok","message":"OK"},"response":{}}':
        poller_config.maxNumTopics+=1
        c_session = requests.Session()
        headers=poller_config.headers
        result=poller_api.getTopic(c_session, poller_config.maxNumTopics, headers)
        poller_utility.checkResult(result, 200, 'Get Max Number of Topics')
        resulttext=result.text
        #print(poller_config.maxNumTopics, result.text, flush=True)
        c_session.close()
    poller_config.maxNumTopics-=1
    print("Determined there are", poller_config.maxNumTopics, "topics.")

def getMaxNumberOfPosts():
    resulttext=""
    while resulttext != '{"status":{"code":"ok","message":"OK"},"response":{}}':
        poller_config.maxNumPosts+=1
        c_session = requests.Session()
        headers=poller_config.headers
        result=poller_api.getPost(c_session, poller_config.maxNumPosts, headers)
        poller_utility.checkResult(result, 200, 'Get Max Number of Posts')
        resulttext=result.text
        #print(poller_config.maxNumTopics, result.text, flush=True)
        c_session.close()
    poller_config.maxNumPosts-=1
    print("Determined there are", poller_config.maxNumPosts, "posts.")

### API WORK
def createGroup(c_session, user, headers):
    groupName=poller_utility.getRandomString(12, 16)
    data = {
        "name": groupName,
        "timestamp": 0,
        "disableJoinRequests": 0,
        "disableLeave": 0,
        "hidden": 0,
        "ownerUid": 0,
        "private": 0,
        "description": "Creating a group for test",
        "userTitleEnabled": 0,
        "createtime": 0,
        "memberPostCids": [
            [
                1,
                2,
                3
            ]
        ]
    }
    result=poller_api.postCreateGroup(c_session, data, headers)
    message=":createGroup"+groupName
    poller_utility.checkResult(result, 200, message)
    print("  Created group:", groupName, flush=True)

def leaveJoinGroup(c_session, user, headers):
    result=poller_api.deleteLeaveGroup(c_session,"users",poller_config.uid[user],headers)
    poller_utility.checkResult(result, 200, "leaveGroup")
    print(" ", user, "left users group", flush=True)
    result=poller_api.putJoinGroup(c_session,"users",poller_config.uid[user],headers)
    poller_utility.checkResult(result, 200, "joinGroup")
    print(" ", user, "joined users group", flush=True)

def createTopic(c_session, user, headers):
    categoryNum=poller_utility.getRandomNumber(1,4)
    data = {
        "cid": categoryNum,
        "title": poller_utility.getRandomString(0, poller_utility.getRandomNumber(10,20)),
        "content": poller_utility.getRandomString(0, poller_utility.getRandomNumber(30,100)),
        "tags": [
            user,
           possibleTags[poller_utility.getRandomNumber(0,len(possibleTags)-1)]
        ]
    }
    result=poller_api.postCreateTopic(c_session, data, headers)
    message=":createTopic-"+str(categoryNum)
    poller_utility.checkResult(result, 200, message)
    print("  Create topic in category", categoryNum, flush=True)
    poller_config.maxNumTopics+=1

def replyTopic(c_session, user, headers):
    data = {
        "content": poller_utility.getRandomString(0, poller_utility.getRandomNumber(30,100)),
        "timestamp": 0,
        "toPid": 0
    }
    topicNumber=poller_utility.getRandomNumber(1,poller_config.maxNumTopics)
    result=poller_api.postReplyTopic(c_session, topicNumber, data, headers)
    message=":replyTopic-"+str(topicNumber)
    poller_utility.checkResult(result, 200, message)
    print("  Reply to topic: ", topicNumber, flush=True)
    poller_config.maxNumPosts+=1

def votePost(c_session, user, headers):
    delta=poller_utility.getRandomNumber(0,1)
    if delta == 0:
        delta=-1
    data = {
        "delta": delta,
    }
    postNumber=poller_utility.getRandomNumber(1,poller_config.maxNumPosts)
    result=poller_api.putVotePost(c_session, postNumber, data, headers)
    if result.status_code == 400:
        print(" ", poller_utility.getMessage(result), flush=True)
        return
    message=":votePost:"+str(postNumber)+":"+str(delta)
    poller_utility.checkResult(result, 200, message)
    print("  Vote", delta, "on topic: ", postNumber, flush=True)

def doBasicSocketIOTask(c_session, user, headers):
    print("  Basic SocketIO operations with", user, flush=True)
    BASEADDR='http://{}:{}'.format(poller_config.HOST, poller_config.PORT)
    SOCKETBASEADDR=BASEADDR+'/socket.io/'
    hdrs_json=json.dumps(headers)
    sClient=sioClient()
    sClient.doConnect(SOCKETBASEADDR, headers)
    time.sleep(0.5)
    sClient.doMessage("meta.rooms.enter",{"enter":"categories"})
    time.sleep(0.5)
    sClient.doMessage("meta.rooms.leaveCurrent", {})
    time.sleep(0.5)
    sClient.doMessage("meta.rooms.enter",{"enter":"category_2"})
    time.sleep(0.5)
    sClient.doMessage("meta.rooms.leaveCurrent", {})
    time.sleep(0.5)
    sClient.doMessage("categories.loadMore",{"cid":2,"after":1,"direction":1,"query":{},"categoryTopicSort":"newest_to_oldest"})
    time.sleep(0.5)
    sClient.doMessage("meta.rooms.leaveCurrent", {})
    time.sleep(0.5)
    sClient.doMessage("meta.rooms.enter",{"enter":"topic_1"})
    time.sleep(0.5)
    sClient.doMessage("topics.loadMore",{"tid":1,"after":1,"count":20,"direction":1,"topicPostSort":"oldest_to_newest"})
    time.sleep(0.5)
    sClient.doMessage("topics.markAsRead",[1])
    time.sleep(0.5)
    sClient.doDisconnect()

def doEverything():
    c_session = requests.Session()
    headers=poller_config.headers
    ## GET FORUM CONFIGURATION
    #print("GET Config:", flush=True)
    result=poller_api.getConfig(c_session, headers)
    poller_utility.checkResult(result, 200)
    headers['x-csrf-token']=poller_utility.getCRSFToken(result)
    ## SUCCESSFUL USER LOGIN
    user=poller_utility.getRandomUser()
    #print("POST Login (", user, ")", flush=True)
    result=poller_api.doLogin(c_session, user, poller_utility.getUserPassword(user), headers)
    poller_utility.checkResult(result, 200, user)
    print("Successful Login:", user, flush=True)
    headers['Cookie']=poller_utility.getCookie(result)
    del headers['x-csrf-token']
    ## GET CHESS USER'S HOMEPAGE
    result=poller_api.getHomepage(c_session, headers)
    print("Get Homepage", flush=True)
    poller_utility.checkResult(result, 200)
    headers['x-csrf-token']=poller_utility.getCRSFTokenSoup(result)
    result=doSomething(c_session, user, headers)
    print("Completed do something for user:", user, flush=True)
    c_session.close()


possibleFunctions = [
    createGroup,
    leaveJoinGroup,
    createTopic,
    replyTopic,
    votePost,
    doBasicSocketIOTask,
    ##additional from povs

]

possibleTags=[
    'test', 'topic', 'dogs', 'cats', 'birds', 'reptiles', 'fishes'
]

def doSomething(c_session, user, headers):
    pickedFunction = possibleFunctions[poller_utility.getRandomNumber(0,len(possibleFunctions)-1)]
    pickedFunction(c_session, user, headers)