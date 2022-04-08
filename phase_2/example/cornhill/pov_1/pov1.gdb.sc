set args -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" -h pov_server -p 9999
r
x /i $pc
i r $r8 $rsi
x /gx $rsp
quit
