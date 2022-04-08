---
title: "mg_set_protocol_dns()"
decl_name: "mg_set_protocol_dns"
symbol_kind: "func"
signature: |
  void mg_set_protocol_dns(struct mg_connection *nc);
---

Attaches a built-in DNS event handler to the given listening connection.

The DNS event handler parses the incoming UDP packets, treating them as DNS
requests. If an incoming packet gets successfully parsed by the DNS event
handler, a user event handler will receive an `MG_DNS_REQUEST` event, with
`ev_data` pointing to the parsed `struct mg_dns_message`.

See
[captive_dns_server](https://github.com/cesanta/mongoose/tree/master/examples/captive_dns_server)
example on how to handle DNS request and send DNS reply. 

