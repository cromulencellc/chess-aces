import socket
import time
import sys
import base64
import struct
import random
import os
import logging
import shutil

methods = ['POST']

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

post_actions = [
        'create_list',
        'edit_list',
        'delete_list',
        'create_entry',
        'edit_entry',
        'delete_entry',
        ]

poss_list_names = [
        'list1',
        'aaaaaaaaaaaa',
        'aaaaaasssssssssfffffffffsad',
        'asdfkljqlkejlkqjwelkjrlkqwrlkjasdiofjqoiwejlkfjqwelkfjsadfiiojqwlekfj',
        'abaaaaaaaaaa',
        'abbaaaaaaaaa',
        'abbbaaaaaaaa',
        'abbbbaaaaaaa',
        'abbbbbaaaaaa',
        'abbbbbbaaaaa',
        'abbbbbbbaaaa',
        'abbbbbbbbaaa',
        'bbbbbbbbbbba',
        'asdfasdfasdfasd',
        'asdfuiwerwekjdjdl',
        'asdiasdmamsdhc',
        'adsficdiwkwerkqwe',
        'asdsacdsocwlqw',
        'asdckduwqwer',
        'asdcasdcaskbhjb',
        'asdcaksdiwqerqwef',
        'adfasclkasdoiuqwerf',
        'asdfasdlkfqlwekjtlkqtw',
        'qweflkasdjflkqwerfwq',
        'qwefjhasdflkjhqwlekjflqwef',
        'lsist1',
        'aafaaaaaaaaaa',
        'aaadaaasssssssssfffffffffsad',
        'asdfskljqlkejlkqjwelkjrlkqlkfjqwelkfjsadfiiojqwlekfj',
        'abaaadaaaaaaa',
        'abbaaaaaaaaaa',
        'abbbaaaasdfcaaaaa',
        'abbbbaaaaaaaxsa',
        'abbbbbaaaawqeraa',
        'abbbbbbaaqweraaa',
        'abbbbbbbasaaaa',
        'abbbbbbaa',
        'bbbbbbbba',
        'asdfasdsdfasd',
        'asdfuiwekjdjdl',
        'asdiasdsdhc',
        'adsficdkqwe',
        'asdsacdlqw',
        'asdckduwer',
        'asdcasdhjb',
        'asdcaksqerqwef',
        'adfascliuqwerf',
        'asdfasdekjtlkqtw',
        'qweflkalkqwerfwq',
        'qwefjhaekjflqwef',
        ]

entries = [
        'aaabbbcccdddeefffeeeffggghh',
        'aa',
        'aaaaaaaaaaaaaabbbbbbbbbbbbffeeeeeeeeffffffff',
        'aaaaabbbbbccccc',
        'aaaaaaaaaaasssssssssddddddfeeeeeeeeaaaaaaf',
        'abaaaaaaaaaa',
        'abbaaaaaaa',
        'abbbaa',
        'abbbbaaa',
        'abbbbaaaaa',
        'abbbbaaaaaa',
        'abbbbabbbaaaa',
        'abbbbbasdfbbbaaa',
        'bbbbbbbbbasdfbba',
        'bbbbbbbbbbasdfba',
        'asdfasdfasdfasd',
        'asdfuiwerwekjdjdl',
        'asdiasdmamsdhc',
        'adsficdiwkwerkqwe',
        'asdsacdsocwlqw',
        'asdckduwqwer',
        'asdcasdcaskbhjb',
        'asdcaksdiwqerqwef',
        'adfasclkasdoiuqwerf',
        'asdfasdlkfqlwekjtlkqtw',
        'qweflkasdjflkqwerfwq',
        'qwefjhasdflkjhqwlekjflqwef',
        'aaabbbcccdddeefffeeeffggghh',
        'aa',
        'abbbbbbbbbbbbbfffffeeeeeeeeffffffff',
        'abbbbbccccc',
        'assssssssdddddffffffffeeeeeeeeaaaaaaf',
        'aaaaaaaa',
        'aaaaaaa',
        'aaaaa',
        'aaaaaaaa',
        'abaaaaaa',
        'aaaa',
        'abbbaaaa',
        'abbbbbaaa',
        'bbbbbbbbba',
        'bbbbbbbbba',
        'asdfasdfasd',
        'asdfekjdjdl',
        'asdiamsdhc',
        'adsfwkwerkqwe',
        'asdsocwlqw',
        'asdcqwer',
        'asdcaskbhjb',
        'asdcerqwef',
        'adfasdoiuqwerf',
        'asdflwekjtlkqtw',
        'qwefqwerfwq',
        'qwefwlekjflqwef',
        ]

