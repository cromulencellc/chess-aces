# cornhill

Cornhill borrows the client tools and libraries from the open source software OpenLDAP. While the slapd server is also provided as part of this challenge, it does not contain any vulnerabilities injected by the CHESS team.

This challenge is different than previous ones in that the vulnerabilities are inserted into already existing code as opposed to writing the entire application from scratch. Remember that diffing is not a permissible method of bug finding for CHESS so don't download the original and compare the two.

Both the clients applications and their linked libraries are in scope.

None of the data is considered unstructured privileged. You are given the admin password. Adding, modifying, or deleting entries is also not considered privileged.

LDAP uses the ldif format which can be seen in the hj.ldif and the yolo.ldif files under the poller folder.

Version used: https://www.openldap.org/software/download/OpenLDAP/openldap-release/openldap-2.4.50.tgz

## Using docker

For the poller, you first need to launch the slapd server.

docker-compose -f docker-compose.yaml build ta3_cornhill_server
docker-compose -f docker-compose.yaml up ta3_cornhill_server

In another window build and run the image containing the clients. This launches an ssh server that will connected to via the poller container.

docker-compose -f docker-compose.yaml build ta3_cornhill
docker-compose -f docker-compose.yaml up ta3_cornhill

In a separate window build and run the poller:

docker-compose -f docker-compose.yaml build ta3_cornhill_poller
docker-compose -f docker-compose.yaml up ta3_cornhill_poller

# Building the clients

You should receive a source tree that is already configured appropriately. To build the challenge change into the challenge folder and simply type 'make'. 

The built client tools will be under challenge/clients/tools. However, these are just scripts that actually refrence the binaries in challenge/clients/tools/.libs/

# slapd

The server that was used for testing the clients is slapd which is also provided by OpenLDAP. It can be found under the slapd directory in ldap_server/openldap-2.4.50.

Building is the same as for the clients.

slapd launch command: 
sudo ./slapd -d 65535 -F <path-to-cornhill>/cornhill/ldap_server/server_data/

The slapd server must be running prior to launching any of the tools. The default port is 389.

# Client tools

There are 9 open ldap client tools that are built under client/tools/ directory. Only four of these are actually tested by the poller: ldapcompare, ldapwhoami, ldapmodify, and ldapsearch.

The two that contain vulnerabilties are ldapmodify and ldapsearch.

## ldapcompare

ldapcompare opens a connection to an LDAP server, binds, and performs a compare using specified parameters.

./ldapcompare -x -D "cn=admin,dc=chess,dc=com" -w secret "uid=hawaiijohn,ou=People,dc=chess,dc=com" -h <target_host> -p <target_port> sn:John
TRUE

./ldapcompare -x -D "cn=admin,dc=chess,dc=com" -w secret "uid=hawaiijohn,ou=People,dc=chess,dc=com" -h <target_host> -p <target_port> sn:Yolo
FALSE

## ldapwhoami

ldapwhoami opens a connection to an LDAP server, binds, and performs a whoami operation.

./ldapwhoami -x -D "cn=admin,dc=chess,dc=com" -w secret -h <target_host> -p <target_port>
dn:cn=admin,dc=chess,dc=com

./ldapwhoami -x -D "uid=hawaiijohn,ou=People,dc=chess,dc=com" -w secret -h <target_host> -p <target_port>
dn:uid=hawaiijohn,ou=People,dc=chess,dc=com

./ldapwhoami -x -D "uid=hawaiijohn,ou=People,dc=chess,dc=com" -w s3cret -h <target_host> -p <target_port>
ldap_bind: Invalid credentials (49)

## ldapmodify

ldapmodify opens a connection to an LDAP server, binds, and modifies or adds entries. The entry information is read from standard input or from file through the use of the -f option.

ldapmodify -x -D "cn=admin,dc=chess,dc=com" -w secret -f modify.ldif -h <target_host> -p <target_port>

There are four ways that you can modify an entry: add, replace, delete, increment

## The following entries don't actually exist in the provided server config

dn: uid=sbrown20,ou=People,dc=example,dc=com
changetype: modify
add: mail
mail: sbrown@example.com

dn: uid=sbrown20,ou=People,dc=example,dc=com
changetype: modify
replace: mail
mail: sbrown2@example.com

dn: uid=sbrown20,ou=People,dc=example,dc=com
changetype: modify
increment: employeeNumber

dn: uid=jsmith1,ou=People,dc=example,dc=com
changetype: modify
delete: mail
mail: jsmith1@example.com

## ldapsearch

ldapsearch opens a connection to an LDAP server, binds, and performs a search using specified parameters. The filter should conform to the string representation for search filters as defined in RFC 4515. If not provided, the default filter, (objectClass=*), is used.

If ldapsearch finds one or more entries, the attributes specified by attrs are returned. If * is listed, all user attributes are returned. If + is listed, all operational attributes are returned. If no attrs are listed, all user attributes are returned. If only 1.1 is listed, no attributes will be returned.

The search results are displayed using an extended version of LDIF. Option -L controls the format of the output.

./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" -h <target_host> -p <target_port>

## POV

LDAP encodes its communication in the Basic Encoding Rules (BER) format which is a format for encoding ASN.1 data. Simplisticly if is a type-length-value format.

### Launching POVs in docker

First you need to launch the pov server:

docker-compose -f docker-compose.yaml build ta3_cornhill_pov_#_server
docker-compose -f docker-compose.yaml up ta3_cornhill_pov_#_server

