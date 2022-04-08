set args -s
set disassembly-flavor intel
set height 0
r
x /i $pc
i r $rcx $rdi
c
quit
