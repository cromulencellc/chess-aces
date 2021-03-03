import sys
import os
import select
import random
import string
import datetime

from subprocess import Popen, PIPE

class client:

    def __init__(self, bin, nick, HOST, PORT):
        jackson_env = os.environ

        jackson_env['HOST'] = HOST
        jackson_env['PORT'] = PORT
        jackson_env['IRCNICK'] = nick
        jackson_env['IRCUSER'] = nick

        self.nick = nick

        self.proc = Popen([bin], stdin=PIPE, stdout=PIPE, env = jackson_env)

        self.rooms = []

        self.responses_expected = []

        self.cmd_count = random.randint(10,20)

        self.privmsgs = []

        self.rooms_visited = []

    def re_empty(self):
        return not (len(self.responses_expected) > 0)

    def deccmdcnt( self ):
        self.cmd_count -= 1

    def cmddone( self ):
        return self.cmd_count > 0

    def sendline( self, line ):
        self.proc.stdin.write(line)
        self.proc.stdin.flush()

    def addexpected( self, line ):
        #print("[DEBUG] %s -- Add expected: %s" %(self.nick, line))
        self.responses_expected.append(line)

    def checklist(self, line):
        for x in self.responses_expected:
            if line.find(x) != -1:
                self.responses_expected.remove(x)
                return 0

        print("[ERROR] %s: Not expected: %s" %(self.nick, line))

        if ( line.find('is already in use.') != -1 ):
            print('[INFO] It is likely that another instance of jackson is running. Make sure to kill them')

        print self.responses_expected

        return 1


### This is a list of rooms that at least one of the clients have joined
all_rooms = {}

def rs(l):
    z = ''

    for _ in range(l):
        z += random.choice( string.lowercase )

    return z

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1)
    except Exception as e:
        print('[ERROR]')
        print (e)
        return None

    return z

def readline( s ):
    return readuntil( s, '\n')

def gen_data_list():
    dl = []

    for _ in range( random.randint(3, 8) ):
        dl.append( rs( random.randint(3,8)))

    return dl


def msg_user( dest_client, src_client, data):
    cmd = "/msg " + dest_client.nick + " "
    message = ""

    for x in data:
        message += " " + x

    message = message.lstrip()

    cmd += message

    expected = message

    ## If this is the first time messaging the dest client with this nick then add it to the list
    if src_client.nick not in dest_client.privmsgs:
        dest_client.privmsgs.append(src_client.nick)

    if dest_client.nick not in src_client.privmsgs:
        src_client.privmsgs.append(dest_client.nick)

    dest_client.addexpected(expected)
    src_client.addexpected(expected)

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )

    src_client.sendline( cmd + '\n')

    return

def info_cmd( dest_client, src_client, data):
    cmd = "/info"

    src_client.addexpected("IRC --")
    src_client.addexpected("Jarkko Oikarinen")
    src_client.addexpected("University of Oulu, Computing Center")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("This program is free software; you can")
    src_client.addexpected("modify it under the terms of the ")
    src_client.addexpected("published by the Free Software Foundation")
    src_client.addexpected("(at your option) any later version")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("The following persons have made")
    src_client.addexpected("code and still know how IRC")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("Kucharski")
    src_client.addexpected("-!-  \n")
    src_client.addexpected(" for their work on 2.11")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("jv@pilsedu.cz")
    src_client.addexpected("kurt@roeckx.be")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("for their work on 2.10")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("kalt@stealth.net")
    src_client.addexpected("d92-mtm@ludd.luth.se")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("Those who helped in prior versions:")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("Oikarinen")
    src_client.addexpected("Vesa Ruokonen")
    src_client.addexpected("Matthew Green")
    src_client.addexpected("Markku Savela")
    src_client.addexpected("Stellan Klebom")
    src_client.addexpected("Ian Frechette")
    src_client.addexpected("Jeff Trim")
    src_client.addexpected("Bill Wisner")
    src_client.addexpected("Tom Hopkins")
    src_client.addexpected("Bo Adler")
    src_client.addexpected("Jan Peterson")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("Thanks also goes to those persons not mentioned here who have added")
    src_client.addexpected("their advice, opinions, and code to IRC.")
    src_client.addexpected("Thanks also to those who provide the kind sys admins who let me and")
    src_client.addexpected("others continue to develop IRC.")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("s_serv.c")
    src_client.addexpected("hash.c")
    src_client.addexpected("-!-  \n")
    src_client.addexpected("Birth Date:")
    src_client.addexpected("On-line since")
    src_client.addexpected("End of INFO")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )

    src_client.sendline( cmd + '\n')

    return


