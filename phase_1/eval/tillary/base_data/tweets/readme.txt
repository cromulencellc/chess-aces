# tweets

this directory stores short status items

status items are named as {decimal-encoded epoch seconds}-{random-slug}.tweet

their content is:
* one byte indicating status length in bytes
* that many bytes of utf-8 text

this directory must be cleaned out between poller runs
