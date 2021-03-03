'''
To run do:
python test.py UNAME_OF_ADMIN PASS_OF_ADMIN #_OF_LANDPAGE_CHOICE #_OF_FUNCTION FP1 FP2... FPN

Example where an ADMIN username is BOB and his password is hunter2, and creating a user SALLY

python test.py BOB hunter2 2 2 SALLY Sue USER

This is the poller for kane:

1)It will run GET, POST, HEAD(not supported method) requests to the server
2)It will run the various functions of the admin console
    a)File

        i)Edit
        ii)Create
        iii)Delete

    b)User List

        i)Edit
        ii)Create
        iii)Delete

'''

import socket
import time
import sys
import base64
import struct
import random
import os
import logging
import shutil

methods = ['GET', 'POST' , 'HEAD']

uri = [
      'Fellowship.html',  
      'get_example.html',  
      'Return_of_the_King.html',  
      'Two_Towers.html',  
      'yolo.html',
      ]

existing_files = [
        'Fellowship.txt',
        'get_example.txt',
        'Return_of_the_King.txt',
        'Two_Towers.txt',
        'yolo.txt',
        ]

new_choices = [
        'aaaa.txt', 
        'bbbb.txt', 
        'cccc.txt', 
        'dddd.txt', 
        'eeee.txt', 
        'ffff.txt', 
        'gggg.txt', 
        'iiii.txt', 
        'jjjj.txt',
        'kkkk.txt',
        'llll.txt',
        'mmmm.txt',
        'nnnn.txt',
        'oooo.txt',
        'pppp.txt',
        'qqqq.txt',
        'rrrr.txt',
        'ssss.txt',
        'tttt.txt',
        'uuuu.txt',
        'vvvv.txt',
        'wwww.txt',
        'xxxx.txt',
        'yyyy.txt',
        'zzzz.txt',
        ]

headers = [
        'Accept:',
        'Accept-Charset:',
        'Accept-Language:',
        'Cache-Control:',
        'Connection:',
        'Content-Length:',
        'Cookie:',
        'Date:',
        'Origin:',
        'Pragma:',
        'Range:',
        'Transfer-Encoding:',
        'User-Agent:',
        'Expect:',
        'From:',
        'Host:',
        'If-Match:',
        'If-Modified-Since:',
        'If-Range:',
        'Max-Forwards:',
        'Warning:',
        'Via:',
        'Trailer:',
        'A-IM:',
        ]


utypes = ['ADMIN', 'USER', 'ANONYMOUS']

existing_uname_passes = [['PETER','hunter'] , ['SALLY', '31337']]

new_uname_passes = [

        ['TEST' , 'TEST'], 
        ['TEST1' , 'TEST1'], 
        ['TEST2' , 'TEST2'],
        ['AAAA','BBBB'],
        ['CCCC','DDDD'],
        ['EEEE','FFFF'],
        ['GGGG','HHHH'],
        ['IIII','JJJJ'],
        ['KKKK','LLLL'],
        ['MMMM','NNNN'],
        ['OOOO','PPPP'],
        ['QQQQ','RRRR'],
        ['SSSS','TTTT'],
        ['UUUU','VVVV'],
        ['XXXX','YYYY'],
        ['ZZZZ','ZZZZ'],
        ['AABB','AABB'],
        ['CCDD','DDEE'],
        ['FFGG','HHII'],
        ['JJKK','LLMM'],

        ]


base_file_contents = [
        'The future of civilization rests in the fate of the One Ring which has been lost for centuries Powerful forces are unrelenting in their search for it But fate has placed it in the hands of a young Hobbit named Frodo Baggins Elijah Wood who inherits the Ring and steps into legend A daunting task lies ahead for Frodo when he becomes the Ringbearer to destroy the One Ring in the fires of Mount Doom where it was forged',

        'This is a test example',

        'The culmination of nearly 10 years work and conclusion to Peter Jackson\'s epic trilogy based on the timeless JRR Tolkien classic The Lord of the Rings The Return of the King presents the final confrontation between the forces of good and evil fighting for control of the future of Middle-earth Hobbits Frodo and Sam reach Mordor in their quest to destroy the one ring while Aragorn leads the forces of good against Sauron\'s evil army at the stone city of Minas Tirith',
        'The Two Towers opens with the disintegration of the Fellowship as Merry and Pippin are taken captive by Orcs after the death of Boromir in battle The Orcs having heard a prophecy that a Hobbit will bear a Ring that gives universal power to its owner wrongly think that Merry and Pippin are the Ring-bearers',

        'This is a great way to test a search feature to make sure that it is working correctly. Lets be real it is probably not but here we go',

        ]