## The need for this is to make sure that the join command
### has been resolved for the other client prior to the second client joining
def check_join_resolve( dest_client, channel_name):
    result = 0

    if channel_name == None:
        return 0

    for line in dest_client.responses_expected:
        if line.find(channel_name):
            result = 1
            break

    return result


def join_cmd( dest_client, src_client, data):
    ### I can only join so many rooms

    if len(src_client.rooms) > 10:
        src_client.cmd_count += 1
        return

    room_to_join = None

    while room_to_join == None:
        ### If there aren't any rooms then create one
        if len(all_rooms) == 0:
            nr = rs( random.randint(5,9))
            all_rooms[nr] = ""

        ## If rooms exist then get a room that I am not already in
        for r in all_rooms:
            if r not in src_client.rooms:
                room_to_join = r
                break

        ## Make sure that the other client isn't still waiting to resolve their join command
        if ( check_join_resolve( dest_client, room_to_join)):
            continue

        ## If a room wasn't found it means that no rooms exist that the client isn't already in
        ### So create one
        if room_to_join == None:
            nr = rs( random.randint(5,9))
            all_rooms[nr] = ""

            room_to_join = nr

    cmd = '/join ' + room_to_join

    src_client.rooms.append(room_to_join)

    ### If the other client is already there then they should expect a message
    if room_to_join in dest_client.rooms:
        print("{DEBUG| %s joining %s" %(src_client.nick, room_to_join))
        dest_client.addexpected( 'has joined #' + room_to_join)


    if room_to_join in all_rooms:
        if all_rooms[room_to_join] != "":
            src_client.addexpected("topic set to: ")

    src_client.addexpected('has joined #' + room_to_join)
    src_client.addexpected("[Users #" + room_to_join + "]")
    src_client.addexpected("[" + src_client.nick + "]")
    src_client.addexpected("#" + room_to_join + ": Total of")

    if room_to_join not in src_client.rooms_visited:
        src_client.rooms_visited.append(room_to_join)

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n' )

    return

def connect_cmd( dest_client, src_client, data):
    ## add expectations first
    src_client.addexpected("Connected...")
    src_client.addexpected("-!- Please wait while we process your connection.")
    src_client.addexpected("RFC2812 PREFIX=(ov)@+ CHANTYPES=#&!+ MODES=3")
    src_client.addexpected("your unique ID")
    src_client.addexpected("users and 0 services on")
    src_client.addexpected("channels formed")
    src_client.addexpected("0 services and 0 servers")
    src_client.addexpected("Current local users")
    src_client.addexpected("Current global users")
    src_client.addexpected("Message of the Day -")
    src_client.addexpected("/201")  ### This line may cause issues
    src_client.addexpected("Debian GNU")
    src_client.addexpected("|------------------")
    src_client.addexpected("| This is Debian's default IRCd server")
    src_client.addexpected("| see this and if you are the server")
    src_client.addexpected("| and ircd.motd in /etc/ircd.")
    src_client.addexpected("Martin Loschwitz, 1st")
    src_client.addexpected("|----------------------------------------")
    src_client.addexpected("End of MOTD command.")

    src_client.sendline("/connect localhost\n")

