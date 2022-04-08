set args -x -D "cn=admin,dc=chess,dc=com", -w secret -f /home/chess/pov2.ldif -h pov_server -p 9999
python
def ig(cmd):
  try:
    gdb.execute(cmd)
  except:
    pass

ig("r")
ig("x /i $pc")
ig("i r $r15")
ig("x /gx $rsp")
ig("quit")