def create_user(s1, uname, password, utype):
    #select create
    s1.send(bytes('2', 'utf-8'))

    #prepare to send size as integer
    uname_buffer = struct.pack('i', len(uname))

    #send size
    s1.send(uname_buffer)

    #send username
    s1.send(bytes(uname, 'utf-8'))

    #prep size of pass
    pass_buffer = struct.pack('i', len(password))

    #send pasword size
    s1.send(pass_buffer)

    #send password
    s1.send(bytes(password, 'utf-8'))

    utype_buffer = struct.pack('i', len(utype))

    #send type size
    s1.send(utype_buffer)

    #send utype
    s1.send(bytes(utype, 'utf-8'))

    existing_uname_passes.append([uname , password])

    print('[create_user] TEST CASE: 22' + str(len(uname)) + uname + str(len(password)) + password + str(len(utype)) + utype + '\n\n')

def edit_user(s1, edit_choice, curr_uname , new_field):

    curr_size = struct.pack('i' , len(curr_uname))

    s1.send(curr_size)

    #send current username
    s1.send(bytes(curr_uname, 'utf-8'))

    new_size = struct.pack('i' , len(new_field))

    s1.send(new_size)

    s1.send(bytes(new_field, 'utf-8'))

    #send edit_choice 1 for username 2 for password 3 for type
    s1.send(bytes(str(edit_choice), 'utf-8'))

    print('[edit_user] TEST CASE: 21' + str(len(curr_uname)) + curr_uname + str(len(new_field)) + new_field  + str(edit_choice)+ '\n\n')

    #need to edit the old field and update with the new field if editing uname or password
    if edit_choice == 1:
        index = [uname[0] for uname in existing_uname_passes].index(curr_uname)
        existing_uname_passes[index][0] = new_field
    elif edit_choice == 2:
        index = [uname[0] for uname in existing_uname_passes].index(curr_uname)
        existing_uname_passes[index][1] = new_field


def delete_user(s1, del_uname):
    #send delete choice
    s1.send(bytes('3','utf-8'))

    del_uname_size = struct.pack('i' , len(del_uname))

    s1.send(del_uname_size)

    s1.send(bytes(del_uname, 'utf-8'))

    #need to remove from the list
    index = [uname[0] for uname in existing_uname_passes].index(del_uname)
    existing_uname_passes.pop(index)

    print('[delete_user] TEST CASE 23' + str(len(del_uname)) + del_uname + '\n\n')

def edit_document(s1, filename):
    #send edit choice
    s1.send(bytes('1', 'utf-8'))

    #pack size
    cont_size = struct.pack('i' , len(filename))

    s1.send(cont_size)

    s1.send(bytes(filename, 'utf-8'))

    print('[edit_doc] TEST-CASE: 11' + str(len(filename)) + filename + '\n\n')


def create_document(s1, filename):
    #send create choice
    s1.send(bytes('2', 'utf-8'))

    #pack size
    cont_size = struct.pack('i' , len(filename))

    s1.send(cont_size)

    s1.send(bytes(filename, 'utf-8'))

    existing_files.append(filename)
    new_choices.remove(filename)

    print( '[create_doc] TEST-CASE: 12' + str(len(cont_size)) + filename + '\n\n')

def delete_document(s1, filename):
    #send create choice
    s1.send(bytes('3', 'utf-8'))

    #pack size
    cont_size = struct.pack('i' , len(filename))

    s1.send(cont_size)

    s1.send(bytes(filename, 'utf-8'))

    new_choices.append(filename)
    existing_files.remove(filename)

    print('[delete_doc] TEST-CASE: 13' + str(len(filename)) + filename + '\n\n')

def catch_landpage(s1, admin_choice):

    #Receive land page
    s = s1.recv(500)

    #Send choice
    s1.send(bytes(admin_choice, 'utf-8'))

    #Receive User prompt
    s = s1.recv(500)

def encode_query(query_string):
    query_string = query_string.replace(' ' , '%20')

    return query_string

def generate_stat_line(method, URI, HTTP_Ver):
    if method == 'GET':
        choice = random.randint(0,1)
        if choice == 1:
            URI = '/wiki?'
            base_contents_choice = random.choice(base_file_contents).split()

            for i in range(0, random.randint(0, len(base_contents_choice))):
                if i == 0:
                    URI += base_contents_choice[i]
                else:
                    URI += '%20'+ base_contents_choice[i]

    stat_line = method + ' ' + URI + ' ' + HTTP_Ver + '\r\n'

    return stat_line

def send_auth(admin_username, admin_password):

    over_wire = admin_username + ':' + admin_password

    auth_string = 'GET / HTTP/1.1\r\n' + 'Authorization: Basic ' + str((base64.b64encode(bytes(over_wire, 'utf-8'))).decode()) + '\r\n\r\n'

    #send Auth with GET req
    s1.send(bytes(auth_string, 'utf-8'))


