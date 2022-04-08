set args -f /data/slapd.conf
set disassembly-flavor intel
set height 0
set follow-fork-mode child
r
x /i $pc
i r $r15
x /gx $rsp
c
quit
