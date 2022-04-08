set args -X -n -c /home/challenge/proftpd-1.3.7a/sample-configurations/basic.conf
set disassembly-flavor intel
set height 0
r
x /i $pc
x /gx $rsp
i r $rbp
c
quit