In another window build and run the image containing the clients. This launches an ssh server that will connected to via the pov launchers.

docker-compose -f docker-compose.yaml build ta3_cornhill
docker-compose -f docker-compose.yaml up ta3_cornhill

In a separate window build and run the pov clients:

docker-compose -f docker-compose.yaml build ta3_cornhill_pov_#_client
docker-compose -f docker-compose.yaml up ta3_cornhill_pov_#_client


#### Reference patch

If you want to launch the patched version instead of the vulnerable one run:

docker-compose -f docker-compose.yaml build ta3_cornhill_ref_patch
docker-compose -f docker-compose.yaml up ta3_cornhill_ref_patch

### POV 1


CWE-691: Insufficient Control Flow Management
CWE-758: Reliance on Undefined, Unspecified, or Implementation-Defined Behavior

Within the lber library there is a file called decode.c. This contains most of the code responsible for parsing the BER encoded data. The function that we are interested in is ber_scanf(). It behaves much like the scanf() library function. It accepts some format specifiers and stores data from the stream according to the specified types. In the unmodifed opensource application there is a "!" format specifier that requests for a callback function to be parsed out of the data stream. This is odd in and of itself but it wasn't ever used in any of the other libraries or tools. I added some additional code to the parser and added the ability to reach it through the ldapsearch tool. Below are the CHESS additions:

#### Code Injection
decode.c:

+ber_tag_t
+ber_get_callback(
+    BerElement *ber,
+    BERDecodeCallback **f )
+{
+    ber_tag_t   tag;
+    ber_len_t   len;
+    struct berval bv;
+
+    assert( f != NULL );
+
+    tag = ber_peek_element( ber, &bv );
+
+    len = bv.bv_len;
+
+    if ( tag == LBER_DEFAULT || len != sizeof(BERDecodeCallback) ) {
+        return LBER_DEFAULT;
+    }
+    
+    if( len ) {
+        *f = (BERDecodeCallback *) bv.bv_val;
+
+    } else {
+        *f = NULL;
+    }
+
+    return tag;
+}

...
                BERDecodeCallback *f;
                void *p;

                f = va_arg( ap, BERDecodeCallback * );
                p = va_arg( ap, void * );

+               rc = ber_get_callback( ber, &f);
                
+               if ( rc == LBER_DEFAULT || *f == NULL) {
+                   break;
+               } else {
                    rc = (*f)( ber, p, 0 );
+               }
            } break;
...

include/lber.h

+LBER_F( ber_tag_t )
+ber_get_callback LDAP_P((
+    BerElement *ber,
+    BERDecodeCallback **f ));

ldapsearch.c

+            case LDAP_RES_CALLBACK:
+                ldap_parse_callback( ld, msg );
+                break;

include/ldap.h
+#define LDAP_RES_CALLBACK  ((ber_tag_t) 0x6aU) /* application + constructed */

#### Exploit

When the user connects to the PoV "server" the attacker responds with ..

### POV 2

The vulnerability here is an arithmetic error resulting from failing to check for negative values. It is possible for an attacker to set the index to be written as negative and overwrite data lower on the stack than the errmsgp variable. I also added a new scanf type 'L' that reads in 8 bytes integers. The patch for this is straight forward. You just have to have a check for >= 0.

CWE-697: Incorrect Comparison

CWE-839: Numeric Range Comparison Without Minimum Check


#### Code Injections

ldapmodify.c
-   rc = ldap_parse_result( ld, res, &err, &matched, &text, &refs, &ctrls, 1 );
+   rc = ldap_parse_mod_result( ld, res, &err, &matched, &text, &refs, &ctrls, 1 );

error.c
I am not going to put the entire function here because I just copied ldap_parse_result() with only the following modification:
-   int rc = ldap_pvt_controls( ber, errmsgp, serverctrls );
+   int rc = ldap_pvt_mod_controls( ber, errmsgp, serverctrls );

controls.c

The ldap_pvt_controls() function expects a tag type of 0xa0 which indicates that it is a tag use for controls. I added a subtype within the controls that handle error messages.
I again copied the entire function but the following changes are the relevant ones:

+        if( tag == LBER_MODERR ) {
+            tag = ber_scanf( ber, "i", &ber_mod_info_index );
+
+            if ( ber_mod_info_index > 12 ) {
+                return LDAP_DECODING_ERROR;
+            }
+        }
+
+        if( tag == LBER_MODERRM ) {
+            if ( ber_mod_info_index > 12 ) {
+                return LDAP_DECODING_ERROR;
+            }
+
+            tag = ber_scanf( ber, "L", &errmsgp[ber_mod_info_index]);
+        }

include/lber.h

+#define LBER_MODERR         ((ber_tag_t) 0x06UL)
+#define LBER_MODERRM        ((ber_tag_t) 0x07UL)

decode.c

+        case 'L':   /* long */
+            l = va_arg( ap, ber_len_t * );
+            rc = ber_get_long( ber, l );
+            break;

#### Exploit

The exploit waits for a connection from a client. It then accepts the bind and regardless of the sent data responds with the PoC. It requires four tags. The first one sends a negative index to the location of a saved r15 value and the second one does the overwrite of r15, the third is a negative indes to the location of a return address, and finally, the fourth overwrites the saved pc. All four of these can be send in a single response due to how the BER parsing of the control codes works.

