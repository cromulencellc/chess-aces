---
title: "mg_url_decode()"
decl_name: "mg_url_decode"
symbol_kind: "func"
signature: |
  int mg_url_decode(const char *src, int src_len, char *dst, int dst_len,
                    int is_form_url_encoded);
---

Decodes a URL-encoded string.

Source string is specified by (`src`, `src_len`), and destination is
(`dst`, `dst_len`). If `is_form_url_encoded` is non-zero, then
`+` character is decoded as a blank space character. This function
guarantees to NUL-terminate the destination. If destination is too small,
then the source string is partially decoded and `-1` is returned.
*Otherwise,
a length of the decoded string is returned, not counting final NUL. 

