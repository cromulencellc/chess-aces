# Nethia

Nethia is an injection challenge based upon unrealircd v6.0.2.

Unrealircd is a common IRC server capable of serving many clients.

## Scope
The main server as well as all modules are in scope.
For a comprehensive view of the modules see "modules.custom.conf" under base_data.
Each module loaded is associated with a <module_name>.so.

## POV 1
CWE-170 - Improper NULL termination

CWE-200: Exposure of Sensitive Information to an Unauthorized Actor

In the custom date command a user can specify a formatted date string that
returns the date/time of the server. However, the server does not null 
terminate the string so an attacker is able to leak addresses using a custom
date format.

Since it is unknown where in memory with relation to the buffer an address lies the reference PoV attempts the leak multiple times with increasingly long strings. Multiple address guesses are printed at least one of which will be a valid address. To confirm this connect via ``docker exec`` and view the process mapping with the command ``cat /proc/`pidof ngircd`/maps``.

## POV 2
CWE-787 Out of bounds write
CWE-123 Write-what-where condition

Using the KNOCK command an attacker can cause a write-what-where condition
due to an overflow in the Client structure.

The reference PoV results in a SIGSEGV with an arbitrary write with an attacker controlled destination and source.

## POV 3
CWE-287: Improper Authentication

This pov leverages a bug at the beginning of the CMD_FUNC function inside modulemanager_irc.c. The module checks for permissions with the ValidatePermissionsPath function. This function allows Servers, Ulines, or an IRC Operator to use the module. In the configuration file there is a linked server called irc.other.org. With the server being linked in the configuration file this server can be spoofed to bypass the permission check. In this pov, after bypassing the authentication check, it starts a http server and adds it to the sources list on the unrealircd server. From here the module hosted on the pov's http server can be installed. It then rehashes the server making the installed module active. The module installed, POV_mod, then reads the token file and sends it back to the POV proving a structured information disclosure. 