def msg_room( dest_client, src_client, data):
    ## Attempt to find a room both clients are in
    lu = list(set(dest_client.rooms).intersection(src_client.rooms))

    ## If they do not share any rooms then just return
    if len(lu) == 0:
        ## Increase the command count so that this doesn't take from it
        src_client.cmd_count += 1
        return

    mr = random.choice(lu)

    cmd = "/msg #" + mr

    message = ""
    for w in data:
        message += " " + w

    cmd += message

    dest_client.addexpected( message )
    src_client.addexpected( message )

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

## I implemented the  userip command before I knew that the ubuntu server didn't implement it
def userip_cmd( dest_client, src_client, data ):
    nn = random.choice( [dest_client.nick, src_client.nick])

    cmd = "/userip " + nn

    src_client.addexpected( "USERIP: Unknown")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

    return

## I also implemented watch before I knew ircd didn't support it
def watch_cmd( dest_client, src_client, data ):
    nn = random.choice( [dest_client.nick, src_client.nick])

    cmd = "/watch " + nn

    src_client.addexpected( "WATCH: Unknown")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

    return

def ison_cmd( dest_client, src_client, data ):
    nn = random.choice( [dest_client.nick, data[0]])

    cmd = "/ison " + nn

    src_client.addexpected( "Users online:")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

    return

def userhost_cmd( dest_client, src_client, data ):
    nn = random.choice( [dest_client.nick, src_client.nick])

    cmd = "/userhost " + nn

    src_client.addexpected( "-!- " + nn)

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

    return

def list_cmd( dest_client, src_client, data ):

    cmd = "/list"

    for c in all_rooms:
        src_client.addexpected( "-!-  #" + c)

    src_client.addexpected("wallops received")
    src_client.addexpected("save messages")
    src_client.addexpected("messages from the authentication slave")
    src_client.addexpected("services joining and leaving")
    src_client.addexpected("notices about local connections")
    src_client.addexpected("hash tables growth")
    src_client.addexpected("servers joining and leaving")
    src_client.addexpected("numerics received")
    src_client.addexpected("fake modes")
    src_client.addexpected("operator and server kills")
    src_client.addexpected("warnings and notices")
    src_client.addexpected("server errors")
    src_client.addexpected("End of LIST")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

def who_cmd( dest_client, src_client, data ):
    nn = random.choice( [dest_client.nick, src_client.nick])

    cmd = "/who " + nn

    src_client.addexpected( "-!-  " + nn)
    src_client.addexpected( "f WHO list")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

    return

def listchan_cmd( dest_client, src_client, data):
    cmd = "/listchans"

    ## Add each joined room
    for c in src_client.rooms_visited:
        src_client.addexpected( ") #" + c + "\n")

    #print src_client.rooms_visited

    ## add the main room
    src_client.addexpected(") main\n")

    ## add all the nicks that have sent private messages
    for n in src_client.privmsgs:
        src_client.addexpected(") " + n + "\n")

    #print src_client.responses_expected

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

def motd_cmd( dest_client, src_client, data):
    cmd = "/motd"

    src_client.addexpected("Message of the Day -")
    src_client.addexpected("/201")  ### This line may cause issues
    src_client.addexpected("Debian GNU")
    src_client.addexpected("|------------------")
    src_client.addexpected("| This is Debian's default IRCd server")
    src_client.addexpected("| see this and if you are the server")
    src_client.addexpected("| and ircd.motd in /etc/ircd.")
    src_client.addexpected("Martin Loschwitz, 1st")
    src_client.addexpected("|----------------------------------------")
    src_client.addexpected("End of MOTD command.")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline( cmd + '\n')