def start_http():
    stat_line = generate_stat_line(random.choice(methods) , '/wiki/cache/' + random.choice(uri) ,'HTTP/1.1')

    global headers

    req_headers = ''

    for header in range(0,random.randint(0, len(headers))):
        multiplier = random.randint(0,200)
        req_headers += str(random.choice(headers)) + ' '  + 'A' * multiplier + '\r\n'

    req_headers += '\r\n'

    if 'POST' in stat_line:
        req_headers += 'AB' * random.randint(0, 2500)

    request = stat_line + req_headers

    print(stat_line)

    s1.send(bytes(request, 'utf-8'))

def start_admin():
    username = 'NOPE'
    password = 'NOPE'

    send_auth(username, password)

    admin_choice = str(random.randint(1,2))
    #admin_choice = '1'

    edit_choice = str(random.randint(1,4))
    #edit_choice = '1'

    #File Edit Options
    if admin_choice == '1':
        if edit_choice == '1': 
            catch_landpage(s1, admin_choice) 

            if len(existing_files) > 0:
                ex_file = random.choice(existing_files)

            else:
                print('There is no file to edit')
                return
            edit_document(s1, ex_file)

        elif edit_choice == '2': 
            catch_landpage(s1, admin_choice) 

            if len(new_choices) > 0:
                new_doc = random.choice(new_choices)
            else:
                print('Out of ideas')
                return

            create_document(s1, new_doc)

        elif edit_choice == '3': 
            catch_landpage(s1, admin_choice) 

            if len(existing_files) > 0:
                ex_file = random.choice(existing_files)

            else:
                print('Nothing to delete')
                return
            delete_document(s1,ex_file)
        
        elif edit_choice == '4':
            print('[EXIT]\n\n')

    #User List Options
    elif admin_choice == '2':

        #edit user
        if edit_choice == '1': 
            catch_landpage(s1, admin_choice)

            inside_choice = random.randint(1,3)

            if len(existing_uname_passes) > 0:
                curr_uname = random.choice(existing_uname_passes)[0]
            else:
                print('Nothing to edit')
                return

            if inside_choice == 1:
                new_field = random.choice(new_uname_passes)[0]

            elif inside_choice == 2:
                new_field = random.choice(new_uname_passes)[1]

            elif inside_choice == 3:
                new_field = random.choice(utypes)

            else:
                print('[ERROR] invalid choice')
                exit(-1)

            print('[edit_user] TEST-CASE: ' + edit_choice)
            s1.send(bytes(edit_choice, 'utf-8'))

            edit_user(s1, inside_choice , curr_uname, new_field)
        
        #create user
        elif edit_choice == '2':

            catch_landpage(s1, admin_choice)

            uname = random.choice(new_uname_passes)[0]

            password = random.choice(new_uname_passes)[1]

            utype = random.choice(utypes)
            
            create_user(s1, uname, password, utype)

        #delete user
        elif edit_choice == '3':

            catch_landpage(s1, admin_choice)

            if len(existing_uname_passes) > 0:
                uname = random.choice(existing_uname_passes)[0]

            else:
                print('Nothing to delete')
                return
            
            delete_user(s1, uname)

        #exit
        elif edit_choice == '4':
            print('[EXIT]\n\n')

s1 = None
def test_run(HOST, ADMIN_PORT, HTTP_PORT, LENGTH):
    logging.info('[INFO] Poller basic test started')

    SERVICES = [start_admin , start_http]

    global s1

    for i in range(0, LENGTH):
        s1 = socket.socket(socket.AF_INET , socket.SOCK_STREAM)
        acti = random.choice(SERVICES)
        
        if acti == start_admin:
            s1.connect((HOST , ADMIN_PORT))
        
        elif acti == start_http:
            s1.connect((HOST, HTTP_PORT))
        
        logging.info('running {0}'.format(acti.__name__))
        got = acti()
        s1.close()

def main():

    if len(sys.argv) == 4:
        HOST = sys.argv[1]
        ADMIN_PORT = int(sys.argv[2])
        HTTP_PORT = int(sys.argv[3])

    elif 'HOST' in os.environ and 'ADMIN_PORT' in os.environ and 'HTTP_PORT' in os.environ:
        HOST = os.environ['HOST']
        HTTP_PORT = int(os.environ['HTTP_PORT'])
        ADMIN_PORT = int(os.environ['ADMIN_PORT'])
    else:
        print('[ERROR] target and port must be specified via arguments or variables.')
        sys.exit(1)


    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])

    else:
        SEED = random.randint(0, 2**64 - 1)

    random.seed(SEED)

    if 'LENGTH' in os.environ:
        LENGTH = int(os.environ['LENGTH'])
    else:
        LENGTH = random.randint(25,75)

    print('SEED={0}'.format(SEED))
    print('LENGTH={0}'.format(LENGTH))

    origin_files = existing_files[:]

    test_run(HOST, ADMIN_PORT, HTTP_PORT, LENGTH)

    print('Success!')

if __name__ == '__main__':
    main()

