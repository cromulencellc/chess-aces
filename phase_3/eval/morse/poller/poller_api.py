import poller_config

def getConfig(conn, headers=""):
    address=poller_config.BASEADDR+'/api/config'
    return conn.get(address, headers=headers)

def getPing(conn, headers=""):
    address=poller_config.BASEADDR+'/api/v3/ping'
    return conn.get(address, headers=headers)

 ### GROUP
def postCreateGroup(conn, data, headers=""):
    address=poller_config.BASEADDR+'/api/v3/groups/'
    result = conn.post(address, headers=headers, data=data)
    return result    

def headCheckGroup(conn, name, headers=""):
    address=poller_config.BASEADDR+'/api/v3/groups/'+name
    result = conn.head(address, headers=headers)
    return result 

def putJoinGroup(conn, name, uid, headers=""):
    address=poller_config.BASEADDR+'/api/v3/groups/'+name+'/membership/'+str(uid)
    result = conn.put(address, headers=headers)
    return result 

def deleteLeaveGroup(conn, name, uid, headers=""):
    address=poller_config.BASEADDR+'/api/v3/groups/'+name+'/membership/'+str(uid)
    result = conn.delete(address, headers=headers)
    return result    

### TOPICS
def getTopic(conn, tid, headers=""):
    address=poller_config.BASEADDR+'/api/v3/topics/'+str(tid)
    result = conn.get(address, headers=headers)
    return result

def postCreateTopic(conn, data, headers=""):
    address=poller_config.BASEADDR+'/api/v3/topics/'
    result = conn.post(address, headers=headers, data=data)
    return result

def postReplyTopic(conn, tid, data, headers=""):
    address=poller_config.BASEADDR+'/api/v3/topics/'+str(tid)
    result = conn.post(address, headers=headers, data=data)
    return result

### POSTS
def getPost(conn, pid, headers=""):
    address=poller_config.BASEADDR+'/api/v3/topics/'+str(pid)
    result = conn.get(address, headers=headers)
    return result

def putVotePost(conn, pid, data, headers=""):
    address=poller_config.BASEADDR+'/api/v3/posts/'+str(pid)+'/vote'
    result = conn.put(address, headers=headers, data=data)
    return result

def putEditPost(conn, pid, data, headers=""):
    address=poller_config.BASEADDR+'/api/v3/post/'+str(pid)
    result = conn.put(address, headers=headers, data=data)
    return result


### LOGIN
def doLogin(conn, username, password, headers=""):
    address=poller_config.BASEADDR+'/api/v3/utilities/login'
    result = conn.post(address, headers=headers, data={'username':username, 'password': password})
    return result

def failLogin(conn, username, headers=""):
    address=poller_config.BASEADDR+'/api/v3/utilities/login'
    result = conn.post(address, headers=headers, data={'username':username, 'password': poller_config.badpassword})
    return result

def getHomepage(conn, headers=""):
    address=poller_config.BASEADDR+'/?loggedin=true'
    result = conn.get(address, headers=headers)
    return result