def names_cmd( dest_client, src_client, data):

    cmd = "/names"

    for mr in src_client.rooms:
        src_client.addexpected("[Users #" + mr + "]")
        src_client.addexpected("[" + src_client.nick + "]")
        src_client.addexpected("#" + mr + ": Total of ")

    for n in all_rooms:
        if (n in src_client.rooms_visited) and (not (n in src_client.rooms)) and (n in dest_client.rooms):
            src_client.addexpected("[Users #" + n + "]")
            src_client.addexpected("[" + dest_client.nick + "]")
            src_client.addexpected("#" + n + ": Total of ")

        elif n in src_client.rooms:
            continue
        else:
            ### if no one is in the room then it is cleared
            if (not n in src_client.rooms) and (not n in dest_client.rooms):
                continue

            src_client.addexpected("el: #" + n)

    src_client.addexpected(": &WALLOPS")
    src_client.addexpected(": &SAVE")
    src_client.addexpected(": &AUTH")
    src_client.addexpected(": &SERVICES")
    src_client.addexpected(": &LOCAL")
    src_client.addexpected(": &HASH")
    src_client.addexpected(": &SERVERS")
    src_client.addexpected(": &NUMERICS")
    src_client.addexpected(": &KILLS")
    src_client.addexpected(": &CHANNEL")
    src_client.addexpected(": &NOTICES")
    src_client.addexpected(": &ERRORS")
    src_client.addexpected("nel: *")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def nick_cmd( dest_client, src_client, data):

    cmd = "/nick"

    new_nick = rs( random.randint(5,8))

    ## If they share a room or have a private message then send the other client the change
    lu = list(set(dest_client.rooms).intersection(src_client.rooms))

    if len(lu) > 0:
        dest_client.addexpected("is now known as " + new_nick)

    src_client.addexpected("You're now known as " + new_nick)

    cmd += " " + new_nick

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

    src_client.nick = new_nick

    return

def stats_mem( src_client ):
    cmd = "/stats mem"

    src_client.addexpected("Client Local")
    src_client.addexpected("- Users ")
    src_client.addexpected("User channels")
    src_client.addexpected("Attached confs")
    src_client.addexpected("Conflines ")
    src_client.addexpected("Classes ")
    src_client.addexpected("Channels ")
    src_client.addexpected("Channel members ")
    src_client.addexpected("Whowas users ")
    src_client.addexpected("Whowas array ")
    src_client.addexpected("Hash: client")
    src_client.addexpected("Dbuf blocks")
    src_client.addexpected("RES table ")
    src_client.addexpected("Structs ")
    src_client.addexpected("Total: ")
    src_client.addexpected("TOTAL: ")
    src_client.addexpected("z End of STATS")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def stats_gen_stat( src_client ):
    cmd = "/stats gen_stat"

    src_client.addexpected("-!- accepts")
    src_client.addexpected("unknown: comm")
    src_client.addexpected("nick collisions")
    src_client.addexpected("wrong direction")
    src_client.addexpected("users without ")
    src_client.addexpected("numerics seen")
    src_client.addexpected("auth: succ")
    src_client.addexpected("local connections")
    src_client.addexpected("udp errors ")
    src_client.addexpected("link checks")
    src_client.addexpected("abuse protections ")
    src_client.addexpected("delay close ")
    src_client.addexpected("local channels ")
    src_client.addexpected("Client - Server")
    src_client.addexpected("connected ")
    src_client.addexpected("bytes sent ")
    src_client.addexpected("bytes recv ")
    src_client.addexpected("time connected ")
    src_client.addexpected("iauth modules stat")
    src_client.addexpected("spawned: ")
    src_client.addexpected("rfc931 connect")
    src_client.addexpected("rfc931 skipped")
    src_client.addexpected("t End of STATS")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def stats_resource( src_client ):
    cmd = "/stats resource"

    src_client.addexpected("CPU Secs")
    src_client.addexpected("- RSS")
    src_client.addexpected("Swaps ")
    src_client.addexpected("Block in")
    src_client.addexpected("Msg Rcv")
    src_client.addexpected("Signals ")
    src_client.addexpected("DBUF alloc")
    src_client.addexpected("r End of STATS report")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def stats_up_time( src_client ):
    cmd = "/stats up_time"

    src_client.addexpected("Server Up ")
    src_client.addexpected("u End of STATS report")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def stats_op_list( src_client ):
    cmd = "/stats op_list"

    src_client.addexpected("o End of STATS report")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def stats_cmd_cnt( src_client ):
    cmd = "/stats cmd_cnt"

    src_client.addexpected("PRIVMSG")
    src_client.addexpected("JOIN")
    src_client.addexpected("NICK")
    src_client.addexpected("PART")
    src_client.addexpected("QUIT")
    src_client.addexpected("PONG")
    src_client.addexpected("USER")
    src_client.addexpected("ISON")
    src_client.addexpected("LIST")
    src_client.addexpected("NAMES")
    src_client.addexpected("STATS")
    src_client.addexpected("INFO")
    src_client.addexpected("MOTD")
    src_client.addexpected("TIME")
    src_client.addexpected("m End of STATS report")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def stats_serv_list( src_client ):
    cmd = "/stats serv_list"

    src_client.addexpected("@.6667")
    src_client.addexpected(src_client.nick + "[~")
    src_client.addexpected("End of STATS report")

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def stats_cmd( dest_client, src_client, data):
    sub_cmd = [stats_serv_list, stats_op_list, stats_up_time, stats_resource, stats_gen_stat, stats_mem]

    sc = random.choice(sub_cmd)

    sc( src_client )

    return

