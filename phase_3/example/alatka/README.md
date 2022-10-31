# Alatka

Injection vulnerability in MaraDNS.

MaraDNS handles DNS requests via UDP port 53. 

## Overview
Externality: MaraDNS

Language:	C

Protocol:	DNS

## Interacting with the server using command line tools

The easiest way to interact is via the tool ``dig``. Check out the ``man`` page for detailed usage but here are a few examples:

```
dig darpachess.com @127.0.0.1 -t CNAME

; <<>> DiG 9.11.3-1ubuntu1.15-Ubuntu <<>> darpachess.com @127.0.0.1 -t CNAME
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 44730
;; flags: qr aa rd; QUERY: 1, ANSWER: 1, AUTHORITY: 1, ADDITIONAL: 1
;; WARNING: recursion requested but not available

;; QUESTION SECTION:
;darpachess.com.			IN	CNAME

;; ANSWER SECTION:
darpachess.com.		86400	IN	CNAME	www.darpachess.com.

;; AUTHORITY SECTION:
darpachess.com.		86400	IN	NS	ns1.darpachess.com.

;; ADDITIONAL SECTION:
ns1.darpachess.com.	86400	IN	A	10.0.2.15

;; Query time: 0 msec
;; SERVER: 127.0.0.1#53(127.0.0.1)
;; WHEN: Tue Jul 20 17:53:17 UTC 2021
;; MSG SIZE  rcvd: 84
```

Replace ``@127.0.0.1`` with the destination DNS server. The ``-t`` option specifies what type of record to return. Some common ones include ``CNAME``, ``TXT``, ``AAAA``, ``MX``, ``A``, and ``NS``.

For a full list of the records see the files ``darpachess.com``, and ``darpachess.org`` under ``base_data``. None of this data is considered privileged information.

## Scope for Analysis

The server ``maradns`` is in scope. Only the data in ``/token`` is considered privileged data.

# POVs Apogee Only

## POV 1

CWE 400 - Uncontrolled Resource Consumption
Denial of Service due to memory consumption

Under certain circumstances the MaraDNS server fails to free blocks of memory that have been allocated. An attacker can leverage this to consume enough memory to force the server to shut down.

In order to trigger the vulnerability the attacker must send a valid request containing multiple targets which, while allowed by the protocol, is typically considered to be invalid. Each request that fails results in an allocation of 4512 bytes that is not freed. On average, after 600 or so requests of this type the server will be unable to allocate memory and will shut down. When this happens the following message will be displayed.

```
Aieeeeee, can not allocate memory (increase max_mem maybe?)
```

