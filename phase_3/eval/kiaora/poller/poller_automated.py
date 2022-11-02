import time
import re
import poller_comms
import poller_config
import poller_utility

HEALTHTHRESHOLD=0.9
MANATHRESHOLD=0.8
ENDURANCETHRESHOLD=0.8
possible_directions = ["north", "east", "south", "west", "northeast", "southeast", "northwest", "southwest", "up", "down"]
exit_descriptions = {
    "warning sign with a skull mounted":"north",
    "a tower in the forest":"north",
    "edge of the forest":"east",
    "narrow trail through the deep, dark forest":"west",
    "south of here a river flows eastwards through the forest":"south",
}
creaturesToHunt = ['fox', 'rabbit', 'bear', 'badger', 'skunk', 'deer', 'ferret']
attackOutcomes = {
    "there is nobody here":1,
    "is dead":2,
    "is stunned":3,
    "is mortally wounded":4,
    "is incapacitated":5,
}

def doPickRandomDir(directions):
    return directions[poller_utility.getRandomNumber(1,len(directions))-1]

def amIAtAnExit(description, directions):
    for key in exit_descriptions:
        if poller_utility.findinmessage(description.replace('\n', ' ').replace('\r', ''), key) != -1:
            poller_utility.printMsg("I'm at an exit, removing "+exit_descriptions[key], 0)
            directions.remove(exit_descriptions[key])
            return directions
    return directions

def doLimitDirections(conn, description):
    availdirections=doDirections(conn)
    availdirections=amIAtAnExit(description, availdirections)
    ##other reasons to limit
    return availdirections

def doGoDirection(conn, direction):
    #print("Going direction:", direction, flush=True)
    poller_comms.sendSingleMessageNoResp(conn, direction)

def doLook(conn):
    poller_comms.sendSingleMessageNoResp(conn, 'look')
    response=poller_comms.recvdata(conn)
    if response is not None:
        return response
    return ""

def doLookForCreatures(description):
    for creature in creaturesToHunt:
        if poller_utility.findinmessage(description, creature) != -1:
            return creature
    return -1

def determineAttackOutcome(description):
    if description is None:
        return -1
    for outcome in attackOutcomes:
        result=poller_utility.findinmessage(description, outcome)
        if result != -1:
            return attackOutcomes[outcome]
    return -1

def doAttackRoutine(conn, description):
    foundCreature=doLookForCreatures(description)
    if foundCreature==-1:
        poller_utility.printMsg("No creatures to attack here", poller_config.STATUS)
        return 0
    poller_comms.sendSingleMessageNoResp(conn, 'kill '+foundCreature)
    fightStatus=-1
    count=0
    while fightStatus != 0:
        fightStatus=determineAttackOutcome(poller_comms.getMessage(conn))
        if fightStatus == -1 and count == 10:
            if poller_utility.findinmessage(description, 'corpse') != -1:
                doBuryRoutine(conn)
            return 0
        if fightStatus==1: # not found
            if poller_utility.findinmessage(description, 'corpse') != -1:
                doBuryRoutine(conn)
            poller_utility.printMsg(foundCreature+" not found", poller_config.STATUS)
            return 0
        if fightStatus==2: # killed
            doLootRoutine(conn)
            doBuryRoutine(conn)
            return 1
        if fightStatus>=3: # stunned, mortally wounded, incapacitated
            poller_comms.sendSingleMessageNoResp(conn, 'kill '+foundCreature)
            continue
        count+=1
    return 0

def doLootRoutine(conn):
    poller_utility.printMsg("Loot Routine", poller_config.STATUS)
    poller_comms.sendSingleMessage(conn, 'get all from corpse')
    return
def doBuryRoutine(conn):
    poller_utility.printMsg("Bury Routine", poller_config.STATUS)
    poller_comms.sendSingleMessage(conn, 'bury corpse')
    return

def doStatusCheckRoutine(conn):
    poller_utility.printMsg("Status Check Routine", poller_config.STATUS)
    poller_comms.sendSingleMessageNoResp(conn, 'score')
    messages=poller_comms.getMessage(conn, 0).splitlines()
    if messages == -1:
        return
    if doCheckForRest(conn, messages) == 1:
            doRestRoutine(conn)
    for line in messages:
        levelmsg=poller_utility.findinmessage(line, 'you need')
        if levelmsg != -1:
            pointstonextlevel=line[levelmsg:].split(' ')[2]
            poller_utility.printMsg("This many points to next level: "+ str(pointstonextlevel), poller_config.STATUS)
            if (int(pointstonextlevel) < 0):           
                doLevelRoutine(conn)
    return

def doLevelRoutine(conn):
    poller_utility.printMsg("Time to level up", poller_config.STATUS)
    poller_comms.sendSingleMessage(conn, 'level')
    poller_comms.sendSingleMessage(conn, 'practice auto')
    return

def doCheckForRest(conn, messages=""):
    if messages == "":
        poller_comms.sendSingleMessageNoResp(conn, 'score')
        messages=poller_comms.getMessage(conn, 0).splitlines()
    if messages == -1:
        return -1
    for line in messages:
        pointmsg=poller_utility.findinmessage(line, 'endurance points')
        if pointmsg != -1:
            maxhp=line.split('(')[1].split(')')[0]
            hp=line.split('(')[0].split(' ')[2]
            maxmp=line.split('(')[2].split(')')[0]
            mp=line.split('(')[1].split(' ')[2]
            maxep=line.split('(')[3].split(')')[0]
            ep=line.split('(')[2].split(' ')[2]
            poller_utility.printMsg("I have "+str(hp)+" hp out of "+str(maxhp)+", " +str(mp)+" mp out of "+str(maxmp)+", "+str(ep)+" ep out of "+str(maxep)+".", poller_config.STATUS)
            hppercent=float(float(hp)/float(maxhp))
            mppercent=float(float(mp)/float(maxmp))
            eppercent=float(float(ep)/float(maxep))
            if hppercent < HEALTHTHRESHOLD or mppercent < MANATHRESHOLD or eppercent < ENDURANCETHRESHOLD:
                return 1
            else:
                return 0
    return -1

def doRestRoutine(conn):
    poller_utility.printMsg("resting", poller_config.STATUS)
    poller_comms.sendSingleMessage(conn, 'rest')
    while doCheckForRest(conn) == 1:
        time.sleep(3)
        poller_utility.printMsg("still resting", poller_config.STATUS)
    poller_utility.printMsg("done resting", poller_config.STATUS)
    poller_comms.sendSingleMessage(conn, 'stand')
    ##gonna need to think about eating and drinking
    return

def doDirections(conn):
    available_exits=[]
    poller_comms.sendSingleMessageNoResp(conn, 'directions')
    response=poller_comms.recvdata(conn)
    if response is not None:
        if response.find('Obvious exits:') != -1:
            directions=response.split('\r')
            for dir in directions:
                if dir.find('-') != -1:
                    addexit=''.join([i if ord(i) < 127 and ord(i) > 32 else '' for i in dir.split('-')[0]])
                    for pdir in possible_directions:
                        result=poller_utility.findinmessage(addexit, pdir)
                        if result != -1:
                            addexit=addexit[result:]
                            break
                    available_exits.append(addexit)
            return available_exits