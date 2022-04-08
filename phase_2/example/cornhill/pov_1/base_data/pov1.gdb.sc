set args -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" -h pov_server -p 9999
python
def ig(cmd):
  try:
    gdb.execute(cmd)
  except:
    pass

ig("r")
ig("x /i $pc")
ig("i r $r8 $rsi")
ig("x /gx $rsp")
ig("quit")