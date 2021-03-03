import random
import os
import socket
import string
import sys
import numpy

variables = {}

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def rs(l):
    z = ''

    for _ in range(l):
        z += random.choice( string.ascii_lowercase )

    return z

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('[ERROR]')
        print (e)
        return None

    return str(z)

def readline( s ):
    return readuntil( s, '\n')

def readprompt(s):
    return readuntil(s, '>>> ')

## used for writing logs
def sendwrapper(s, data ):
    s.send(data.encode('utf-8'))

def create_named_scalar_matrix( s ):
    global variables

    varname = rs(7)

    val = float(random.randint(-5, 5))

    data = varname + ' = ' + str(val)

    sendwrapper(s,data + '\n')

    variables[varname] = [[val]];

    print("[TEST] create_named_scalar_matrix -- %s" %(data) )

    try:
        x = readuntil(s, '= \n')
    except:
        print('[ERROR] create_named_scalar_matrix -- failed to read to "="')
        return 0

    expectation = varname + ' = \n'

    if x != expectation:
        print('[ERROR] expected %s ; received %s' %(expectation, x) )
        return 0

    expectation = '\n\t' + "{0:.3f}".format(val) + ' ' + '\n\n'

    try:
        x = readuntil(s, '\n\n')
    except:
        print('ERROR] create_named_scalar_matrix -- failed to read "\n\n"')
        return 0

    if x != expectation:
        print('[ERROR] expected %s ; received %s' %(expectation, x) )
        return 0

    readprompt(s)

    print('[SUCCESS] create_named_scalar_matrix')

    return 1

## returns a tuple: list of lists and the string to declare it
def genmatrix( rows, cols, start_interval=-5, end_interval=5 ):
    nm = []

    data = "[ "

    for i in range(rows):
        c = []
        for j in range(cols):
            element = float(random.randint(start_interval, end_interval))

            data += str(element) + " "

            c.append(element)

        if i < rows - 1:
            data += '; '

        nm.append(c)

    data += ']'

    return (nm, data)

def create_matrix(s):
    global variables

    varname = rs(7)

    A, data = genmatrix( random.randint(2,5), random.randint(2,5) )

    data = varname + ' = ' + data

    print('[TEST] create_matrix %d x %d -- ' %(len(A), len(A[0])) + data)

    sendwrapper(s, data + '\n')

    variables[varname] = A

    try:
        x = readuntil(s, '= \n')
    except:
        print('[ERROR] create_matrix -- failed to read to "="')
        return 0

    expectation = varname + ' = \n'

    if x != expectation:
        print('[ERROR] expected %s ; received %s' %(expectation, x) )
        return 0

    expectation = ''

    for row in A:
        expectation += '\n\t'

        for val in row:
            expectation += "{0:.3f}".format(val) + ' '


    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, x) )
        return 0

    readprompt(s)

    print('[SUCCESS] create_matrix')
    return 1

def check_named_matrix(s):
    global variables 

    if len(variables) == 0:
        check_named_matrix(s)

    print('[TEST] check_named_matrix')

    y = random.choice([*variables.keys()])

    sendwrapper(s,y + '\n')

    nm = variables[y]

    expectation = ''

    for row in nm:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            expectation += "{0:.3f}".format(val) + ' '


    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] check_named_matrix')

def transpose_matrix(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        nm = variables[varname]
        cmd += varname + "'"
    else:
        nm, data = genmatrix( random.randint(2,10), random.randint(2,10))
        cmd += data + "'"

    sendwrapper(s,cmd + '\n')

    print('[TEST] transpose_matrix -- %s' %(cmd) )

    tp = numpy.array(nm).transpose().tolist()

    if store_matrix:
        variables[newvarname] = tp

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] transpose_matrix -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in tp:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] transpose_matrix')

    return 1

def scalar_matrix_exp(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    nm, data = genmatrix( random.randint(2,10), random.randint(2,10), -5, 5)

    cmd += data + " ^ "

    ### I need a scalar
    sc = random.randint(1, 3)

    cmd += str(sc)

    A = numpy.power( numpy.array(nm), sc ).tolist()

    sendwrapper(s,cmd + '\n')

    print('[TEST] scalar_matrix_exp -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = A

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] scalar_matrix_exp -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in A:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] scalar_matrix_exp')

    return 1