sids = []

sessions = []

curr_ent = []


session_count = 0

def print_dir_state(sids):
    i = 0
    for i in range(0, len(sids)):
        print('SID: ' + str(i) + ' ' + str(sids[i]))
    print('\n')

def single_query(s1):
    global session_count
    method = 'POST '
    path = '/lists?action='
    version =' HTTP/1.1\r\n'
    
    print('\n===========>SINGLE QUERY<===========')
    action = random.choice(post_actions)

    if action == 'create_list':
        print('CREATE_LIST\n')

        #Pick name that hasn't been created
        if len(poss_list_names) != 0:
            listname = random.choice(poss_list_names)
        else:
            print('No name to chose from possible list names')
            print('====================================')
            return
        
        if len(poss_list_names) != 0:
            #Now that it is created, remove it from possible pool
            poss_list_names.remove(listname)
        else:
            print('Nothing to remove from poss_list_names')
            print('====================================')
            return
        
        print('SID: sid_' + str(session_count))
        sids.insert(session_count, [])
        sids[session_count].insert(0, [list(), list()])

        print('LISTNAME: ' + str(listname))
        sids[session_count][len(sids[session_count]) - 1][0].insert(0, listname)

        sessions.append(session_count)

        session_count += 1

        query = 'create_list'
        header = 'Standard: Header' + '\r\n\r\n'
        req = method + path + query + '&list_name=' + str(listname) + version + header
        s1.send(bytes(req, 'utf-8'))

        print(req)

        print_dir_state(sids) 

    elif action == 'edit_list':
        print('EDIT_LIST\n')

        if len(sessions) < 1:
            print('There are not sessions created')
            print('====================================')
            return

        if len(poss_list_names) < 1:
            print('There are no in possible')
            print('====================================')
            return

        #Generate a 0 or 1 to see if this will create a query
        #that will pick from an already created list, making the query work
        #or one that is in the possible pool, making the request invalid
        #branch_number = random.randint(0,1)
        branch_number = 0

        if branch_number == 0:
            print('THIS WILL BE A VALID QUERY\n')

            #Generate number between 0 and session_count for sid
            if sessions: 
                rand = int(random.choice(sessions))
                listrand = random.randint(0, len(sids[rand]) - 1)
                sid = 'sid_' + str(rand)
            else:
                print('no valid session')
                return

            if len(sids[rand][listrand]) != 0:
                listname = ''.join(sids[rand][listrand][0])
                if not listname:
                    print('No list in sid')
                    return
            else:
                print('No list in sid: ' + str(rand) + ' to edit')
                return
            
            #Pick name from possible pool
            change_name = random.choice(poss_list_names)

            if not change_name:
                print('No possible name')
                return

        if branch_number == 0:
            sids[rand][0][0] = change_name
            poss_list_names.append(listname)

            #Remove change from possible add to created
            poss_list_names.remove(change_name)

        print('SID: ' + sid)
        print('LISTNAME: ' + str(listname))
        print('CHANGE: ' + str(change_name))

        query = 'edit_list'
        header = 'Standard: Header' + '\r\n\r\n'
        req = method + path + query + '&sid=' + sid +'&list_name=' + str(listname) + '&change=' + str(change_name) + version + header
        s1.send(bytes(req, 'utf-8'))

        print(req)
       
        print_dir_state(sids) 

    elif action =='delete_list':
        print('DELETE_LIST\n')

        if len(sessions) < 1:
            print('There are not lists created')
            print('====================================')
            return

        if len(poss_list_names) < 1:
            print('There are no in possible')
            print('====================================')
            return

        #Generate a 0 or 1 to see if this will create a query
        #that will pick from an already created list, making the query work
        #or one that is in the possible pool, making the request invalid
        #branch_number = random.randint(0,1)
        branch_number = 0

        if branch_number == 0:
            print('THIS WILL BE A VALID QUERY\n')
            
            #Generate number between 0 and session_count for sid
            rand = int(random.choice(sessions))
            listrand = random.randint(0, len(sids[rand]) - 1)
            sid = 'sid_' + str(rand)

            if len(sids[rand][listrand]) != 0:
                listname = ''.join(sids[rand][listrand][0])
                if not listname:
                    print('No list in sid')
                    return
            else:
                print('No list in sid: ' + str(rand) + ' to edit')
                return

            #Remove it from created pool
            del sids[rand][listrand][0]
            sessions.remove(rand)
        
            #Insert to possible pool
            poss_list_names.append(listname)

        print('SID: ' + sid)
        print('DELETE: ' + str(listname))

        query = 'delete_list'
        header = 'Standard: Header' + '\r\n\r\n'
        req = method + path + query + '&sid=' + sid + '&list_name=' + str(listname) + version + header
        s1.send(bytes(req, 'utf-8'))

        print(req)
        
        print_dir_state(sids)

    elif action =='create_entry':
        print('CREATE_ENTRY\n')

        if len(sessions) < 1:
            print('There are not lists created')
            print('====================================')
            return

        if len(poss_list_names) < 1:
            print('There are no in possible')
            print('====================================')
            return

        #Generate a 0 or 1 to see if this will create a query
        #that will pick from an already created list, making the query work
        #or one that is in the possible pool, making the request invalid
        #branch_number = random.randint(0,1)
        branch_number = 0

        if branch_number == 0:
            print('THIS WILL BE A VALID QUERY\n')

            #Generate number between 0 and session_count for sid
            rand = int(random.choice(sessions))
            listrand = random.randint(0, len(sids[rand]) - 1)
            sid = 'sid_' + str(rand)

            if len(sids[rand][listrand]) > 0:

                listname = ''.join(sids[rand][listrand][0])

                if not listname:
                    print('No list in SID')
                    return
            else:
                print('No list in SID to add entry to')
                return

            #Pick name from possible entries pool
            if len(entries) > 0:
                entry = random.choice(entries)
            else:
                print('No possible entry names to choose from')
                return
        
        #No else clause because if it is not a valid query
        #It will not effect the service state
        if branch_number == 0:
            entries.remove(entry)

        print('SID: ' + sid)
        print('LISTNAME: ' + str(listname))
        print('ENTRY: ' + str(entry))

        query = 'create_entry'
        header = 'Standard: Header' + '\r\n\r\n'
        req = method + path + query + '&sid=' + sid +'&list_name=' + str(listname) + '&entry=' + entry + version + header
        s1.send(bytes(req, 'utf-8'))

        print(req)
        print_dir_state(sids)

    elif action =='edit_entry':
        print('EDIT_ENTRY\n')
       
        if len(sessions) < 1:
            print('There are not lists created')
            print('====================================')
            return

        if len(poss_list_names) < 1:
            print('There are no in possible')
            print('====================================')
            return

        #Generate a 0 or 1 to see if this will create a query
        #that will pick from an already created list, making the query work
        #or one that is in the possible pool, making the request invalid
        #branch_number = random.randint(0,1)
        branch_number = 0

        if branch_number == 0:
            print('THIS WILL BE A VALID QUERY\n')

            #Generate number between 0 and session_count for sid
            rand = random.choice(sessions)
            listrand = random.randint(0, len(sids[rand]) - 1)

            sid = 'sid_' + str(rand)

            
            #Pick name that has been created so it can be edited
            if len(sids[rand][listrand]) > 0:

                listname = ''.join(random.choice(sids[rand][listrand]))

                if not listname:
                    print('No list in SID')
                    return
            else:
                print('Nothing in this sid to remove')
                return

            #Pick name from possible pool
            if len(sids[rand][listrand][1]) > 0:
                erand = random.randint(0, len(sids[rand][listrand][1]) - 1)
                entry = ''.join(sids[rand][listrand][1][erand])

            else:
                print('No entry to pick')
                return

            if entries:
                change = random.choice(entries)
            else:
                print('Nothing in possible pool')
                return

        #No else clause because if it is not a valid query
        #It will not effect the service state
        if branch_number == 0:
            #Remove list_name from created pool and add it to the possible pool
            sids[rand][listrand][1][erand] = change

        print('SID: ' + sid)
        print('LISTNAME: ' + str(listname))
        print('ENTRY: ' + str(entry))
        print('CHANGE: ' + change)

        query = 'edit_entry'
        header = 'Standard: Header' + '\r\n\r\n'
        req = method + path + query + '&sid=' + sid +'&list_name=' + str(listname) + '&entry=' + str(entry) +'&change=' + change + version + header
        s1.send(bytes(req, 'utf-8'))

        print(req)

        print_dir_state(sids)

    elif action =='delete_entry':
        print('DELETE_ENTRY\n')

        if len(sessions) < 1:
            print('There are not lists created')
            print('====================================')
            return

        if len(poss_list_names) < 1:
            print('There are no in possible')
            print('====================================')
            return

        branch_number = 0

        if branch_number == 0:
            print('THIS WILL BE A VALID QUERY\n')

            #Generate number between 0 and session_count for sid
            rand = random.choice(sessions)
            listrand = random.randint(0, len(sids[rand]) - 1)
            sid = 'sid_' + str(rand)
            
            if len(sids[rand][listrand]) > 0:

                listname = ''.join(sids[rand][listrand][0])

                print(str(listname))

                if not listname:
                    print('No list in SID')
                    return
            else:
                print('No list in SID to add entry to')
                return

            if len(sids[rand][listrand][1]) > 0:
                    erand = random.randint(0, len(sids[rand][listrand][1]) - 1)
                    entry = ''.join(sids[rand][listrand][1][erand])
            else:
                print('No Entry there')
                return

        #No else clause because if it is not a valid query
        #It will not effect the service state
        if branch_number == 0:
            sids[rand][listrand][1].pop(erand)

        print('SID: ' + sid)
        print('LISTNAME: ' + listname)
        print('ENTRY: ' + entry)

        query = 'delete_entry'
        header = 'Standard: Header' + '\r\n\r\n'
        req = method + path + query + '&sid=' + sid +'&list_name=' + listname + '&entry=' + entry + version + header
        s1.send(bytes(req, 'utf-8'))

        print(req)

        print_dir_state(sids)

    print('====================================\n')