def part_cmd( dest_client, src_client, data):

    cmd = "/part "

    ## Make sure that there are rooms to part from
    if len( src_client.rooms ) == 0:
        src_client.cmd_count += 1

        return

    pr = random.choice( src_client.rooms )

    src_client.rooms.remove(pr)

    message = ""

    for l in data:
        message += " " + l

    if pr in dest_client.rooms:
        dest_client.addexpected(src_client.nick + " left: ")
    else:
        ### This means the room is empty
        all_rooms.pop(pr)

    src_client.addexpected( src_client.nick + " left: ")
    
    cmd += "#" + pr + message

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

    return

## 12:35 -!- irc.localhost Friday September 20 2019 -- 12:35 +00:00
def time_cmd( dest_client, src_client, data):
    cmd = "/time"

    src_client.addexpected(datetime.datetime.today().strftime('%A %B'))

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")

def topic_cmd( dest_client, src_client, data):
    cmd = "/topic"

    if len(src_client.rooms) == 0:
        src_client.cmd_count += 1
        return

    channel = random.choice(src_client.rooms)

    cmd += " #" + channel

    ### possibly set the topic
    if ( random.randint(0,100) > 80):
        tp = " " + data[0]

        all_rooms[channel] = tp
        cmd += tp

        src_client.addexpected("topic set to:" + tp)

        if channel in dest_client.rooms:
            dest_client.addexpected("topic set to:" + tp)
    else:
        if all_rooms[channel] == "":
            src_client.addexpected("No topic is set")
        else:
            src_client.addexpected( all_rooms[channel] )

    print("[TEST %s] cmd: %s" %(src_client.nick, cmd) )
    src_client.sendline(cmd + "\n")


