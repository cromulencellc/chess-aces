# KNOWN BUGS

## Spoofable data on all busses [CWE-923](https://cwe.mitre.org/data/definitions/923.html)

Traffic on busses is unencrypted, and available to all devices. Of specific importance, all devices have access to the, "Privileged bus," and can spoof any message on this bus as there is no authentication mechanism for traffic.

## Race conditions on the bus

A device may see a message on the bus, and _potentially_ may be able to spoof a crafted a response before the correct device can respond.

## Access Control Device

### Hardcoded credentials [CWE-798](https://cwe.mitre.org/data/definitions/798.html)

`devices/challenge/acd2/acd2_store.c`

"Keys," for the access control device are hard-coded. If someone can understand bus traffic, they can derive the format of access requests, and then look for keys in the access control device binary. They can then recover additional keys for other devices not communicating on the bus.

### Tokens receive all available permissions [CWE-266](https://cwe.mitre.org/data/definitions/266.html)

`devices/challenge/acd2/acd2.c`

The permissions for a key are checked against those given in the request, but then the full permissions for that key are made available to the token. If the key has the requested permissions, the token should only receive the requested permissions.

## Sensitive Information on the Maintenance Bus

The MCD transmits keys on the maintenance bus which, if observed, would allow players to perform arbitrary actions. While the RCD cannot see the maintenance bus _in the configuration the binary and challenge originally comes in_, the binary can be modified, and/or the source can be modified and recompiled.

Re-running the binary then allows the sniffing of sensitive keys over the maintenance bus using code supplied in the poller.