def multi_query(s1):
    global session_count
    method = 'POST '
    path = '/lists?action=session'
    version =' HTTP/1.1\r\n'

    list_count = 0

    req_line = method + path + version
    body = ''
    rand_iter = random.randint(1, 10)

    for i in range(0,rand_iter):
        action = str(random.choice(post_actions))

        if action == 'create_list':
            print('\nCREATE_LIST')

            #Pick name that hasn't been created
            if len(poss_list_names) != 0:
                listname = random.choice(poss_list_names)
                print('LISTNAME: ' + listname)
            else:
                print('No name to chose from possible list names')
                print('====================================')
                continue
        
            if len(poss_list_names) != 0:
                #Now that it is created, remove it from possible pool
                poss_list_names.remove(listname)
            else:
                print('Nothing to remove from poss_list_names')
                print('====================================')
                continue
        
            if list_count == 0:
                sids.insert(session_count, [])
                print(str(session_count) + str(list_count))
                sids[session_count].insert(list_count, [list(), list()])
            
            sids[session_count].append([list(), list()])
            sids[session_count][list_count][0].append(listname)
            list_count += 1
            
            print_dir_state(sids)

            body += action + '\n'
            body += 'list_name: ' + str(listname) + '\n\n'

        elif action == 'edit_list':
            print('EDIT_LIST')

            if sids == []:
                print('NO list to edit')
                continue
            
            try:
                if sids[session_count]:
                    print('Session exists to edit')
            except IndexError:
                continue

            if list_count == 0:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count)

            else:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count - 1)

            if sids[session_count][listrand][0]:
                listname = ''.join(random.choice(sids[session_count][listrand][0]))
                print('EDIT Listname: ' + str(listname))
            else:
                print('Empty')
                continue

            if poss_list_names:
                change = ''.join(random.choice(poss_list_names))
                print('EDIT Change: ' + str(change))
            else:
                print('Poss list names empty')
                continue

            check = sids[session_count][listrand][0].index(listname)

            sids[session_count][listrand][0][check] = change

            poss_list_names.append(listname)
            poss_list_names.remove(change)

            print_dir_state(sids)

            body += action + '\n'
            body += 'list_name: ' + str(listname) + '\n'
            body += 'change: ' + str(change) + '\n\n'

        elif action == 'delete_list':
            print('DELETE_LIST')

            if sids == []:
                print('NO list to delete')
                continue
            
            try:
                if sids[session_count]:
                    print('Session exists to edit')
            except IndexError:
                continue

            if list_count == 0:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count)

            else:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count - 1)

            if len(sids[session_count][listrand][0]) > 0:
                listname = ''.join(random.choice(sids[session_count][listrand][0]))
                print('DELETE Listname: ' + str(listname))
            
            else:
                print('Nothing to delete')
                continue

            sids[session_count][listrand][0].remove(listname)
            
            if list_count > 0:
                list_count -= 1

            poss_list_names.append(listname)

            print_dir_state(sids)

            body += action + '\n'
            body += 'list_name: ' + str(listname) + '\n\n'

        elif action == 'create_entry':
            print('CREATE_ENTRY')

            if sids == []:
                print('NO list to add entry')
                continue
            
            try:
                if sids[session_count]:
                    print('Session exists to add entry')
            except IndexError:
                continue

            if list_count == 0:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count)

            else:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count - 1)

            if sids[session_count][listrand][0]:
                listname = ''.join(random.choice(sids[session_count][listrand][0]))
                print('Listname: ' + str(listname))
            else:
                print('Empty')
                continue

            #Pick name from possible entries pool
            if len(entries) > 0:
                entry = random.choice(entries)
                print('Entry: ' + str(entry))
            else:
                print('No possible entry names to choose from')
                return

            check = sids[session_count][listrand][0].index(listname)
            sids[session_count][check][1].append(entry)

            print_dir_state(sids)

            body += action + '\n'
            body += 'list_name: ' + str(listname) + '\n'
            body += 'entry: ' + str(entry) + '\n\n'

        elif action == 'edit_entry':
            print('EDIT_ENTRY')

            if sids == []:
                print('NO list to delete entry')
                continue
            
            try:
                if sids[session_count]:
                    print('Session exists to edit entry')
            except IndexError:
                continue

            if list_count == 0:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count)

            else:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count - 1)

            if sids[session_count][listrand][0]:
                listname = ''.join(random.choice(sids[session_count][listrand][0]))
                print('Listname: ' + str(listname))
            else:
                print('Empty')
                continue

            #Pick name from possible entries pool
            if sids[session_count][listrand][1]:
                entry = ''.join(random.choice(sids[session_count][listrand][1]))
                print('Entry: ' + str(entry))
            else:
                print('No possible entry names to choose from')
                return

            check = sids[session_count][listrand][0].index(listname)
            check1 = sids[session_count][check][1].index(entry)

            if entries:
                change = ''.join(random.choice(entries))
                print('EDIT Change: ' + str(change))
            else:
                print('Entires is empty')
                continue

            print(str(session_count) + str(check) + str(check1))
            sids[session_count][check][1][check1] = change

            print_dir_state(sids)

            body += action + '\n'
            body += 'list_name: ' + str(listname) + '\n'
            body += 'entry: ' + str(entry) + '\n'
            body += 'change: ' + str(change) + '\n\n'

        elif action == 'delete_entry':
            print('DELETE_ENTRY')

            if sids == []:
                print('NO list to delete entry')
                continue
            
            try:
                if sids[session_count]:
                    print('Session exists to delete entry')
            except IndexError:
                continue

            if list_count == 0:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count)

            else:
                print('List: ' + str(list_count))
                listrand = random.randint(0, list_count - 1)

            if sids[session_count][listrand][0]:
                listname = ''.join(random.choice(sids[session_count][listrand][0]))
                print('Listname: ' + str(listname))
            else:
                print('Empty')
                continue

            #Pick name from possible entries pool
            if sids[session_count][listrand][1]:
                entry = ''.join(random.choice(sids[session_count][listrand][1]))
                print('Entry: ' + str(entry))
            else:
                print('No possible entry names to choose from')
                return

            check = sids[session_count][listrand][0].index(listname)
            sids[session_count][check][1].remove(entry)

            print_dir_state(sids)

            body += action + '\n'
            body += 'list_name: ' + str(listname) + '\n'
            body += 'entry: ' + str(entry) + '\n\n'

        else:
            print('Error with selecting action')

    header = 'Content-Length: ' + str(len(body)) + '\r\n\r\n'
    req = req_line + header + body

    print('\n\n'  + str(req))

    s1.send(bytes(req, 'utf-8'))

    print('\nEND\n')
    
    if body.find('create_list') == -1:
        return
    else:
        session_count += 1


def test_run(HOST, PORT, LENGTH):

    SERVICES = [single_query, multi_query]

    global s1

    for i in range(0, LENGTH):
        s1 = socket.socket(socket.AF_INET , socket.SOCK_STREAM)
        s1.connect((HOST, PORT))
        acti = random.choice(SERVICES)
        got = acti(s1)

        s1.close()

def main():

    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])

    elif 'HOST' in os.environ and 'PORT' in os.environ:
        HOST = os.environ['HOST']
        PORT = int(os.environ['PORT'])

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

    test_run(HOST, PORT, LENGTH)

    print('Success!')

if __name__ == '__main__':
    main()

