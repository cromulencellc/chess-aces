# Olmstead

Olmstead is an injection vulnerability in openldap 2.4.56

## Scope for Analysis

This challenge processes LDAP messages which of course are in scope. Authentication is done via the ``chess`` user with password ``chess`` which a regular user can be expected to know. There is also an admin account which should be considered privileged and should not be known by regular users. The LDAP database is already populated with data but the PoV MUST NOT expect any specific content aside from the chess and the admin accounts. 

Both the server binary ``slapd`` and its libraries are in scope for analysis.

## LDAP (from Wikipedia)

The Lightweight Directory Access Protocol (LDAP /ˈɛldæp/) is an open, vendor-neutral, industry standard application protocol for accessing and maintaining distributed directory information services over an Internet Protocol (IP) network. Directory services play an important role in developing intranet and Internet applications by allowing the sharing of information about users, systems, networks, services, and applications throughout the network. As examples, directory services may provide any organized set of records, often with a hierarchical structure, such as a corporate email directory. Similarly, a telephone directory is a list of subscribers with an address and a phone number.

LDAP is specified in a series of Internet Engineering Task Force (IETF) Standard Track publications called Request for Comments (RFCs), using the description language ASN.1. The latest specification is Version 3, published as RFC 4511.

A common use of LDAP is to provide a central place to store usernames and passwords. This allows many different applications and services to connect to the LDAP server to validate users.

LDAP is based on a simpler subset of the standards contained within the X.500 standard. Because of this relationship, LDAP is sometimes called X.500-lite.

## ldapsearch

./ldapsearch -D "cn=admin,dc=chess,dc=com" -x -w secrets -b "ou=people,dc=chess,dc=com"

## ldapcompare

./ldapcompare -d 65535 -D "cn=admin,dc=chess,dc=com" -x -w secret -v "cn=Hawaii John,ou=people,dc=chess,dc=com" sn:Yodog

## ldapmodify

./ldapmodify -h localhost -D "cn=admin,dc=chess,dc=com" -w secret -f ~/ldap-data/mod.ldif -a


## Launching the slapd server

./slapd -d 65535 -f <path to config>

## Launching the poller

The poller expects the HOST, PORT, and TOOLS environment variables or command line arguments in that order.

olmstead.py <host> <port> <tools>

The tools option specifies where to find the ldap client tools such as ldapmodify and ldapsearch.


## POV 1

CWE-787: Out-of-bounds Write

When sending a modify request on an invalid value there is a vulnerability in a call to snprintf that allows an attacker to overflow a stack-based buffer and gain control of a stored return address.

## POV 2

CWE-305 Authentication Bypass by Primary Weakness
CWE-697 Incorrect Comparison
Effects: Authentication, Information Disclosure, Logic Error

This is similar to POV 1 of hackensack but it was injected into a library instead of into the main executable.

#ifdef PATCHED_2
		return ( passwd->bv_len == cred->bv_len ) ?
			memcmp( passwd->bv_val, cred->bv_val, passwd->bv_len )
			: 1;
#else
			return memcmp( passwd->bv_val, cred->bv_val, cred->bv_len);
#endif

The cred->bv_len is the length of the password sent by the user.



