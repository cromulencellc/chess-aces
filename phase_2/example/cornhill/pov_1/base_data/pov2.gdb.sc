set args -x -D "cn=admin,dc=chess,dc=com", -w secret -f /data/pov2.ldif -h $HOST -p $PORT
r
x /i $pc
i r $r15
x /gx $rsp
quit