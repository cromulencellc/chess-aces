## Overview

remsen is an FTP server that allows a user to interact with a file system to create, modify, delete etc. files.

##

The directory structure is:

/tmp/admin/         -- Home directory of the admin user
/tmp/admin/token    -- This is secure data to which a user should not have access
/tmp/guest/         -- Home directory of the guest user
/tmp/guest/testfile -- Random data used by the poller for testing
/tmp/plist.txt      -- Password file used for authentication. Normal users also should not have access to this file.
/tmp/alice          -- Home directory for the alice user
/tmp/bob            -- Home directory for the bob user

## Logging in
remsen is compatible with the default ftp client provided by Ubuntu.
The only known user/password combo to an outside person is guest/guest.

To log in:
USER guest
PASS guest

To get a list of available commands:
HELP

## Challenge

To launch the challenge you must specify the location of the user list with -u <path/to/file.txt>. You must also set the PORT environment variable to the port on which you want to listen.

## FTP Verbs

ABOR - Not implemented

ACCT - Not implemented

ALLO - Not implemented

APPEND (with create) (APPE)

            This command causes the server-DTP to accept the data
            transferred via the data connection and to store the data in
            a file at the server site.  If the file specified in the
            pathname exists at the server site, then the data shall be
            appended to that file; otherwise the file specified in the
            pathname shall be created at the server site.

CDUP - Changes to the parent directory

CWD - Changes the current working directory

DELE - Deletes a file

EPRT - Not implemented

EPSV - Not implemented

FEAT - Not implemented

HELP - Gives a list of FTP verbs

LIST (LIST)

            This command causes a list to be sent from the server to the
            passive DTP.  If the pathname specifies a directory or other
            group of files, the server should transfer a list of files
            in the specified directory.  If the pathname specifies a
            file then the server should send current information on the
            file.  A null argument implies the user's current working or
            default directory.
            
MDTM - Gets the file modification time

MKD - Makes a directory

MODE - Sets the mode

NAME LIST (NLST)

            This command causes a directory listing to be sent from
            server to user site.  The pathname should specify a
            directory or other system-specific file group descriptor; a
            null argument implies the current directory.

NOOP - Does nothing

OPTS - Not implemented

PASSIVE (PASV)

            This command requests the server-DTP to "listen" on a data
            port (which is not its default data port) and to wait for a
            connection rather than initiate one upon receipt of a
            transfer command.  The response to this command includes the
            host and port address this server is listening on.

DATA PORT (PORT)

            The argument is a HOST-PORT specification for the data port
            to be used in data connection.  There are defaults for both
            the user and server data ports, and under normal
            circumstances this command and its reply are not needed.  If
            this command is used, the argument is the concatenation of a
            32-bit internet host address and a 16-bit TCP port address.
            This address information is broken into 8-bit fields and the
            value of each field is transmitted as a decimal number (in
            character string representation).  The fields are separated
            by commas.  A port command would be:

               PORT h1,h2,h3,h4,p1,p2

            where h1 is the high order 8 bits of the internet host
            address.

SITE - Not Implemented

SIZE - Not Implemented

STAT - Not implemented

SYST - Prints system type

PWD - Returns the present working directory

REINITIALIZE (REIN)

            This command terminates a USER, flushing all I/O and account
            information, except to allow any transfer in progress to be
            completed.  All parameters are reset to the default settings
            and the control connection is left open.

RETRIEVE (RETR)

            This command causes the server-DTP to transfer a copy of the
            file, specified in the pathname, to the server- or user-DTP
            at the other end of the data connection.

RMD - Remove directory

RENAME FROM (RNFR)

            This command specifies the old pathname of the file which is
            to be renamed.  This command must be immediately followed by
            a "rename to" command specifying the new file pathname.

RENAME TO (RNTO)

            This command specifies the new pathname of the file
            specified in the immediately preceding "rename from"
            command.  Together the two commands cause a file to be
            renamed.

STORE (STOR)

            This command causes the server-DTP to accept the data
            transferred via the data connection and to store the data as
            a file at the server site.  If the file specified in the
            pathname exists at the server site, then its contents shall
            be replaced by the data being transferred.  A new file is
            created at the server site if the file specified in the
            pathname does not already exist.

STORE UNIQUE (STOU)

            This command behaves like STOR except that the resultant
            file is to be created in the current directory under a name
            unique to that directory.  The 250 Transfer Started response
            must include the name generated.

STRU - Set file structure

REPRESENTATION TYPE (TYPE)

            The argument specifies the representation type. Can be I, A, or L

XCUP - Identical to CDUP

XCWD - Identical to CWD

XMKD - Identical to MKD

XPWD - Identical to PWD

XRMD - Identical to RMD

USER - Specify the username for the login

PASS - Specify the password for the login

QUIT - Close the connection

## Verb Formatting

USER <SP> <username> <CRLF>
PASS <SP> <password> <CRLF>
CWD  <SP> <pathname> <CRLF>
CDUP <CRLF>
QUIT <CRLF>
REIN <CRLF>
PORT <SP> <host-port> <CRLF>
PASV <CRLF>
TYPE <SP> <type-code> <CRLF>
STRU <SP> <structure-code> <CRLF>
MODE <SP> <mode-code> <CRLF>
RETR <SP> <pathname> <CRLF>
STOR <SP> <pathname> <CRLF>
STOU <CRLF>
REST <SP> <marker> <CRLF>
RNFR <SP> <pathname> <CRLF>
RNTO <SP> <pathname> <CRLF>
DELE <SP> <pathname> <CRLF>
RMD  <SP> <pathname> <CRLF>
MKD  <SP> <pathname> <CRLF>
PWD  <CRLF>
LIST [<SP> <pathname>] <CRLF>
NLST [<SP> <pathname>] <CRLF>
SITE <SP> <string> <CRLF>
SYST <CRLF>
STAT [<SP> <pathname>] <CRLF>
HELP [<SP> <string>] <CRLF>
NOOP <CRLF>

## Poller

The poller requires that the PORT and HOST environment variables are set. It also needs "testfile" to be in the directory of the poller in order to test the get and put functionality.