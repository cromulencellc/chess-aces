set disassembly-flavor intel
set height 0
c
x /i $pc
i r $ymm0 $rdi
c
quit
