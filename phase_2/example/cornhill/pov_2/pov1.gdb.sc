set args -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" -h $HOST -p $PORT
r
x /i $pc
i r $r8 $rsi
x /gx $rsp
quit
