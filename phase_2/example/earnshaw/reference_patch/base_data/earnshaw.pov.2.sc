set args /service/vsftpd.conf
set disassembly-flavor intel
set follow-fork-mode child
set height 0
r
x /i $pc
i r $ymm0 $rdi
c
quit
