---
title: "mg_connect_ws_opt()"
decl_name: "mg_connect_ws_opt"
symbol_kind: "func"
signature: |
  struct mg_connection *mg_connect_ws_opt(
      struct mg_mgr *mgr, MG_CB(mg_event_handler_t ev_handler, void *user_data);
---

Helper function that creates an outbound WebSocket connection

Mostly identical to `mg_connect_ws`, but allows to provide extra parameters
(for example, SSL parameters) 

