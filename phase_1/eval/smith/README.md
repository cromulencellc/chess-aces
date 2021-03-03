# smith
This challenge implements an IRC server capable of serving multiple clients.

# IRC Verbs

Parameters encased by [ ] indication optional arguments

All channel names begin with either a '#' or '&'

NICK <nick>CRLF

USER <username> <hostname> <servername> :<realname>

## Nick and User must both be supplied prior to being considered connected

QUIT [<Quit message>]

Terminates the client connection

AWAY [<Away message>]

If the away message is not provided then this is a signal to the server to remove the away flag.

If the away message is set then the server marks the user as away and any PRIVMSGs from other users have
an automatic response of <Away message>.

JOIN <channel>{,<channel>} [<key>{,<key>}]

The <key> arguments correspond to each requested join. These are only necessary if the +k mode is set for the channel
Multiple channels can be joined simultaneously.

PART <channel>{,<channel>}

Leave one or more channels

MODE <channel> {[+|-]|p|s|i|t|n|k|l|b} [<limit>] [<key>] [<banregex>]

Set the mode for a channel + indicates an addition - indicates removal
p - private channel flag
s - secret channel flag
i - Set invite-only flag
t - The topic can only be changed by an operator
n - Only members are allowed to message the channel
m - Channel is moderated, only voiced members can chat
l - set the maximum number of members. Requires the <limit> argument
k - Set a channel key. Requires the <key> argument
b - Add a regular expression to ban certain users

MODE <channel> {[+|-]o|v} <nick>

Set the mode for a nick within a channel. The client setting the mode must be a channel operator
o - give/take channel operator privileges;
v - give/take the ability to speak on a moderated channel;

TOPIC <channel> [<topic>]

Sets the topic if <topic> is provided otherwise the server sends the channel's topic to the client

INVITE <nickname> <channel>

Sends a channel invite to nickname

PRIVMSG <receiver>{,<receiver>} <text to be sent>

receiver can be either a channel or another user

PING <server>

PONG <server>

KICK <channel> <user> [<comment>]

This boots a user from the channel but the client sending the request must be an operator of the channel

TIME

Returns the local time on the server

ISON <nickname>{<space><nickname>}

This can be used to ask the server is any of the space separated list of users is currently online

KNOCK <channel> [<message>]

Sends a NOTICE to an invitation-only <channel> with an optional <message>, requesting an invite.
KNOCK cannot be sent to a private channel.

LIST [<channel>{,<channel>}]

The list message is used to list channels and their topics.  If  the <channel>  parameter  is  used,
only  the  status  of  that  channel is displayed.  Private  channels  are  listed  (without  their
topics)  as channel "Prv" unless the client generating the query is actually on that channel.  
Likewise, secret channels are not listed at  all  unless  the client is a member of the channel in question.

NAMES [<channel>{,<channel>}]

By using the NAMES command, a user can list all nicknames that are visible to them on any channel that 
they can see.  Channel names which they can see are those which aren't private (+p) or secret (+s)
or those which they are actually on.  The <channel> parameter specifies which channel(s) to return 
information about if valid. There is no error reply for bad channel names.

If no <channel> parameter is given, a list of all channels and their occupants is returned.  At 
the end of this list, a list of users who are visible but either not on any channel or not on a 
visible channel are listed as being on `channel' "*".

NOTICE <target> <text>

The NOTICE message is used similarly to PRIVMSG.  The difference between NOTICE and PRIVMSG is that 
automatic replies must never be sent in response to a NOTICE message.

SILENCE <+nickname/-nickname>

Add or remove clients from the silence list. You will still see them in channels but they cannot send you a PRIVMSG

# Unhandled IRC verbs

PASS
SERVER
OPER
SQUIT
VERSION
STATS
LINKS
ADMIN
INFO


## PoV Apogee Only

The PoV connects with 15 users which is the maximum users that can join a single channel.

Each username is given the maximum length of 30 characters.

The first user joins a channel with a name of 50 characters and becomes the operator.

The operator then sets the channel to moderated and gives everyone voice. The operator also gives
operator status to everyone in the channel.

The purpose for this is to overflow the response buffer when sending the NAMES request. With these parameters the length of the response will be: 608 bytes

5 + hostnamelength + 5 + 30 + 3 + 50 + 2 + (33*15) + 5

makeresponse(response, ":irc.%s 353 %s = %s :%s %s\r\n", this->GetServerHostname().c_str(), c->GetNick().c_str(), ch->GetChannelName().c_str(), ch->GetMemberList().c_str(), ch->GetModeString().c_str());

For testing, the length of the hostname is 13. It would probably be easiest if the hostname of the evaluation environment is
the same length

':irc.ubuntu-bionic 353 hhhhhhhhhhhhhhhhhhhhhhhhhhhhhh = #bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb :@+aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa @+bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb @+cccccccccccccccccccccccccccccc @+ppppppppqqqqqqqqrrrrrrrrssssss @+eeeeeeeeeeeeeeeeeeeeeeeeeeeeee @+ffffffffffffffffffffffffffffff @+gggggggggggggggggggggggggggggg @+hhhhhhhhhhhhhhhhhhhhhhhhhhhhhh @+iiiiiiiiiiiiiiiiiiiiiiiiiiiiii @+jjjjjjjjjjjjjjjjjjjjjjjjjjjjjj @+kkkkkkkkkkkkkkkkkkkkkkkkkkkkkk @+llllllllllllllllllllllllllllll @+mmmmmmmmmmmmmmmmmmmmmmmmmmmmmm @+nnnnnnnnnnnnnnnnnnnnnnnnnnnnnn @+oooooooooooooooooooooooooooooo +tn\r\n'

The buffer allocated in Server::HandleNAMES() is of length 576.

The PoV overwrites RBP and the saved RIP so on a return from the HandleNAMES function the challenge crashes at a retq.

Program received signal SIGSEGV, Segmentation fault.
0x0000558440f45ac6 in Server::HandleNAMES (
    this=<error reading variable: Cannot access memory at address 0x6f6f6f6f6f6f6f67>)
    at src/irc_handles.cpp:1302
warning: Source file is more recent than executable.
1302    }
(gdb) i r $pc
pc             0x558440f45ac6   0x558440f45ac6 <Server::HandleNAMES()+4694>
(gdb) x /i $pc
=> 0x558440f45ac6 <Server::HandleNAMES()+4694>: retq
(gdb) i r $rbp
rbp            0x6f6f6f6f6f6f6f6f   0x6f6f6f6f6f6f6f6f
(gdb) x /gx $rsp
0x7ffc155da9d8: 0x6f6f6f6f6f6f6f6f
(gdb)