def dot_hat_matrix(s):
    global variables

    cmd = ''

    varname = ''

    A, data = genmatrix( random.randint(2,5), random.randint(2,5))

    cmd += data + " .^ "

    ## I need a matrix of the same size
    B, data = genmatrix( len(A), len(A[0]), -3, 3 )

    C = numpy.power( numpy.array(A), numpy.array(B) ).tolist()

    cmd += data

    sendwrapper(s,cmd + '\n')

    print('[TEST] dot_hat_matrix -- %s' %(cmd) )

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            if val == -float('Inf'):
                val = float('Inf')

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] dot_hat_matrix')

    return 1

def scalar_multiply_matrix(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
    else:
        A, data = genmatrix( random.randint(2,10), random.randint(2,10))

    ## I need a scalar
    B = float(random.randint(-5,5))

    ## decide which side the scalar is on
    if random.randint(0,100) > 50:
        ## scalar first
        cmd += str(B) + ' * '

        if use_existing:
            cmd += varname
        else:
            cmd += data
    else:
        if use_existing:
            cmd += varname
        else:
            cmd += data

        cmd += ' * ' + str(B)

    C = ( numpy.array(A) * numpy.array(B) ).tolist()

    sendwrapper(s,cmd + '\n')

    print('[TEST] scalar_multiply_matrix -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] scalar_multiply_matrix -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0;
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] scalar_multiply_matrix')

    return 1

def multiply_matrices(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        cmd += varname + ' * '
    else:
        A, data = genmatrix( random.randint(2,10), random.randint(2,10))
        cmd += data + ' * '

    ## I need a new matrix of specific size
    B, data = genmatrix( len(A[0]), random.randint(2,5) )

    cmd += data
    C = ( numpy.array(A).dot( numpy.array(B) )).tolist()

    sendwrapper(s,cmd + '\n')

    print('[TEST] multiply_matrices -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] multiply_matrices -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] multiply_matrices')

    return 1

def scalar_add_matrix(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
    else:
        A, data = genmatrix( random.randint(2,10), random.randint(2,10))

    ## I need a scalar
    B = float(random.randint(-5,5))

    ## decide which side the scalar is on
    if random.randint(0,100) > 50:
        ## scalar first
        cmd += str(B) + ' + '

        if use_existing:
            cmd += varname
        else:
            cmd += data
    else:
        if use_existing:
            cmd += varname
        else:
            cmd += data

        cmd += ' +' + str(B)

    C = ( numpy.array(A) + numpy.array(B) ).tolist()

    sendwrapper(s,cmd + '\n')

    print('[TEST] scalar_add_matrix -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] scalar_add_matrix -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] scalar_add_matrix')

    return 1

def add_func(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        Adata = varname
    else:
        A, Adata = genmatrix( random.randint(2,10), random.randint(2,10))

    size_selection = random.randint(0,100)

    B, Bdata = genmatrix(len(A), len(A[0]))
    
    ## decide order

    use_func = (random.randint(0,100) > 50)

    if random.randint(0,100) > 50:
        ## A first
        cmd += '%add( ' + Adata + ', ' + Bdata + ')'
    else:
        cmd += '%add( ' + Bdata + ', ' + Adata + ')'

    C = (numpy.array(A) + numpy.array(B)).tolist()

    sendwrapper(s, cmd + '\n')

    print('[TEST] add_func -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] add_func -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] add_func')

    return 1

def add_matrices(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        Adata = varname
    else:
        A, Adata = genmatrix( random.randint(2,10), random.randint(2,10))

    size_selection = random.randint(0,100)

    if size_selection > 66:
        ## same rows
        B, Bdata = genmatrix( len(A), 1)
    elif size_selection > 33:
        ## same cols
        B, Bdata = genmatrix( 1, len(A[0]))
    else:
        ## same both
        B, Bdata = genmatrix(len(A), len(A[0]))
    
    ## decide order
    if random.randint(0,100) > 50:
        ## A first
        cmd += Adata + ' + ' + Bdata
    else:
        cmd += Bdata + ' + ' + Adata

    C = (numpy.array(A) + numpy.array(B)).tolist()

    sendwrapper(s, cmd + '\n')

    print('[TEST] add_matrices -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] add_matrices -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] add_matrices')

    return 1

def scalar_sub_matrix(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
    else:
        A, data = genmatrix( random.randint(2,10), random.randint(2,10))

    ## I need a scalar
    B = float(random.randint(-5,5))

    ## decide which side the scalar is on
    if random.randint(0,100) > 50:
        ## scalar first
        cmd += str(B) + ' - '

        if use_existing:
            cmd += varname
        else:
            cmd += data

        C = ( numpy.array(B) - numpy.array(A) ).tolist()
    else:
        if use_existing:
            cmd += varname
        else:
            cmd += data

        cmd += ' - ' + str(B)

        C = ( numpy.array(A) - numpy.array(B) ).tolist()

    sendwrapper(s,cmd + '\n')

    print('[TEST] scalar_sub_matrix -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] scalar_sub_matrix -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] scalar_sub_matrix')

    return 1

def reshape(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        Adata = varname
    else:
        A, Adata = genmatrix( random.randint(2,10), random.randint(2,10))

    new_row = len(A[0])
    new_col = len(A)

    ## create the reshape matrix
    nv = rs(7)

    rscmd = nv + ' = [' + str(new_row) + ' ' + str(new_col) + ' ] '

    cmd += '%reshape( ' + Adata + ', ' + nv + ')'

    sendwrapper( s, rscmd + '\n')
    readprompt(s)

    sendwrapper(s, cmd + '\n')

    print('[TEST] reshape -- %s' %(cmd) )

    C = numpy.reshape( numpy.array(A), (len(A[0]), len(A)))

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] reshape -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] reshape')

    return 1

def sub_func(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        Adata = varname
    else:
        A, Adata = genmatrix( random.randint(2,10), random.randint(2,10))

    size_selection = random.randint(0,100)

    B, Bdata = genmatrix(len(A), len(A[0]))
    
    ## decide order
    if random.randint(0,100) > 50:
        ## A first
        cmd += '%sub( ' + Adata + ', ' + Bdata + ' )'
        
        C = (numpy.array(A) - numpy.array(B)).tolist()

    else:
        cmd += '%sub( ' + Bdata + ', ' + Adata + ' )'

        C = (numpy.array(B) - numpy.array(A)).tolist()

    
    sendwrapper(s, cmd + '\n')

    print('[TEST] sub_func -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] sub_func -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            if val == -float('Inf'):
                val = float('Inf')

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] sub_func')

    return 1

def sub_matrices(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        Adata = varname
    else:
        A, Adata = genmatrix( random.randint(2,10), random.randint(2,10))

    size_selection = random.randint(0,100)

    if size_selection > 66:
        ## same rows
        B, Bdata = genmatrix( len(A), 1)
    elif size_selection > 33:
        ## same cols
        B, Bdata = genmatrix( 1, len(A[0]))
    else:
        ## same both
        B, Bdata = genmatrix(len(A), len(A[0]))
    
    ## decide order

    if random.randint(0,100) > 50:
        ## A first
        cmd += Adata + ' - ' + Bdata
        C = (numpy.array(A) - numpy.array(B)).tolist()

    else:
        cmd += Bdata + ' - ' + Adata

        C = (numpy.array(B) - numpy.array(A)).tolist()

    
    sendwrapper(s, cmd + '\n')

    print('[TEST] sub_matrices -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] sub_matrices -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            if val == -float('Inf'):
                val = float('Inf')

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] sub_matrices')

    return 1

def dot_multiply_matrices(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        Adata = varname
    else:
        A, Adata = genmatrix( random.randint(2,10), random.randint(2,10))

    size_selection = random.randint(0,100)

    if size_selection > 66:
        ## same rows
        B, Bdata = genmatrix( len(A), 1)
    elif size_selection > 33:
        ## same cols
        B, Bdata = genmatrix( 1, len(A[0]))
    else:
        ## same both
        B, Bdata = genmatrix(len(A), len(A[0]))
    
    ## decide order

    if random.randint(0,100) > 50:
        ## A first
        cmd += Adata + ' .* ' + Bdata
    else:
        cmd += Bdata + ' .* ' + Adata

    C = (numpy.array(A) * numpy.array(B)).tolist()

    sendwrapper(s, cmd + '\n')

    print('[TEST] dot_multiply_matrices -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] dot_multiply_matrices -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] dot_multiply_matrices')

    return 1

def dot_divide_matrices(s):
    global variables

    newvarname = ''
    cmd = ''

    ## I'm not going to use existing because I only want to gen non zero elements
    A, Adata = genmatrix( random.randint(2,10), random.randint(2,10), 1, 5)

    size_selection = random.randint(0,100)

    if size_selection > 66:
        ## same rows
        B, Bdata = genmatrix( len(A), 1, 1, 5)
    elif size_selection > 33:
        ## same cols
        B, Bdata = genmatrix( 1, len(A[0]), 1, 5)
    else:
        ## same both
        B, Bdata = genmatrix(len(A), len(A[0]), 1, 5)
    
    ## decide order

    if random.randint(0,100) > 50:
        ## A first
        cmd += Adata + ' ./ ' + Bdata

        C = (numpy.array(A) / numpy.array(B)).tolist()
    else:
        cmd += Bdata + ' ./ ' + Adata

        C = (numpy.array(B) / numpy.array(A)).tolist()

    sendwrapper(s, cmd + '\n')

    print('[TEST] dot_divide_matrices -- %s' %(cmd) )

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0
                
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] dot_divide_matrices')

    return 1

def sigmoid(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        Adata = varname
    else:
        A, Adata = genmatrix( random.randint(2,10), random.randint(2,10))

    cmd += '%sig( ' + Adata + ')'

    C = numpy.array(A)

    C = 1 / (1 + numpy.exp(-C))
    C = C.tolist()

    sendwrapper(s, cmd + '\n')

    print('[TEST] sigmoid -- %s' %(cmd) )

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0
                
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] sigmoid')

    return 1

def log_two(s):
    global variables

    cmd = ''

    A, Adata = genmatrix( random.randint(2,10), random.randint(2,10), 2, 8)

    cmd += '%lg( ' + Adata + ')'

    C = numpy.array(A)

    C = numpy.log2(A)
    C = C.tolist()

    sendwrapper(s, cmd + '\n')

    print('[TEST] log_two -- %s' %(cmd) )

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0
            
            if val == -float('Inf'):
                val = float('Inf')

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] log_two')

    return 1

def log_ten(s):
    global variables

    cmd = ''

    A, Adata = genmatrix( random.randint(2,10), random.randint(2,10), 2, 8)

    cmd += '%log( ' + Adata + ')'

    C = numpy.array(A)

    C = numpy.log10(A)
    C = C.tolist()

    sendwrapper(s, cmd + '\n')

    print('[TEST] log_ten -- %s' %(cmd) )

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0
            
            if val == -float('Inf'):
                val = float('Inf')

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] log_ten')

    return 1

def sqrtm(s):
    global variables

    cmd = ''

    A, Adata = genmatrix( random.randint(2,10), random.randint(2,10), 2, 8)

    cmd += '%sqrt( ' + Adata + ')'

    C = numpy.array(A)

    C = numpy.sqrt(A)
    C = C.tolist()

    sendwrapper(s, cmd + '\n')

    print('[TEST] sqrtm -- %s' %(cmd) )

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0
            
            if val == -float('Inf'):
                val = float('Inf')

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] sqrtm')

    return 1

def get_columns(s):
    global variables

    cmd = ''
    varname = random.choice( [*variables.keys()])
    nm = variables[varname]
    cmd += '%cols( ' + varname + ")"

    sendwrapper(s,cmd + '\n')

    print('[TEST] get_columns -- %s' %(cmd) )

    expectation = '\n\t' + "{0:.3f}".format(len(nm[0])) + ' \n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] get_columns')

    return 1

def get_rows(s):
    global variables

    cmd = ''
    varname = random.choice( [*variables.keys()])
    A = variables[varname]
    cmd += '%rows( ' + varname + ")"

    sendwrapper(s,cmd + '\n')

    print('[TEST] get_rows -- %s' %(cmd) )

    expectation = '\n\t' + "{0:.3f}".format(len(A)) + ' \n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] get_rows')

    return 1

def identity(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '


    size_selection = random.randint(2,5)

    cmd += '%I(' + str(size_selection) + ')'

    C = []

    for i in range(size_selection):
        row = []
        for j in range(size_selection):
            if i == j:
                row.append(float(1))
            else:
                row.append(float(0))
        
        C.append(row)

    
    sendwrapper(s, cmd + '\n')

    print('[TEST] identity -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] identity -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] identity')

    return 1

def magnitude(s):
    global variables

    newvarname = ''
    cmd = ''

    A, Adata = genmatrix( random.randint(2,10), 1)

    mag = 0

    for x in A:
        mag += x[0] ** 2

    mag = numpy.sqrt(mag)

    C = [[mag]]

    ## decide to use function or bars
    if random.randint(0,100) > 50:
        cmd += '||' + Adata + '||'
    else:
        cmd += '%mag(' + Adata + ')'
    
    sendwrapper(s, cmd + '\n')

    print('[TEST] magnitude -- %s' %(cmd) )

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0
                
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] magnitude')

    return 1

def zeros(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '


    rows = random.randint(2,5)
    cols = random.randint(2,5)

    cmd += '%zeros(' + str(rows) + ',' + str(cols) + ')'

    C = []

    for i in range(rows):
        row = []
        for j in range(cols):
            row.append(float(0))
        
        C.append(row)

    
    sendwrapper(s, cmd + '\n')

    print('[TEST] zeros -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] zeros -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] zeros')

    return 1

def ones(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '


    rows = random.randint(2,5)
    cols = random.randint(2,5)

    cmd += '%ones(' + str(rows) + ',' + str(cols) + ')'

    C = []

    for i in range(rows):
        row = []
        for j in range(cols):
            row.append(float(1))
        
        C.append(row)

    
    sendwrapper(s, cmd + '\n')

    print('[TEST] ones -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] ones -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] ones')

    return 1

def sum_matrix(s):
    global variables

    newvarname = ''
    cmd = ''

    ## decide to store or not
    store_matrix = (random.randint(0, 100) > 50)

    if store_matrix:
        newvarname = rs(7)
        cmd = newvarname + ' = '

    ## decide to use an existing variable or not
    use_existing = (random.randint(0, 100) > 50)

    varname = ''

    if use_existing:
        varname = random.choice( [*variables.keys()])
        A = variables[varname]
        data = varname
    else:
        A, data = genmatrix( random.randint(2,10), random.randint(2,10))

    ## decide to sum rows or sum cols
    if random.randint(0,100) > 50:
        ## cols
        C = numpy.array(A).sum(0).tolist()

        row = []

        for x in C:
            row.append(x)

        C = [row]



        dimension = 2.0
    else:
        ##rows
        C = numpy.array(A).sum(1).tolist()
        dimension = 1.0

        B = []

        for x in C:
            B.append([x])

        C = B[:]

    cmd += '%sum( ' + data + ',' + str(dimension) + ')'

    sendwrapper(s,cmd + '\n')

    print('[TEST] sum_matrix -- %s' %(cmd) )

    if store_matrix:
        variables[newvarname] = C

        expectation = newvarname + ' = \n'
        x = ''

        try:
            x = readuntil( s, ' = \n')
        except:
            print('[ERROR] sum_matrix -- failed to read "="')
            return 0

        if ( x != expectation):
            print('[ERROR] expected %s ; received %s' %(expectation, x) )
            return 0

    expectation = ''

    for row in C:
        expectation += '\n\t'

        for val in row:
            if val == 0.0:
                val = 0.0

            expectation += "{0:.3f}".format(val) + ' '

    expectation += '\n\n'

    z = readuntil(s, '\n\n')

    if ( z != expectation):
        print('[ERROR] expected %s ; received %s' %(expectation, z) )
        return 0

    readprompt(s)

    print('[SUCCESS] sum_matrix')

    return 1


## TODO: specify timeout
def run_test(HOST, PORT):
    s = connect( HOST, PORT )

    ### eat the initial prompt
    readprompt(s)

    ## begin by creating a few named matrices
    for _ in range(2):
        create_matrix(s)

    for _ in range(200):
        test_func = random.choice( [scalar_matrix_exp, transpose_matrix, check_named_matrix, 
                                    create_named_scalar_matrix, create_matrix, dot_hat_matrix,
                                    scalar_multiply_matrix, multiply_matrices, scalar_add_matrix,
                                    add_matrices, reshape, scalar_sub_matrix, sub_matrices,
                                    dot_multiply_matrices, add_func, dot_divide_matrices, sigmoid,
                                    log_two, log_ten, get_columns, identity, magnitude,
                                    zeros, ones, sub_func, sum_matrix] )

        r = test_func(s)

        if r == 0:
            print('[FAILED] Test did not succeed')
            s.close()
            sys.exit()

    sendwrapper(s,'exit\n')

    s.close()

def main():

    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print('[ERROR] target and port must be specified via arguments or variables.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] Poller for the Pierrepont Challenge Binary')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( a=SEED )

    run_test(HOST, PORT)
    print('[SUCCESS] Poller completed successfully')

if __name__ == '__main__':
    main()