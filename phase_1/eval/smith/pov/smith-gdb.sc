set pagination off
set logging file smith-log.txt
set logging on
r
x /i $pc
i r $rbp
x /gx $rsp
set logging off
quit

