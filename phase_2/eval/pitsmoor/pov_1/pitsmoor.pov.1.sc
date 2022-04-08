set disassembly-flavor intel
set height 0
r args -n -f ./doc/sample-ngircd.conf
x /gx $rsp
i r $r15
c
quit
