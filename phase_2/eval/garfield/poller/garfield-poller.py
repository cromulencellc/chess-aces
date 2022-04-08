import sys
import socket
import random
import time
import string
import base64
import os
import datetime
import string
import signal
import copy

pwd = ''
homedir = ''

def handler( signum, frame):
    print('[FAIL] sigalrm fired Poller failed')
    exit(1)

def randomstring(l):
    q = b''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase).encode('UTF-8')

    return q

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def RU( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def RL( s ):
    return RU( s, '\n')

def SW(s, data):
    s.sendall(data)

def RB(s):
    return RU(s, '>>> ')

ints_g = {}
doubles_g = {}
strings_g = {}
lists_g = {}

def create_int( s, value ):
    global ints_g

    RB(s)

    name = randomstring(7)

    cmd = b'int ' + name + b' = ' + str(value).encode('UTF-8') + b';'

    SW(s, cmd + b'\n')

    ints_g[name] = value

    print(f'\t[SENDING] {cmd}')

    return name

def print_int( s, name ):
    global strings_g
    global ints_g

    cmd = b''

    vs = name + b'_s'
    cmd += b'string ' + vs + b' = itos(' + name + b'); '
    strings_g[vs] = str(ints_g[name]).encode('UTF-8')

    cmd += b'puts(' + name + b'_s);'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    return

def create_double( s, value ):
    global doubles_g

    RB(s)

    name = randomstring(7)

    cmd = b'double ' + name + b' = ' + str(value).encode('UTF-8') + b';'

    SW(s, cmd + b'\n')

    doubles_g[name] = value

    print(f'\t[SENDING] {cmd}')

    return name

def print_double( s, name ):
    global strings_g
    global doubles_g

    cmd = b''

    vs = name + b'_s'
    cmd += b'string ' + vs + b' = dtos(' + name + b'); '

    v = '{:.6f}'.format(doubles_g[name])

    strings_g[vs] = str(v).encode('UTF-8')

    cmd += b'puts(' + name + b'_s);'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    return

def create_string( s, value ):
    global strings_g

    RB(s)

    name = randomstring(7)

    cmd = b'string ' + name + b' = "' + value + b'";'

    SW(s, cmd + b'\n')

    strings_g[name] = value

    print(f'\t[SENDING] {cmd}')

    return name

def print_string( s, name ):
    global strings_g

    cmd = b''

    if name not in strings_g:
        return

    cmd += b'puts(' + name + b');'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    return

def create_list( s ):
    global lists_g

    RB(s)

    name = randomstring(7)

    nl = []

    cmd = b'list ' + name + b' = ['

    for x in range(random.randint(1, 5)):
        r = random.randint(0,100)

        if r < 33:
            v = random.randint(-5,5)
            nl.append(v)
        elif r < 66:
            v = round(random.uniform(-5.0, 5.0), 2)
            nl.append(v)
        else:
            tp = randomstring(5)
            v = '"' + tp.decode('UTF-8') + '"'
            nl.append(tp)

        cmd += str(v).encode('UTF-8') + b', '

    cmd = cmd[:-2] + b' ];'

    SW(s, cmd + b'\n')

    lists_g[name] = nl

    print(f'\t[SENDING] {cmd}')

    return name

def print_list( s, name ):
    global lists_g

    cmd = b''

    if name not in lists_g:
        return

    cmd += b'putl(' + name + b');'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    return

def global_int( s ):
    global ints_g
    global strings_g

    print('[TEST] global_int')

    RB(s)

    varname = randomstring(6)
    value = random.randint( -5, 5)

    value_s = str(value).encode('UTF-8')

    cmd = b'int ' + varname + b' = ' + value_s + b';'
    cmd += b' string ' + varname + b'_s = itos(' + varname + b');'
    cmd += b' puts(' + varname + b'_s' + b');'

    ints_g[varname] = value
    strings_g[varname + b'_s'] = value_s


    SW(s, cmd + b'\n')

    ## confirm that it worked by converting it to a string and reading it back

    y = RL(s)

    if y != str(value) + '\n':
        print(f'[ERROR] expected: {str(value)} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_double( s ):
    global doubles_g
    global strings_g

    print('[TEST] global_double')

    RB(s)

    varname = randomstring(6)
    value = round( random.uniform(-5.0, 5.0), 2 )
    value_s = '{:.6f}'.format(value).encode('UTF-8')

    cmd = b'double ' + varname + b' = ' + value_s + b';'
    cmd += b' string ' + varname + b'_s = dtos(' + varname + b');'
    cmd += b' puts(' + varname + b'_s' + b');'

    doubles_g[varname] = value
    strings_g[varname + b'_s'] = value_s

    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != value_s + b'\n':
        print(f'[ERROR] expected: {value_s} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')
    
    return 0

def global_string( s ):
    global strings_g

    print('[TEST] global_string')

    RB(s)

    varname = randomstring(6)
    value = randomstring(10)
    cmd = b'string ' + varname + b' = "' + value + b'";'
    cmd += b' puts(' + varname + b');'

    strings_g[varname] = value

    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != value + b'\n':
        print(f'[ERROR] expected: {value} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')
    
    return 0

def global_list( s ):
    global lists_g
    print('[TEST] global_list')

    nm = create_list(s)

    cmd = b'putl(' + nm + b');'

    x = b'['

    for el in lists_g[nm]:
        if type(el) == type(0.0):
            x += '{:.6f}'.format(el).encode('UTF-8')
        elif type(el) == type(b"h"):
            x += b'"' + el + b'"'
        elif type(el) == type(1):
            x += str(el).encode('UTF-8')
        else:
            print(f"HKH: {el}")

        x += b', '

    x = x[:-2] + b']'

    RB(s)

    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != x + b'\n':
        print(f'[ERROR] expected: {x} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')
    
    return 0

def global_add_int( s ):
    global ints_g

    print('[TEST] global_add_int')

    assign_var = ''
    assign_val = 0

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'int ' + assign_var + b' = '
    else:
        if len(ints_g) == 0:
            assign_var = create_int(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(ints_g.keys()) )
            cmd += assign_var +b' = '

    operand_count = random.randint(2, 5)

    if len(ints_g) < operand_count:
        for x in range(operand_count - len(ints_g) ):
            create_int(s, random.randint(-5, 5) )

    for x in range(operand_count):
        ## select an integer or an existing variable
        if random.randint(0, 100) > 50:
            ## existing
            c = random.choice( list(ints_g.keys()) )
            cmd += c + b' + '
            assign_val += ints_g[c]
        else:
            c = random.randint(-5,5)
            cmd += str(c).encode('UTF-8') + b' + '
            assign_val += c

    ## just add this because
    cmd += b'0;'

    ints_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_int(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != str(assign_val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_sub_int( s ):
    global ints_g

    print('[TEST] global_sub_int')

    assign_var = ''
    assign_val = 0

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'int ' + assign_var + b' = '
    else:
        if len(ints_g) == 0:
            assign_var = create_int(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(ints_g.keys()) )
            cmd += assign_var +b' = '

    operand_count = random.randint(2, 5)

    if len(ints_g) < operand_count:
        for x in range(operand_count - len(ints_g) ):
            create_int(s, random.randint(-5, 5) )

    values = []

    if random.randint(0, 100) > 50:
        ## existing
        opa = random.choice( list(ints_g.keys()) )
        cmd += opa + b' - '
        values.append(ints_g[opa])
    else:
        opa = random.randint(-5,5)
        cmd += str(opa).encode('UTF-8') + b' - '
        values.append(opa)

    if random.randint(0, 100) > 50:
        ## existing
        opb = random.choice( list(ints_g.keys()) )
        cmd += opb + b';'
        values.append(ints_g[opb])
    else:
        opb = random.randint(-5,5)
        cmd += str(opb).encode('UTF-8') + b';'
        values.append(opb)

    assign_val = values[0] - values[1]

    ints_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_int(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != str(assign_val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_mul_int( s ):
    global ints_g

    print('[TEST] global_mul_int')

    assign_var = ''
    assign_val = 1

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'int ' + assign_var + b' = '
    else:
        if len(ints_g) == 0:
            assign_var = create_int(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(ints_g.keys()) )
            cmd += assign_var +b' = '

    operand_count = random.randint(2, 3)

    if len(ints_g) < operand_count:
        for x in range(operand_count - len(ints_g) ):
            create_int(s, random.randint(-5, 5) )

    for x in range(operand_count):
        ## select an integer or an existing variable
        if random.randint(0, 100) > 50:
            ## existing
            c = random.choice( list(ints_g.keys()) )
            cmd += c + b' * '
            assign_val *= ints_g[c]
        else:
            c = random.randint(-5,5)
            cmd += str(c).encode('UTF-8') + b' * '
            assign_val *= c

    ## just add this because
    cmd += b'1;'

    ints_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_int(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != str(assign_val).encode('UTF-8') +b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_div_int( s ):
    global ints_g

    print('[TEST] global_div_int')

    assign_var = ''
    assign_val = 1

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'int ' + assign_var + b' = '
    else:
        if len(ints_g) == 0:
            assign_var = create_int(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(ints_g.keys()) )
            cmd += assign_var +b' = '

    a = random.randint(1, 10)
    c = random.randint(1, 10)
    
    cmd += str(c).encode('UTF-8') + b' / ' + str(a).encode('UTF-8')
    assign_val = int(c / a)

    cmd += b';'

    ints_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_int(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != str(assign_val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_add_double( s ):
    global doubles_g

    print('[TEST] global_add_double')

    assign_var = ''
    assign_val = 0.0

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'double ' + assign_var + b' = '
    else:
        if len(doubles_g) == 0:
            assign_var = create_double(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(doubles_g.keys()) )
            cmd += assign_var +b' = '

    operand_count = random.randint(2, 5)

    if len(doubles_g) < operand_count:
        for x in range(operand_count - len(doubles_g) ):
            create_double(s, round(random.uniform(-5, 5), 4) )

    for x in range(operand_count):
        ## select an integer or an existing variable
        if random.randint(0, 100) > 50:
            ## existing
            c = random.choice( list(doubles_g.keys()) )
            cmd += c + b' + '
            assign_val += doubles_g[c]
        else:
            c = round( random.uniform(-5,5), 4)
            cmd += str(c).encode('UTF-8') + b' + '
            assign_val += c

    ## just add this because
    cmd += b'0.0;'

    doubles_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_double(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != '{:.6f}'.format(assign_val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_sub_double( s ):
    global doubles_g

    print('[TEST] global_sub_double')

    assign_var = ''
    assign_val = 0.0

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'double ' + assign_var + b' = '
    else:
        if len(doubles_g) == 0:
            assign_var = create_double(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(doubles_g.keys()) )
            cmd += assign_var +b' = '


    values = []

    if random.randint(0, 100) > 50:
        ## existing
        if len(doubles_g) == 0:
            create_double( s, round( random.uniform(-5.0, 5.0), 3) )

        opa = random.choice( list(doubles_g.keys()) )
        cmd += opa + b' - '
        values.append(doubles_g[opa])
    else:
        opa = round(random.uniform(-5,5), 4)
        cmd += str(opa).encode('UTF-8') + b' - '
        values.append(opa)

    if random.randint(0, 100) > 50:
        ## existing
        if len(doubles_g) == 0:
            create_double(s, round(random.uniform(-5,5), 2))

        opb = random.choice( list(doubles_g.keys()) )
        cmd += opb + b';'
        values.append(doubles_g[opb])
    else:
        opb = round( random.uniform(-5,5), 4)
        cmd += str(opb).encode('UTF-8') + b';'
        values.append(opb)


    assign_val = values[0] - values[1]
    doubles_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_double(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != '{:.6f}'.format(assign_val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_mul_double( s ):
    global doubles_g

    print('[TEST] global_mul_double')

    assign_var = ''
    assign_val = 1.0

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'double ' + assign_var + b' = '
    else:
        if len(doubles_g) == 0:
            assign_var = create_double(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(doubles_g.keys()) )
            cmd += assign_var +b' = '

    operand_count = random.randint(2, 3)

    if len(doubles_g) < operand_count:
        for x in range(operand_count - len(doubles_g) ):
            create_double(s, round(random.uniform(-5, 5), 4) )

    for x in range(operand_count):
        ## select an integer or an existing variable
        if random.randint(0, 100) > 50:
            ## existing
            c = random.choice( list(doubles_g.keys()) )
            cmd += c + b' * '
            assign_val *= doubles_g[c]
        else:
            c = round( random.uniform(-5,5), 4)
            cmd += str(c).encode('UTF-8') + b' * '
            assign_val *= c

    ## just add this because
    cmd += b'1.0;'

    doubles_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_double(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != '{:.6f}'.format(assign_val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_div_double( s ):
    global doubles_g

    print('[TEST] global_div_double')

    assign_var = ''
    assign_val = 0.0

    cmd = b''

    if len(doubles_g) == 0:
        create_double( s, assign_val )

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'double ' + assign_var + b' = '
    else:
        if len(ints_g) == 0:
            assign_var = create_double(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(doubles_g.keys()) )
            cmd += assign_var +b' = '

    a = round( random.uniform(1, 10), 4)
    c = round( random.uniform(1, 10), 4)
    
    cmd += str(c).encode('UTF-8') + b' / ' + str(a).encode('UTF-8')
    assign_val = c / a

    cmd += b';'

    doubles_g[assign_var] = assign_val

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_double(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != '{:.6f}'.format(assign_val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_add_string( s ):
    global strings_g

    print('[TEST] global_add_string')

    assign_var = ''
    assign_val = b''

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'string ' + assign_var + b' = '
    else:
        if len(strings_g) == 0:
            assign_var = create_string(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(strings_g.keys()) )
            cmd += assign_var +b' = '

    operand_count = random.randint(2, 5)

    if len(strings_g) < operand_count:
        for x in range(operand_count - len(strings_g) ):
            create_string(s, randomstring( random.randint(5, 9)) )


    values = []

    if random.randint(0, 100) > 50:
        ## existing
        c = random.choice( list(strings_g.keys()) )
        cmd += c + b' + '
        values.append(strings_g[c])
    else:
        c = randomstring( random.randint(5, 9))
        cmd += b'"' + c + b'" + '
        values.append(c)

    if random.randint(0, 100) > 50:
        ## existing
        c = random.choice( list(strings_g.keys()) )
        cmd += c + b' + '
        values.append(strings_g[c])
    else:
        c = randomstring( random.randint(5, 9))
        cmd += b'"' + c + b'" + '
        values.append(c)


    ## just add this because
    cmd += b'"";'

    assign_val = values[0] + values[1]
    strings_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_string(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != assign_val + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def global_mul_string( s ):
    global strings_g

    print('[TEST] global_mul_string')

    assign_var = ''
    assign_val = b''

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'string ' + assign_var + b' = '
    else:
        if len(strings_g) == 0:
            assign_var = create_string(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(strings_g.keys()) )
            cmd += assign_var +b' = '

    if random.randint(0, 100) > 50:
        ## existing
        if len(strings_g) == 0:
            create_string( s, randomstring(5) )

        c = random.choice( list(strings_g.keys()) )
        cmd += c + b' * '
        assign_val = strings_g[c]
    else:
        c = randomstring( random.randint(5, 9))
        cmd += b'"' + c + b'" * '
        assign_val += c

    ## just add this because
    val = random.randint(2, 5)
    cmd += str(val).encode('UTF-8') +b';'

    assign_val *= val
    strings_g[assign_var] = assign_val;

    RB(s)

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    print_string(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != assign_val + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_gets( s ):
    global strings_g

    print('[TEST] builtin_gets')

    varname = randomstring(8)

    RB(s)

    cmd = b'string ' + varname + b' = gets();'

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    var = b''

    for x in range( 1, random.randint(3, 5)):
        var += randomstring( random.randint(3, 7)) + b' '

    var = var.rstrip(b' ')

    print(f'\t[SENDING] {var}')
    SW(s, var + b'\n')

    RB(s)

    cmd = b'puts( ' + varname + b' );'

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != var + b'\n':
        print(f'[ERROR] expected {var} received {y}')
        return 1

    return 0

def builtin_len( s ):
    print('[TEST] builtin_len')

    global strings_g
    global ints_g

    assign_var = randomstring( 6 )
    assign_val = 0

    if len(strings_g) == 0:
        create_string(s, b'');

    if len(ints_g) == 0:
        create_int(s, 0)

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'int ' + assign_var + b' = '
    else: 
        if len(strings_g) == 0:
            assign_var = create_int(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(ints_g.keys()) )
            cmd += assign_var +b' = '

    if random.randint(0, 100) > 50:
        ## existing
        c = random.choice( list(strings_g.keys()) )
        cmd += b'len(' + c + b');'
        assign_val = len(strings_g[c])
    else:
        c = randomstring( random.randint(5, 9))
        cmd += b'len(\"' + c + b'\");'
        assign_val = len(c)

    ints_g[assign_var] = assign_val

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    print_int(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != str(assign_val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_atoi( s ):
    print('[TEST] builtin_atoi')

    global strings_g
    global ints_g

    assign_var = ""
    assign_val = 0

    cmd = b''

    ## decide if assigning to a new variable
    if random.randint(0, 100) > 50:
        assign_var = randomstring( 6 )
        cmd += b'int ' + assign_var + b' = '
    else: 
        if len(ints_g) == 0:
            assign_var = create_int(s, assign_val)
            cmd += assign_var +b' = '
        else:
            assign_var = random.choice( list(ints_g.keys()) )
            cmd += assign_var +b' = '

    val = random.randint(-5, 5)
    c = str(val).encode('UTF-8')
    cmd += b'atoi(\"' + c + b'\");'
    assign_val = val

    ints_g[assign_var] = assign_val

    RB(s)

    SW(s, cmd + b'\n')

    print_int(s, assign_var)

    y = RL(s).encode('UTF-8')

    if y != str(val).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {assign_val} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_substr( s ):
    print('[TEST] builtin_substr')

    global strings_g

    RB(s)

    varname = randomstring(5)
    value = randomstring(20)

    cmd = b'string ' + varname + b' = "' + value + b'";'

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    strings_g[varname] = value

    RB(s)

    start = random.randint( len(value) / 4, 3 * len(value) / 4)
    sl = random.randint( 1, len(value) - start )

    result = value[start: start + sl]

    newvar = randomstring(5)

    cmd = b'string ' + newvar + b' = substr(' + varname + b', ' + str(start).encode('UTF-8') + b', ' + str(sl).encode('UTF-8') + b');'

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    strings_g[newvar] = result

    print_string(s, newvar)

    y = RL(s).encode('UTF-8')

    if y != result + b'\n':
        print(f'[ERROR] expected: {result} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_strstr( s ):
    print('[TEST] builtin_strstr')
    global strings_g

    if len(strings_g) == 0:
        create_string(s, randomstring(20))

    value = ''

    while len(value) == 0:
        varname = random.choice( list(strings_g.keys()) )
        value = strings_g[varname]

    ## pick an index in the first half
    index = random.randint(0, int((len(value) - 1)/ 2))

    ## pick an end in the second half
    end = random.randint( index + 1, len(value) )

    RB(s)

    sub = strings_g[varname][index:end]

    loc = strings_g[varname].find(sub)

    print(f'[INFO] {strings_g[varname]} -- {sub}')
    cmd = b'puti( strstr( ' + varname + b', "' + sub + b'") );'

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != str(loc).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected: {loc} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_setchr( s ):
    print('[TEST] builtin_setchr')
    global strings_g

    if len(strings_g) == 0:
        create_string(s, randomstring(20))

    value = ''

    while len(value) == 0:
        varname = random.choice( list(strings_g.keys()) )
        value = strings_g[varname]

    index = random.randint(0, len(value) - 1)

    c = randomstring(1)

    RB(s)

    cmd = b'setchr(' + varname + b', "' + c + b'", ' + str(index).encode('UTF-8') +b'); puts(' + varname + b');'

    z = copy.deepcopy(value)
    z = z[0:index] + c + z[index+1:]

    strings_g[varname] = z

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != z + b'\n':
        print(f'[ERROR] expected: {z} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_type( s ):
    print('[TEST] builtin_type')

    global strings_g
    global ints_g
    global doubles_g
    global lists_g

    r = random.randint(0, 100)

    if r < 25:
        if len(strings_g) == 0:
            create_string(s, randomstring(10))

        var = random.choice( list( strings_g.keys()))

        expected = b'string'
    elif r < 50:
        if len(ints_g) == 0:
            create_int(s, random.randint(-5,5))

        var = random.choice( list( ints_g.keys()))

        expected = b'int'
    elif r < 25:
        if len(doubles_g) == 0:
            create_double(s, round( random.uniform(-5.0, 5.0), 2))

        var = random.choice( list( doubles_g.keys()))

        expected = b'double'
    else:
        if len(lists_g) == 0:
            create_list(s )

        var = random.choice( list( lists_g.keys()))

        expected = b'list'

    RB(s)

    cmd = b'puts( type(' + var + b'));'

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != expected + b'\n':
        print(f'[ERROR] expected: {expected} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_hex( s ):
    print('[TEST] builtin_hex')

    cmd = b'puts( hex( '

    if random.randint(0, 100) > 50:
        if len(ints_g) == 0:
            var = create_int(s, random.randint(-30, 30) )
        else:
            var = random.choice( list(ints_g.keys()) )

        cmd += var

        expected = hex(ints_g[var]).encode('UTF-8')
    else:
        val = random.randint(-30, 30)

        cmd += str(val).encode('UTF-8')

        expected = hex(val).encode('UTF-8')

    cmd += b'));'


    print(f'\t[SENDING] {cmd}')

    RB(s)
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != expected + b'\n':
        print(f'[ERROR] expected: {expected} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_sum( s ):
    print('[TEST] builtin_sum')

    cmd = b'puti( sum ( ['

    count = random.randint( 3, 6 )

    expected = 0

    for x in range(count):
        if random.randint( 0, 100) > 50:
            if len(ints_g) == 0:
                var = create_int(s, random.randint(-5, 5))
            else:
                var = random.choice( list(ints_g.keys()))

            expected += ints_g[var]

            cmd += var + b', '
        else:
            val = random.randint(-5, 5)
            expected += val

            cmd += str(val).encode('UTF-8') + b', '

    cmd = cmd.rstrip(b', ')

    cmd += b' ] ));'

    RB(s)
    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    expected = str(expected).encode('UTF-8')

    if y != expected + b'\n':
        print(f'[ERROR] expected: {expected} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_max( s ):
    print('[TEST] builtin_max')

    i = random.randint(-5, 5)
    j = random.randint(-5, 5)

    expected = str(max(i, j)).encode('UTF-8')

    RB(s)

    cmd = b'puti( max( ' + str(i).encode('UTF-8') + b', ' + str(j).encode('UTF-8') + b') );'

    print(f'\t[SENDING] {cmd}')

    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != expected + b'\n':
        print(f'[ERROR] expected: {expected} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0


def ltos( l ):
    x = b'['

    for el in l:
        if type(el) == type(0.0):
            x += '{:.6f}'.format(el).encode('UTF-8')
        elif type(el) == type(b"h"):
            x += b'"' + el + b'"'
        elif type(el) == type(1):
            x += str(el).encode('UTF-8')
        else:
            print(f"HKH: {el}")

        x += b', '

    x = x.rstrip(b', ')

    x += b']'

    return x

def builtin_strtok( s ):
    if len(strings_g) == 0:
        var = create_string(s, randomstring(20))
    else:
        var = random.choice( list( strings_g.keys()))

    val = strings_g[var]

    if len(val) < 5:
        return 0

    print('[TEST] builtin_strtok')

    c = chr(random.choice(list(val))).encode('UTF-8')

    toks = [ x for x in val.split(c) if x.strip() ]

    cmd = b'putl( strtok( ' + var + b', "' + c + b'"));'

    print(f'\t[SENDING] {cmd}')
    RB(s)
    SW(s, cmd + b'\n')

    tp = ltos(toks)

    y = RL(s).encode('UTF-8')

    if y != tp + b'\n':
        print(f'[ERROR] expected: {tp} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_append(s):
    print('[TEST] builtin_append')

    global lists_g

    if len(lists_g) == 0:
        create_list(s)

    ## select a list
    var = random.choice( list( lists_g.keys()))

    b = random.randint( -5, 5)

    cmd = b'append(' + var + b', ' + str(b).encode('UTF-8') + b');'

    print(f'\t[SENDING] {cmd}')

    RB(s)

    SW(s, cmd + b'\n')

    lists_g[var].append(b)

    tp = ltos( lists_g[var] )

    cmd = b'putl(' + var + b');'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != tp + b'\n':
        print(f'[ERROR] expected: {tp} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_prepend(s):
    print('[TEST] builtin_prepend')

    global lists_g

    if len(lists_g) == 0:
        create_list(s)

    ## select a list
    var = random.choice( list( lists_g.keys()))

    b = random.randint( -5, 5)

    cmd = b'prepend(' + var + b', ' + str(b).encode('UTF-8') + b');'

    print(f'\t[SENDING] {cmd}')

    RB(s)

    SW(s, cmd + b'\n')

    lists_g[var] = [b] + lists_g[var]

    tp = ltos( lists_g[var] )

    cmd = b'putl(' + var + b');'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != tp + b'\n':
        print(f'[ERROR] expected: {tp} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_popend(s):
    print('[TEST] builtin_popend')

    global lists_g

    if len(lists_g) == 0:
        create_list(s)

    ## select a list
    var = random.choice( list( lists_g.keys()))

    ## TODO I should actually either push one on or choose another list
    if len(lists_g[var])== 0:
        return 0

    b = random.randint( -5, 5)

    cmd = b'popend(' + var + b');'

    print(f'\t[SENDING] {cmd}')

    RB(s)

    SW(s, cmd + b'\n')

    lists_g[var].pop()

    tp = ltos( lists_g[var] )

    cmd = b'putl(' + var + b');'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != tp + b'\n':
        print(f'[ERROR] expected: {tp} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_popfront(s):
    print('[TEST] builtin_popfront')

    global lists_g

    if len(lists_g) == 0:
        create_list(s)

    ## select a list
    var = random.choice( list( lists_g.keys()))

    ## TODO I should actually either push one on or choose another list
    if len(lists_g[var])== 0:
        return 0

    b = random.randint( -5, 5)

    cmd = b'popfront(' + var + b');'

    print(f'\t[SENDING] {cmd}')

    RB(s)

    SW(s, cmd + b'\n')

    lists_g[var] = lists_g[var][1:]

    tp = ltos( lists_g[var] )

    cmd = b'putl(' + var + b');'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')
    print(y)

    if y != tp + b'\n':
        print(f'[ERROR] expected: {tp} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_getbyindex(s):
    print('[TEST] builtin_getbyindex')

    if len(lists_g) == 0:
        var = create_list(s)
    else:
        var = random.choice(list(lists_g.keys()))

    if len(lists_g[var]) == 1:
        i = 0
    elif len(lists_g[var]) == 0:
        var = create_list(s)
        i = random.randint(0, len(lists_g[var]) - 1)
    else:
        i = random.randint(0, len(lists_g[var]) - 1)

    cmd = b'puts(str( getbyindex(' + var + b', ' + str(i).encode('UTF-8') + b')));'

    RB(s)

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    val = lists_g[var][i]

    if type(val) == type(0.0):
        d = '{:.6f}'.format(val).encode('UTF-8')
    elif type(val) == type(b"h"):
        d = b'"' + val + b'"'
    elif type(val) == type(1):
        d = str(val).encode('UTF-8')

    y = RL(s).encode('UTF-8')

    if y != d + b'\n':
        print(f'[ERROR] expected: {d} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_exists(s):
    if len(lists_g) == 0:
        var = create_list(s)
    else:
        var = random.choice(list(lists_g.keys()))

    if len(lists_g[var]) == 0:
        return 0

    print('[TEST] builtin_exists')

    if random.randint(0, 100) > 50:
        v = random.choice( lists_g[var] )

        if type(v) == type(b''):
            v = b'"' + v + b'"'
        else:
            v = str(v).encode('UTF-8')

        cmd = b'puti( exists (' + var + b', ' + v + b'));'

        expected = b'1'
    else:
        cmd = b'puti( exists (' + var + b', "hello world"));'

        expected = b'0'

    print(f'\t[SENDING] {cmd}')

    RB(s)
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != expected + b'\n':
        print(f'[ERROR] expected: {expected} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def builtin_erase(s):
    print('[TEST] builtin_erase')

    if len(lists_g) == 0:
        var = create_list(s)
    else:
        var = random.choice( list(lists_g.keys()))

    if len(lists_g[var]) == 0:
        return 0

    index = random.randint( 0, len(lists_g[var]) - 1)

    cmd = b'erase (' + var + b', ' + str(index).encode('UTF-8') + b'); puti(len( ' + var + b'));'

    RB(s)
    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    del lists_g[var][index]

    expected = str(len(lists_g[var])).encode('UTF-8')

    y = RL(s).encode('UTF-8')

    if y != expected + b'\n':
        print(f'[ERROR] expected: {expected} received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def eq_if_check( s ):
    print('[TEST] eq_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' == ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' == ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'after\n':
        print(f'[ERROR] expected: "after" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def neq_if_check( s ):
    print('[TEST] neq_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s) 

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' != ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' != ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'after\n':
        print(f'[ERROR] expected: "after" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def lt_if_check( s ):
    print('[TEST] lt_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' < ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' < ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'after\n':
        print(f'[ERROR] expected: "after" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def lte_if_check( s ):
    print('[TEST] lte_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' <= ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' <= ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'after\n':
        print(f'[ERROR] expected: "after" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def gt_if_check( s ):
    print('[TEST] gt_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' > ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' > ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'after\n':
        print(f'[ERROR] expected: "after" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def gte_if_check( s ):
    print('[TEST] gte_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' >= ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: "true" received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' >= ' + varb + b') { puts("true"); }; puts("after");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'after\n':
        print(f'[ERROR] expected: "after" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def eq_ifelse_check( s ):
    print('[TEST] eq_ifelse_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' == ' + varb + b') { puts("true"); } else { puts("false"); };'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        print(y)
        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' == ' + varb + b') { puts("true"); } else { puts("false"); };'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y != b'false\n':
            print(f'[ERROR] expected: "false" received: {y}')
            return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def neq_ifelse_check( s ):
    print('[TEST] neq_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' != ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' != ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'false\n':
        print(f'[ERROR] expected: "false" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def lt_ifelse_check( s ):
    print('[TEST] lt_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)
    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' < ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' < ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y  != b'false\n':
        print(f'[ERROR] expected: "false" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def lte_ifelse_check( s ):
    print('[TEST] lte_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' <= ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' <= ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'false\n':
        print(f'[ERROR] expected: "false" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def gt_ifelse_check( s ):
    print('[TEST] gt_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' > ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' > ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y  != b'false\n':
        print(f'[ERROR] expected: "false" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def gte_ifelse_check( s ):
    print('[TEST] gte_if_check')

    global ints_g

    vara = randomstring(7)
    varb = randomstring(7)

    RB(s)

    ## true
    if random.randint(0, 100) > 50:
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' >= ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

        y = RL(s).encode('UTF-8')

        if y + b'\n' == b'true\n':
            print(f'[ERROR] expected: {result} received: {y}')
            return 1
    else:
        ## false
        z = random.randint(-5, 5)
        y = random.randint(6, 10)

        cmd = b'int ' + vara + b' = ' + str(z).encode('UTF-8') + b'; '
        cmd += b'int ' + varb + b' = ' + str(y).encode('UTF-8') + b'; '
        cmd += b'if (' + vara + b' >= ' + varb + b') { puts("true"); }; puts("false");'

        print(f'\t[SENDING] {cmd}')

        SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'false\n':
        print(f'[ERROR] expected: "false" received: {y}')
        return 1

    print(f'\t[SUCCESS]: {cmd}')

    return 0

def while_check( s ):
    print('[TEST] while_check')

    count = random.randint(3, 5)

    varname = randomstring(6)

    cmd = b'int ' + varname + b' = 0; '

    cmd += b'while ( ' + varname + b' != ' + str(count).encode('UTF-8') + b') {'
    cmd += b' puti( ' + varname + b'); ' + varname + b' = ' + varname + b' + 1; };'

    print(f'\t[SENDING] {cmd}')

    RB(s)

    SW(s, cmd + b'\n')

    for x in range( count ):
        y = RL(s).encode('UTF-8')

        if y != str(x).encode('UTF-8') + b'\n':
            print(f'[ERROR] expected: {x} received {y}')
            return 1

    print(f'\t[SUCCESS] {cmd}')

def for_check( s ):
    print('[TEST] for_check')

    start = random.randint(3, 5)
    end = random.randint( 10, 15 )

    varname = randomstring(6)

    cmd = b'int ' + varname + b' = 0; '

    cmd += b'for ( ' + varname + b' = ' + str(start).encode('UTF-8') + b' ; '
    cmd += varname + b' < ' + str(end).encode('UTF-8') + b' ; '
    cmd += varname + b' = ' + varname + b' + 1 ) { '
    cmd += b' puti( ' + varname + b'); };'

    print(f'\t[SENDING] {cmd}')

    RB(s)

    SW(s, cmd + b'\n')

    for x in range( start, end ):
        y = RL(s).encode('UTF-8')

        if y != str(x).encode('UTF-8') + b'\n':
            print(f'[ERROR] expected: {x} received {y}')
            return 1

    print(f'\t[SUCCESS] {cmd}')

def define_int_function(s):
    print('\t[TEST] define_int_function')

    fname = randomstring( 8 )
    argone = randomstring(4)
    argtwo = randomstring(4)

    cmd = b'int ' + fname + b'( int ' + argone + b', int ' + argtwo + b' ) {'
    cmd += b' return ' + argone + b' + ' + argtwo + b'; };'

    print(f'\t[SENDING] {cmd}')
    RB(s)
    SW(s, cmd + b'\n')

    RB(s)

    a = random.randint(-5,5)
    b = random.randint(-5,5)

    cmd = b'puti( ' + fname + b'(' + str(a).encode('UTF-8') + b', ' + str(b).encode('UTF-8') + b'));'

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')
    
    y = RL(s).encode('UTF-8')

    if y != str(a + b).encode('UTF-8') + b'\n':
        print(f'[ERROR] expected {a + b} received {y}')
        return 1

    return 0

def define_double_function(s):
    print('\t[TEST] define_double_function')
    
    fname = randomstring( 8 )
    argone = randomstring(4)
    argtwo = randomstring(4)

    cmd = b'double ' + fname + b'( double ' + argone + b', double ' + argtwo + b' ) {'
    cmd += b' return ' + argone + b' + ' + argtwo + b'; };'

    print(f'\t[SENDING] {cmd}')
    RB(s)
    SW(s, cmd + b'\n')

    RB(s)

    a = round( random.uniform(-5.0,5.0), 3 )
    b = round( random.uniform(-5.0,5.0), 3 )

    cmd = b'putd( ' + fname + b'(' + str(a).encode('UTF-8') + b', ' + str(b).encode('UTF-8') + b'));'

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')
    
    y = RL(s).encode('UTF-8')

    d = '{:.6f}'.format(a + b).encode('UTF-8')

    if y != d + b'\n':
        print(f'[ERROR] expected {d} received {y}')
        return 1

    return 0

def define_string_function(s):
    print('\t[TEST] define_string_function')
    
    fname = randomstring( 8 )
    argone = randomstring(4)
    argtwo = randomstring(4)

    cmd = b'string ' + fname + b'( string ' + argone + b', string ' + argtwo + b' ) {'
    cmd += b' return ' + argone + b' + ' + argtwo + b'; };'

    print(f'\t[SENDING] {cmd}')
    RB(s)
    SW(s, cmd + b'\n')

    RB(s)

    a = randomstring( 9 )
    b = randomstring( 9 )

    cmd = b'puts( ' + fname + b'("' + a + b'", "' + b + b'"));'

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')
    
    y = RL(s).encode('UTF-8')

    d = a + b

    if y != d + b'\n':
        print(f'[ERROR] expected {d} received {y}')
        return 1

    return 0

def define_function_check( s ):
    print('[TEST] define_function_check')

    f = random.choice( [define_double_function, define_int_function, define_string_function])

    return f(s)

'''
These functions do the error checking portion
'''

def invalid_return_check( s ):
    print('[TEST] invalid_return_check')

    fname = randomstring( 8 )
    argone = randomstring(4)

    if random.randint(0, 100) > 50:
        var = round( random.uniform(-5.0, 5.0), 2)
    else:
        var = random.randint( -5, 5)


    cmd = b'string ' + fname + b'( string ' + argone + b' ) {'
    cmd += b' return ' + str(var).encode('UTF-8') + b'; }; '

    cmd += fname + b'( "' + randomstring(5) + b'");'

    print(f'\t[SENDING] {cmd}')

    RB(s)
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'Unknown type encountered: return value is not the same as the defined function return type\n':
        print(f'[ERROR] received {y}')
        return 1

    return 0

def no_expected_return_check( s ):
    print('[TEST] no_expected_return_check')

    fname = randomstring( 8 )
    argone = randomstring(4)

    if random.randint(0, 100) > 50:
        var = round( random.uniform(-5.0, 5.0), 2)
    else:
        var = random.randint( -5, 5)


    cmd = b'string ' + fname + b'( string ' + argone + b' ) {'
    cmd += argone + b' = "' + randomstring(6) + b'" + ' + argone + b'; }; '

    cmd += fname + b'( "' + randomstring(5) + b'");'

    print(f'\t[SENDING] {cmd}')

    RB(s)
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'Unknown type encountered: function expected a return value\n':
        print(f'[ERROR] received {y}')
        return 1

    return 0

def func_return_check( s ):
    print('[TEST] invalid_return_check')

    fname = randomstring( 8 )
    argone = randomstring(4)

    if random.randint(0, 100) > 50:
        var = round( random.uniform(-5.0, 5.0), 2)
    else:
        var = random.randint( -5, 5)


    cmd = b'void ' + fname + b'( string ' + argone + b' ) {'
    cmd += b' return ' + str(var).encode('UTF-8') + b'; }; '

    cmd += fname + b'( "' + randomstring(5) + b'");'

    print(f'\t[SENDING] {cmd}')

    RB(s)
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'Unknown type encountered: return value is not the same as the defined function return type\n':
        print(f'[ERROR] received {y}')
        return 1

    return 0

def invalid_type_return_check( s ):
    print('[TEST] invalid_type_return_check')

    ivt = randomstring(4)
    fname = randomstring( 8 )
    argone = randomstring(4)

    if random.randint(0, 100) > 50:
        var = round( random.uniform(-5.0, 5.0), 2)
    else:
        var = random.randint( -5, 5)


    cmd = ivt + b' ' + fname + b'( string ' + argone + b' ) {'
    cmd += b' return ' + str(var).encode('UTF-8') + b'; }; '

    cmd += fname + b'( "' + randomstring(5) + b'");'

    print(f'\t[SENDING] {cmd}')

    RB(s)
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'Unknown type encountered: invalid type in function declaration\n':
        print(f'[ERROR] received {y}')
        return 1

    return 0

def invalid_funccall_check( s ):
    print('[TEST] invalid_funccall_check')

    fname = randomstring( 8 )
    argone = randomstring(4)
    argtwo = randomstring(4)

    cmd = b'string ' + fname + b'( string ' + argone + b', string ' + argtwo + b' ) {'
    cmd += b' return ' + argone + b' + ' + argtwo + b'; };'

    print(f'\t[SENDING] {cmd}')
    RB(s)
    SW(s, cmd + b'\n')

    RB(s)

    a = randomstring( 9 )
    b = random.randint(-5,5)

    cmd = fname + b'("' + a + b'", ' + str(b).encode('UTF-8') + b');'

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y != b'Unknown type encountered: int\n':
        print(f'[ERROR] received {y}')
        return 1

    return 0

def invalid_argcount_check( s ):
    print('[TEST] invalid_argcount_check')

    fname = randomstring( 8 )
    argone = randomstring(4)
    argtwo = randomstring(4)

    cmd = b'string ' + fname + b'( string ' + argone + b', string ' + argtwo + b' ) {'
    cmd += b' return ' + argone + b' + ' + argtwo + b'; };'

    print(f'\t[SENDING] {cmd}')
    RB(s)
    SW(s, cmd + b'\n')

    RB(s)

    a = randomstring( 9 )

    cmd = fname + b'("' + a + b'");'

    print(f'\t[SENDING] {cmd}')
    SW(s, cmd + b'\n')

    y = RL(s).encode('UTF-8')

    if y.find(b'invalid function argument count:') == -1:
        print(f'[ERROR] received {y}')
        return 1

    return 0

def main():
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print('[ERROR] HOST and PORT must be specified via arguments or variables.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(100)

    try:
        s = connect( HOST, PORT )
    except:
        print('[ERROR] Failed to connect to: %s:%d' %(HOST, PORT))
        exit(1)

    fs = [builtin_strtok, builtin_max, builtin_sum, builtin_hex, builtin_gets, builtin_erase, builtin_exists, builtin_getbyindex,builtin_popfront, builtin_popend, builtin_prepend, builtin_append, global_list, builtin_type, builtin_strstr, for_check, invalid_argcount_check, invalid_funccall_check, invalid_type_return_check, func_return_check, no_expected_return_check, invalid_return_check, define_function_check, while_check, gte_ifelse_check, gt_ifelse_check, lte_ifelse_check, lt_ifelse_check, neq_ifelse_check, eq_ifelse_check, gte_if_check, lte_if_check, gt_if_check, lt_if_check, neq_if_check, eq_if_check, builtin_setchr, builtin_substr, builtin_atoi, builtin_len, global_int, global_double, global_string, global_add_int, global_sub_int, global_mul_int, global_div_int, global_add_double, global_sub_double, global_mul_double, global_div_double, global_add_string, global_mul_string ]
    #fs = [ builtin_strtok ]

    for _ in range(100):
        d = random.choice( fs )

        if d( s ):
            print('FAILURE: ')
            print(d)
            SW(s, b'exit(0)\n')
            s.close()
            exit(0)

    print('[SUCCESS] Poller completed!')
    SW(s, b'exit(0);\n')
    s.close()

if __name__ == '__main__':
    main()