def run_test( HOST, PORT ):
    if 'JACKSON' not in os.environ:
        print("[ERROR]: The JACKSON environment variable needs to be set to the location of the jackson CB")

    bin = os.environ['JACKSON']

    clienta = client( bin, "clienta", HOST, PORT)

    clientb = client( bin, "clientb", HOST, PORT)

    ### Connect each one
    connect_cmd( None, clienta, None)

    connect_cmd( None, clientb, None)

    last_count_a = None
    last_count_b = None
    no_reduce = 0

    cmd_list = [who_cmd, watch_cmd, userip_cmd, userhost_cmd, topic_cmd, time_cmd, stats_cmd, msg_user, info_cmd, join_cmd, msg_room, ison_cmd, list_cmd, listchan_cmd, motd_cmd, nick_cmd, names_cmd, part_cmd]

    while (clienta.cmd_count > 0) or (clientb.cmd_count > 0) or (len(clienta.responses_expected) > 0) or (len(clientb.responses_expected) > 0):

        ## It can take time for the server responses to catch up
        ### After there are no remaining commands to be executed for either client I begin monitoring expected responses
        ### As long as the responses are decreasing without throwing an error then we are still good
        if not clienta.cmddone() and not clientb.cmddone():
            cur_cnt_a = len(clienta.responses_expected)
            cur_cnt_b = len(clientb.responses_expected)

            ### If they are none then this is the first round
            if last_count_b == None or last_count_a == None:
                last_count_a = cur_cnt_a
                last_count_b = cur_cnt_b
            

            ## If one of them is reduced then we are good
            if (cur_cnt_b < last_count_b) or (cur_cnt_a < last_count_a):
                print('INFO: The response queue is reducing. Hang tight: %d -- %d' %(cur_cnt_a, cur_cnt_b))
                last_count_b = cur_cnt_b
                last_count_a = cur_cnt_a

                no_reduce = 0
            else:
                no_reduce += 1

                if ( no_reduce >= 5):
                    print('INFO: Something is wrong with the server response queue: %d -- %d' %(cur_cnt_a, cur_cnt_b))
                    print clienta.responses_expected
                    print '-'*20
                    print clientb.responses_expected
                    print '-'*20


        ## This while loop goes until the expected responses are cleared out
        ## I want to clear the read queue prior to writing a new command because some commands result in multiple lines
        ### There isn't a one to one correlation
        while (len(clienta.responses_expected) > 0) or (len(clientb.responses_expected) > 0):

            ## This select will only be called if I am expecting data from the server
            slist = select.select( [clienta.proc.stdout, clientb.proc.stdout], [], [], 30)

            ## If the read list is empty after 30 seconds that means something is ver wrong
            rlist = slist[0]

            if len(rlist) == 0:
                print("[INFO] Waiting for: ")
                print("Client A: ")
                print clienta.responses_expected
                print("Client B: ")
                print clientb.responses_expected

                continue

            for r in rlist:
                if r == clienta.proc.stdout:
                    y = clienta.proc.stdout.readline()

                    #print("[DEBUG] %s -- Received: %s" %(clienta.nick, y))
                    if clienta.checklist( y ):
                        clienta.proc.kill()
                        clientb.proc.kill()
                        print clienta.responses_expected
                        print '-'*20
                        print clientb.responses_expected
                        sys.exit()

                elif r == clientb.proc.stdout:
                    y = clientb.proc.stdout.readline()

                    #print("[DEBUG] %s -- Received: %s" %(clientb.nick, y.rstrip("\n")))
                    if clientb.checklist( y):
                        clienta.proc.kill()
                        clientb.proc.kill()
                        print clienta.responses_expected
                        print '-'*20
                        print clientb.responses_expected
                        sys.exit()


        '''
        print("Post loop Client A: ")
        print clienta.responses_expected
        print("Post loop Client B: ")
        print clientb.responses_expected
        '''

        clients = []

        if clienta.cmd_count > 0:
            clients.append(clienta)

        if clientb.cmd_count > 0:
            clients.append(clientb)

        ### If commands are done then just continue
        if len(clients) == 0:
            continue

        csrc = random.choice( clients )

        ## awful way to get the other one
        cdest = list( set([clienta, clientb]).difference([csrc]) )[0]

        cmd = random.choice(cmd_list)

        ## only run info once
        if cmd in [info_cmd, list_cmd, motd_cmd]:
            cmd_list.remove(cmd)

        cmd( cdest, csrc, gen_data_list())
        
        csrc.deccmdcnt()

        '''
        print("Client A: ")
        print clienta.responses_expected
        print clientb.rooms
        print("Client B: ")
        print clientb.responses_expected
        print clientb.rooms
        '''

    print("[INFO] All poller expectations met")

    clienta.proc.kill()
    clientb.proc.kill()

    os.system("pkill jackson")

    return 0

if __name__ == '__main__':
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = os.environ['PORT']
        else:
            print('[ERROR] target and port must be specified via arguments or variables.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] Poller for the Jackson Challenge Binary')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    run_test(HOST, PORT)


    

    