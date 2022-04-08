## Access Control Protocol

The Access Control Protocol (ACP) works through streaming Bus Protocol messages, and provides interoperability between the ACD and other JS-DOFSS devices. The ACP exchanges 16-byte keys for 8-byte, randomly generated tokens, and allows other devices to verify permissions against those tokens.

A JS-DOFSS device sends the ACD its 16-byte key, along with its requested permissions, via an Access Token Creation Request. If the key is valid, and the key has access to the associated permissions, the ACD generates a random 8-byte token and returns it in an Access Token Creation Response. This token can be used in requests to other devices, proving the requesting device has the appropriate permissions without revealing the device's key.

This interaction is modeled in the below diagram:

```
Client               ACD                 Server
--------------------------------------------------
   |                   |                    |
Request Token--------->|                    |
   |             Validates Token            |
   |               Request                  |
   |                   |                    |
   |<-------------Issues Token              |
   |                   |                    |
Requests action        |                    |
with Token -------------------------------->|
   |                   |                    |
   |                   |<------------- Validates Token
   |                   |                    |
   |             Validates Token ---------->|
   |                   |                    |
   |                   |             Performs Action
   |<--------------------------------Sends Response
```

There are four message types implemented by the ACP. They are:

* Access Token Creation Request
* Access Token Creation Response
* Access Token Verification Request
* Access Token Verification Response

Each request and response begins with a 1-byte header, denoting the message type. The values for each message type are:

```
ACCESS_TOKEN_CREATE_REQUEST        = 0
ACCESS_TOKEN_CREATE_RESPONSE       = 1
ACCESS_TOKEN_VERIFICATION_REQUEST  = 2
ACCESS_TOKEN_VERIFICATION_RESPONSE = 3
```

Also important are the various values for access types. Access types denote the various permissions a key may hold, and a token is valid for. The access type is an 8-bit value with the following fields:

```
Bit Index:    0 1 2 3 4 5 6 7
Name:         F D U U C M U V

C: ACCESS_TYPE_COMMUNICATIONS
D: ACCESS_TYPE_DEBUG
F: ACCESS_TYPE_FILESYSTEM
M: ACCESS_TYPE_MAINTENANCE
U: Unused
V: ACCESS_TYPE_VALID
```

The access type is only valid if the `ACCESS_TYPE_VALID` bit is set. If the `ACCESS_TYPE_VALID` bit is set, the token has the permissions represented by the other set bits in the access type.

### Access Token Creation Request

```
+---------------------+--------------------+-------------+
| 1-byte Request Type | 1-byte Acesss Type | 16-byte key |
+---------------------+--------------------+-------------+
```

The Access Token Creation Request consists of 1 byte, indicating the request type, 1-byte indiciating the permissions requested, at the 16-byte key from the requesting device.

The Request Type will be `ACCESS_TOKEN_CREATE_REQUEST`, and will always be set to `0`.

### Access Token Creation Response

```
+----------------------+--------------------+--------------+
| 1-byte Response Type | 1-byte Acesss Type | 8-byte Token |
+----------------------+--------------------+--------------+
```

The Access Token Creation Response consists of 1 byte, indicating the response type, 1-byte indiciating the permissions requested, and the 8-byte generated token when the request is valid.

The Response Type will be `ACCESS_TOKEN_CREATE_RESPONSE`, and will always be set to `1`.

The access type will have the `ACCESS_TYPE_VALID` bit set if the request is valid, as well as the associated permissions. If the `ACCESS_TYPE_VALID` bit is set, the 8-byte token will be given in the token field. If the `ACCESS_TYPE_VALID` bit is not set, the 8-byte token field is undefined.

### Access Token Verification Request

```
+---------------------+--------------------+--------------+
| 1-byte Request Type | 1-byte Acesss Type | 8-byte Token |
+---------------------+--------------------+--------------+
```

The Access Token Verification Request consists of 1 byte, indicating the request type, 1-byte indiciating the permissions which need to be verified, and the 8-byte token to verify.

The Request Type will be `ACCESS_TOKEN_VERIFICATION_REQUEST`, and will always be set to `2`.

### Access Token Verification Response

```
+----------------------+--------------------+--------------+
| 1-byte Response Type | 1-byte Acesss Type | 8-byte Token |
+----------------------+--------------------+--------------+
```

The Access Token Verification Response consists of 1 byte, indicating the response type, 1-byte indiciating the verified permissions, and the 8-byte token which was verified.

The Response Type will be `ACCESS_TOKEN_VERIFICATION_RESPONSE`, and will always be set to `3`.

If the requested access type is valid for the token, the requested permissions will be returned in the access type with the `ACCESS_TYPE_VALID` bit set. The 8-byte token field will always return the token given in the request.


