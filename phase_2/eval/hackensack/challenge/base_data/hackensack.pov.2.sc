set args -c /data/mosquitto.conf
set disassembly-flavor intel
set height 0
r
x /i $pc
i r $r15
x /gx $rsp
c
quit
