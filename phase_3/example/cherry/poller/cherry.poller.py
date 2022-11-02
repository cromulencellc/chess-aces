import socket
import sys
import telnetlib
import shutil
import os
import time
import datetime
import random
import string

class cvs:
	"""Class used to connect to an communicate with a CVS server

	It can authenticate, commit to a repos, read from a repo, etc.
	"""

	def __init__(self, server, user, password, root='/cvsroot', port=2401):
		"""
			server: IP address or hostname of the cvs server
			user: user to authenticate as
			password: Password of the specified user
			root: This is the root of the CVS repo. Defaults to /cvsroot
			port: Port of the server. Defaults to 2401
		"""

		self.shifts = [   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
						 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
						114, 120,  53,  79,  96, 109,  72, 108,  70,  64,  76,  67, 116,  74,  68,  87,
						111,  52,  75, 119,  49,  34,  82,  81,  95,  65, 112,  86, 118, 110, 122, 105,
						 41,  57,  83,  43,  46, 102,  40,  89,  38, 103,  45,  50,  42, 123,  91,  35,
						125,  55,  54,  66, 124, 126,  59,  47,  92,  71, 115,  78,  88, 107, 106,  56,
						 36, 121, 117, 104, 101, 100,  69,  73,  99,  63,  94,  93,  39,  37,  61,  48,
						 58, 113,  32,  90,  44,  98,  60,  51,  33,  97,  62,  77,  84,  80,  85, 223,
						225, 216, 187, 166, 229, 189, 222, 188, 141, 249, 148, 200, 184, 136, 248, 190,
						199, 170, 181, 204, 138, 232, 218, 183, 255, 234, 220, 247, 213, 203, 226, 193,
						174, 172, 228, 252, 217, 201, 131, 230, 197, 211, 145, 238, 161, 179, 160, 212,
						207, 221, 254, 173, 202, 146, 224, 151, 140, 196, 205, 130, 135, 133, 143, 246,
						192, 159, 244, 239, 185, 168, 215, 144, 139, 165, 180, 157, 147, 186, 214, 176,
						227, 231, 219, 169, 175, 156, 206, 198, 129, 164, 150, 210, 154, 177, 134, 127,
						182, 128, 158, 208, 162, 132, 167, 209, 149, 241, 153, 251, 237, 236, 171, 195,
						243, 233, 253, 240, 194, 250, 191, 155, 142, 137, 245, 235, 163, 242, 178, 152 ]

		if type(server) == type(''):
			server = server.encode('UTF-8')

		if type(user) == type(''):
			user = user.encode('UTF-8')

		if type(root) == type(''):
			root = root.encode('UTF-8')

		if type(password) == type(''):
			password = password.encode('UTF-8')

		self.server = server
		self.user = user
		self.root=root
		self.password = password
		self.password_s = self.scramble(password)
		self.port = port
		self.connected = 0
		self.valid_responses = []
		self.valid_requests = []

		self.modules_expected = []

		## Clear out any previous CVSROOT
		try:
			shutil.rmtree('./CVSROOT')
		except OSError as e:
			## Means that it doesn't exist so no big deal
			pass

		if self.connect( ):
			raise Exception("Failed to connect")

	def readline(self, keep=False):
		if self.connected == 0:
			print("[ERROR] Cannot read line, not connected")
			return ''

		z = b''

		while z.endswith(b'\n') == False:
			z += self.server_fd.recv(1)

		if keep == False:
			z = z.strip(b'\n')

		return z

	def send_root(self):
		"""
		Sends the root command to the cvs server
		"""

		if self.connected == 0:
			print("Cannot send Root command. Not connected")
			return 1

		self.server_fd.send(b'Root %s\n' %(self.root))

		## It doesn't expect a response

		return 0

	def send_valid_responses(self, resp):
		"""
		Lets the server know which responses the client understands

		Input: resp - List of valid responses that this client understands
		Ouput: None but this set of valid responses will be set in the class
		"""
		if self.connected == 0:
			print("Cannot send valid-responses command. Not connected")
			return 1

		cmd = b'Valid-responses '

		for x in resp:
			cmd += x + b' '

		cmd = cmd.rstrip(b' ') + b'\n'

		self.server_fd.send(cmd)

		self.valid_responses = resp[:]

		## Doesn't expect a response

		return 0

	def send_valid_requests(self):
		"""
		Lets the client know which requests the server understands

		Input: none
		Ouput: None but this set of valid requests will be set in the class
		"""
		if self.connected == 0:
			print("Cannot send valid-requests command. Not connected")
			return 1

		self.server_fd.send(b'valid-requests\n')

		y = self.readline().split(b' ')
		z = self.readline()

		if y[0] != b'Valid-requests':
			print(f'Unexpected response from Valid-requests command: {y[0]}'.encode('UTF-8'))
			return 1
		else:
			self.valid_requests = y[1:]

		if z != b'ok':
			print(f'Expected \'ok\'. Received {z}')
			return 1

		return 0

	def connect( self ):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		try:
			s.connect((self.server, self.port))
		except Exception as e:
			print(f'Caught exception when attempting to connect to {self.server}:{self.port} : {e}')
			return 1

		s.send(b'BEGIN AUTH REQUEST\n')
		s.send(self.root + b'\n')
		s.send(self.user + b'\n')
		## This should be the scrambled password
		s.send(self.password_s + b'\n')
		s.send(b'END AUTH REQUEST\n')

		self.connected = 1
		self.server_fd = s

		y = self.readline()

		if y != b'I LOVE YOU':
			print('[ERROR] Authentication failed')
			s.close()
			self.connected = 0
			return 1

		if self.send_root( ):
			return 1

		if self.send_valid_responses([b'ok', b'error', b'Valid-requests', b'Force-gzip', b'Referrer', b'Redirect', 
			                       b'Checked-in', b'New-entry', b'Checksum', b'Copy-file', b'Updated', b'Created', 
			                       b'Update-existing', b'Merged', b'Patched', b'Rcs-diff', b'Mode', b'Mod-time',
			                       b'Removed', b'Remove-entry', b'Set-static-directory', b'Clear-static-directory', 
			                       b'Set-sticky', b'Clear-sticky', b'Edit-file', b'Template', b'Clear-template', 
			                       b'Notified', b'Module-expansion', b'Wrapper-rcs', b'Option', b'M', b'Mbinary', 
			                       b'LOGM', b'E', b'F', b'MT']):
			return 1

		if self.send_valid_requests():
			return 1

		return 0

	def send_command_prep(self, cmd):
		if self.connected == 0:
			print("Cannot send Command-prep command. Not connected")
			return 1

		self.server_fd.send(b'Command-prep %s\n' %(cmd))

		## Server should send ok
		y = self.readline()

		if y != b'ok':
			print("Expected 'ok' received: %s" %(y))
			return 1

		return 0

	def send_argument(self, arg):
		if self.connected == 0:
			print("Cannot send Argument command. Not connected")
			return 1

		self.server_fd.send(b'Argument %s\n' %(arg))

		self.modules_expected.append(arg)

		## No response expected
		return 0

	def send_directory(self, directory, nlcnt=2):
		if self.connected == 0:
			print("Cannot send Directory command. Not connected")
			return 1

		self.server_fd.send(b'Directory %s%s' %(directory, b'\n'*nlcnt))

		## No response expected
		return 0

	def send_expand_modules(self):
		if self.connected == 0:
			print("Cannot send Directory command. Not connected")
			return 1

		self.server_fd.send(b'expand-modules\n')

		for x in self.modules_expected:
			y = self.readline().split(b' ')

			if y[0] != b'Module-expansion':
				self.modules_expected = []

				print("In send_expand_modules() expected 'Module-expansion. Received: %s" %(y[0]))
				return 1

			if y[1] != x:
				self.modules_expected = []

				print("Expected module %s. Received %s" %(x, y[1]))

				return 1

		y = self.readline()

		self.modules_expected = []

		if y != b'ok':
			print("Expected 'ok'. Received %s" %y)

			return 1

		return 0

	def add_entry(self, base_dir, name, version, entry_type, modtime):
		f = open(base_dir + b'/CVS/Entries', 'rb')

		entries = f.read().split(b'\n')

		data = b''

		f.close()

		found = 0

		for x in entries:
			if x == b'':
				continue

			e = x.split(b'/')

			if name == e[1]:
				if e[0] != entry_type:
					print('[ERROR] Entry types don\'t match %s' %name)
					return 1

				found = 1
				if e[0] == b'F':
					## Fri Aug  6 16:14:16 2021
					dt = datetime.datetime.fromtimestamp(time.mktime(modtime))
					data += entry_type + b'/' + name + b'/' + version + b'/' + dt.strftime('%a %b %d %H:%M:%S %Y').encode('UTF-8') + b'//\n'
				else:
					data += entry_type + b'/' + name + b'////\n'
			else:
				data += x + b'\n'

		if found == 0:
			if entry_type == b'F':
				dt = datetime.datetime.fromtimestamp(time.mktime(modtime))
				data += entry_type + b'/' + name + b'/' + version + b'/' + dt.strftime('%a %b %d %H:%M:%S %Y').encode('UTF-8') + b'//\n'
			else:
				data += entry_type + b'/' + name + b'////\n'

		f = open(base_dir + b'/CVS/Entries', 'wb')
		f.write(data)
		f.close()

		return 0

	def checkout_file(self, modtime):
		t = modtime[9:]

		y = self.readline()

		if y != b'MT +updated':
			print('[FAIL] Expected: MT +updated Received %s' %(y))
			return 1

		y = self.readline()

		if y != b'MT text U ':
			print('[FAIL] Expected: MT text U Received %s' %(y))
			return 1

		y = self.readline().split(b' ')

		if y[0] != b'MT':
			print('[FAIL] Expected: MT Received %s' %(y[0]))
			return 1

		if y[1] != b'fname':
			print('[FAIL] Expected: fname Received %s' %(y[1]))
			return 1

		fp = y[2][:]
		last_slash = y[2].rfind(b'/')
		fname = y[2][last_slash + 1:]
		dirs = y[2][:last_slash].split(b'/')

		y = self.readline()

		if y != b'MT newline':
			print('[FAIL] Expected: MT newline Received %s' %(y))
			return 1

		y = self.readline()

		if y != b'MT -updated':
			print('[FAIL] Expected: MT -updated Received %s' %(y))
			return 1

		## Create all of the subdirs if necessary
		subby = b''

		for x in dirs:
			subby += x

			if os.path.exists(subby) == False:
				os.mkdir(subby)
				os.mkdir(subby + b'/CVS')

				## Make the blank entries file
				open(subby + b'/CVS/Entries', 'a').close()

				self.add_entry(subby[:-1 * len(x)], x, None, b'D', None)


			subby += b'/'

		y = self.readline().split(b' ')

		if y[0] != b'Created':
			print('[FAIL] Expected: Created Received %s' %(y[0]))
			return 1

		if y[1] != subby:
			print('[FAIL] Expected %s Received %s' %(subby, y[1]))
			return 1

		y = self.readline()

		if y != fp:
			print('[FAIL] Expected: %s Received %s' %(subby, y))
			return 1

		## version
		version = self.readline().split(b'/')[2]

		perms = self.readline()
		size = self.readline()

		data = self.server_fd.recv(int(size))

		f = open(fp, 'wb')
		f.write(data)
		f.close()

		# 9 Aug 2021 13:13:18 -0000
		datevar = time.strptime(t.decode('UTF-8'), '%d %b %Y %H:%M:%S %z')

		os.utime(fp, (time.mktime(datevar), time.mktime(datevar)))

		self.add_entry(subby, fname, version, b'F', datevar)

		print('[INFO] Checkout %s successful' %(fp))

		return 0
		

	def checkout_folder(self, clear_sticky):
		clear_sticky = clear_sticky.split(b' ')

		repo = clear_sticky[1]

		y = self.readline()

		if y != repo:
			print('[FAIL] Expected \'%s\'. Received %s' %(repo, y))
			return 1

		y = self.readline().split(b' ')

		if y[0] != b'Clear-static-directory':
			print('[FAIL] Expected \'Clear-static-directory\'. Received %s' %(y[0]))
			return 1

		if y[1] != repo:
			print('[FAIL] Expected \'%s\'. Received %s' %(repo, y[1]))
			return 1

		y = self.readline()

		if y != repo:
			print('[FAIL] Expected \'%s\'. Received %s' %(repo, y))
			return 1

		y = self.readline().split(b' ')

		if y[0] != b'Clear-template':
			print('[FAIL] Expected \'Clear-template\'. Received %s' %(y[0]))
			return 1

		if y[1] != repo:
			print('[FAIL] Expected \'%s\'. Received %s' %(repo, y[1]))
			return 1

		y = self.readline()

		if y != repo:
			print('[FAIL] Expected \'%s\'. Received %s' %(repo, y))
			return 1

		y = self.readline().split(b': ')

		if y[0] != b'E cvs checkout':
			print('[FAIL] Expected\'E cvs checkout\'. Received %s' %(y[0]))
			return 1

		if y[1] != b'Updating %s' %(repo[:-1]):
			print('[FAIL] Expected\'Updating %s\'. Received %s' %(repo[:-1], y[1]))
			return 1

		y = self.readline()

		while y != b'Clear-sticky %s' %repo:
			## Mod time if it is a file Clear-sticky if it is a subdir
			if y.startswith(b'Mod-time') == True:
				if self.checkout_file(y):
					return 1
			elif y.startswith(b'Clear-sticky') == True:
				print('INFO GOING DEEPER')
				if self.checkout_folder(y):
					return 1
			else:
				print('[FAIL] Expected either Mod-time or Clear-sticky. Received %s' %(y))
				return 1

			y = self.readline()

		y = self.readline()

		if y != repo:
			print('[FAIL] Expected %s. Received %s' %(repo, y[1]))
			return 1

		return 0


	def checkout(self, repo):
		"""
		Checkout a cvs repository specified by repo
		"""
		if self.send_command_prep(b'checkout'):
			return 1

		if self.send_argument(repo):
			return 1

		if self.send_directory(b'.'):
			return 1

		if self.send_expand_modules():
			return 1

		if self.send_argument(b'--'):
			return 1

		if self.send_argument(repo):
			return 1

		if self.send_directory(b'.'):
			return 1

		print('Sending co')

		self.server_fd.send(b'co\n')

		y = self.readline().split(b' ')

		if y[0] != b'Clear-sticky':
			print('[FAIL] Expected \'Clear-sticky\'. Received %s' %(y[0]))
			return 1

		if y[1] != repo + b'/':
			print('[FAIL] Expected \'%s\\\'. Received %s' %(repo, y[1]))
			return 1

		y = self.readline()

		if y != repo + b'/':
			print('[FAIL] Expected \'%s\\\'. Received %s' %(repo, y))
			return 1

		y = self.readline().split(b' ')

		if y[0] != b'Clear-static-directory':
			print('[FAIL] Expected \'Clear-static-directory\'. Received %s' %(y[0]))
			return 1

		if y[1] != repo + b'/':
			print('[FAIL] Expected \'%s\\\'. Received %s' %(repo, y[1]))
			return 1

		y = self.readline()

		if y != repo + b'/':
			print('[FAIL] Expected \'%s\\\'. Received %s' %(repo, y))
			return 1

		y = self.readline().split(b' ')

		if y[0] != b'Clear-template':
			print('[FAIL] Expected \'Clear-template\'. Received %s' %(y[0]))
			return 1

		if y[1] != repo + b'/':
			print('[FAIL] Expected \'%s\\\'. Received %s' %(repo, y[1]))
			return 1

		y = self.readline()

		if y != repo + b'/':
			print('[FAIL] Expected \'%s\\\'. Received %s' %(repo, y))
			return 1

		y = self.readline().split(b': ')

		if y[0] != b'E cvs checkout':
			print('[FAIL] Expected\'E cvs checkout\'. Received %s' %(y[0]))
			return 1

		if y[1] != b'Updating %s' %(repo):
			print('[FAIL] Expected\'Updating %s\'. Received %s' %(repo, y[1]))
			return 1

		## Try making the directory for the repo
		try:
			os.mkdir(repo)
		except:
			## THe directory already exists
			pass

		try:
			os.mkdir(repo + b'/CVS')
		except:
			## THe directory already exists
			pass

		## Make the blank entries file
		open(repo + b'/CVS/Entries', 'a').close()

		### After this will begin the files
		y = self.readline()

		while y != b'ok':
			## Mod time if it is a file Clear-sticky if it is a subdir
			if y.startswith(b'Mod-time') == True:
				if self.checkout_file(y):
					return 1
			elif y.startswith(b'Clear-sticky') == True:
				if self.checkout_folder(y):
					return 1
			else:
				print('[FAIL] Expected either Mod-time or Clear-sticky. Received %s' %(y))
				return 1

			y = self.readline()

	def cmd_add_modified(self, repo, file):
		if self.send_command_prep(b'add'):
			return 1

		self.server_fd.send(b'wrapper-sendme-rcsOptions\n')

		y = self.readline()

		if y != b'ok':
			print('[FAIL] Expected ok. Received %s' %y)
			return 1

		if self.send_argument(b'--'):
			return 1

		if self.send_directory(b'.', 1):
			return 1

		print(b'AAA' + repo)
		self.server_fd.send(repo + b'\n')

		self.server_fd.send(b'Is-modified ' + file.split(b'/')[-1] + b'\n')

		if self.send_argument(file.split(b'/')[-1]):
			return 1

		self.server_fd.send(b'add\n')

		y = self.readline()

		if y != b'E cvs add: scheduling file \`%s\' for addition' %(file):
			print('[FAIL] Unexpected data received at cvs add: %s' %(y))
			print(file)
			#return 1

		y = self.readline()

		if y != b'Checked-in ./':
			print('[FAIL] Expected Checked-in ./. Received %s' %y)
			#return 1

		y = self.readline()

		if y != repo + b'/' + file:
			print('[FAIL] Expected %s/%s. Received %s' %(repo, file, y))
			#return 1

		y = self.readline()
		print(y)
		y = self.readline()

		if y != b'ok':
			print('[FAIL] Expected ok. Received %s' %(y))
			#return 1

		## Modify file here? Add entry?

		return 0

	def get_version(self, file):
		## expects the full path
		fname = file.split(b'/')[-1]
		path = b'/'.join(file.split(b'/')[:-1])

		f = open(path + b'/CVS/Entries')

		entries = f.read().split('\n')

		f.close()

		for x in entries:
			if x == '':
				continue

			e = x.split('/')
			if e[1] != fname.decode('UTF-8'):
				continue

			return e[2].encode('UTF-8')

		print('[FAIL] No entry found: %s' %file)
		exit(1)

	def cmd_commit(self, repo, file, data, is_new=False):
		print('[TEST] cmd_commit')

		if self.send_command_prep(b'commit'):
			return 1

		if self.send_argument(b'-m'):
			return 1

		if self.send_argument(b'adding test'):
			return 1

		if self.send_argument(b'--'):
			return 1

		if self.send_directory(b'.', 1):
			return 1

		basedir = file[:file.rfind(b'/')]

		self.server_fd.send(basedir + b'\n')

		if is_new:
			self.server_fd.send(b'Entry /' + file.split(b'/')[-1] + b'/0///\n')
		else:
			self.server_fd.send(b'Entry /' + file.split(b'/')[-1] + b'/' + self.get_version(file) + b'///\n')

		self.server_fd.send(b'Modified ' + file.split(b'/')[-1] + b'\n')

		self.server_fd.send(b'u=rw,g=rw,o=rw\n')

		self.server_fd.send(str(len(data)).encode('UTF-8') + b'\n')

		self.server_fd.send(data)

		if self.send_argument(file.split(b'/')[-1]):
			return 1

		self.server_fd.send(b'ci\n')

		y = self.readline()
		y = self.readline()
		y = self.readline()

		y = self.readline()
		y = self.readline()

		y = self.readline().split(b'/')

		self.add_entry(b'/'.join(file.split(b'/')[:-1]), y[1], y[2], b'F', time.localtime())

		y = self.readline()

		if y != b'ok':
			print('[FAIL] Expected ok. Received %s' %y)
			return 1

		print('[SUCCESS] cmd_commit')
		return 0

	def cmd_add(self, repo, fname, data):
		print('[TEST] cmd_add')

		just_the_file = fname.split(b'/')[-1]
		path = fname[:len(just_the_file) * -1]
		print(path)

		if self.send_command_prep(b'add'):
			return 1

		self.server_fd.send(b'wrapper-sendme-rcsOptions\n')

		y = self.readline()

		if y != b'ok':
			print('[ERROR] Expected ok. Received %s' %y)
			return 1

		if self.send_argument(b'--'):
			return 1

		if self.send_directory(b'.', 1):
			return 1

		self.server_fd.send(path + b'\n')

		self.server_fd.send(b'Is-modified %s\n' %just_the_file)

		if self.send_argument(just_the_file):
			return 1

		print('[DEBUG] Sending add')

		self.server_fd.send(b'add\n')

		y = self.readline()

		d = b'E cvs add: scheduling file `%s\' for addition' %just_the_file

		if y != d:
			print('[ERROR] Expected "%s". Received %s' %(d, y))
			return 1

		y = self.readline()

		if y != b'Checked-in ./':
			print('[ERROR] Expected Checked-in ./. Received %s'%y)
			return 1

		y = self.readline()

		if y != fname:
			print('[ERROR] Expected %s. Received %s' %(fname, y))
			return 1

		d = b'/%s/0///' %(just_the_file)

		y = self.readline()

		if y != d:
			print('[ERROR] Expected %s. Received %s' %(d, y))
			return 1

		y = self.readline()

		d = b'E cvs add: use `cvs commit\' to add this file permanently'

		if y != d:
			print('[ERROR] Expected %s. Received %s' %(d,y))
			return 1

		y = self.readline()

		if y != b'ok':
			print('[ERROR] Expected ok. Received %s' %(y))
			return 1

		print('[SUCCESS] cmd_add')
		return 0

	def cmd_version(self, repo):
		print('[TEST] cmd_version')

		if self.send_command_prep(b'version'):
			return 1

		self.server_fd.send(b'version\n')

		y = self.readline()

		d = b'M Concurrent Versions System (CVS) 1.12.13 (client/server)'

		if y != d:
			print('[FAIL] Expected %s. Received %s' %(d,y))
			return 1

		y = self.readline()

		if y != b'ok':
			print('[FAIL] Expected ok. Received %s' %y)
			return 1

		print('[SUCCESS] cmd_version')
		return 0

	def create_and_commit(self, repo):
		## Select the subdirectory under which we will create the file
		subdir = random.choice(self.gen_dir_list(repo))

		## Generate teh data
		data = ''.join(random.choice(string.ascii_lowercase + ' ') for _ in range(20))

		## Generate the name
		name = ''.join(random.choice(string.ascii_lowercase) for _ in range(20)) + '.txt'

		name = name.encode('UTF-8')
		data = data.encode('UTF-8')

		f = open(subdir + b'/' + name, 'wb')
		f.write(data)
		f.close()

		if self.cmd_add(repo, subdir + b'/' + name, data):
			return 1

		print('[INFO] Added %s' %(subdir + b'/' + name))

		if self.cmd_commit(repo, subdir + b'/' + name, data, True):
			return 1

		print('[INFO] Committed %s' %(subdir + b'/' + name))

		return 0

	def cmd_add_tag(self, repo, tag):
		print('[TEST] cmd_add_tag')

		paths = self.gen_dir_list(repo)

		## add the tag
		if self.send_command_prep(b'rtag'):
			return 1

		if self.send_argument(b'--'):
			return 1

		if self.send_argument(tag):
			return 1

		if self.send_argument(repo):
			return 1

		self.server_fd.send(b'rtag\n')

		responses = []

		## Generate the list of possible response strings
		for p in paths:
			responses.append(b'E cvs rtag: Tagging %s' %p)

		print(paths)

		while(len(responses)):
			y = self.readline()

			if y not in responses:
				print('[FAIL] Expected a tagging string. Received %s' %y)
				return 1

			responses.remove(y)

		y = self.readline()

		if y != b'ok':
			print('[FAIL] Expected ok. Received %s' %y)
			return 1

		return 0

	def cmd_rem_tag(self, repo, tag):
		print('[TEST] cmd_rem_tag')

		paths = self.gen_dir_list(repo)

		## add the tag
		if self.send_command_prep(b'rtag'):
			return 1

		if self.send_argument(b'-d'):
			return 1

		if self.send_argument(b'--'):
			return 1

		if self.send_argument(tag):
			return 1

		if self.send_argument(repo):
			return 1

		self.server_fd.send(b'rtag\n')

		responses = []

		## Generate the list of possible response strings
		for p in paths:
			responses.append(b'E cvs rtag: Untagging %s' %p)

		print(paths)

		while(len(responses)):
			y = self.readline()

			if y not in responses:
				print('[FAIL] Expected an untagging string. Received %s' %y)
				return 1

			responses.remove(y)

		y = self.readline()

		if y != b'ok':
			print('[FAIL] Expected ok. Received %s' %y)
			return 1

		print('[SUCCESS] cmd_rem_tag')
		return 0

	def cmd_rls(self, repo):
		print('[TEST] cmd_rls')

		if self.send_command_prep(b'rls'):
			return 1

		if self.send_argument(b'-R'):
			return 1

		if self.send_argument(b'--'):
			return 1

		if self.send_argument(b'cvstesta'):
			return 1

		self.server_fd.send(b'rlist\n')

		d = b'E cvs rls: Listing module: `%s\'' %repo

		y = self.readline()

		if y != d:
			print('[FAIL] Expected %s. Received %s' %(d,y))
			return 1

		y = self.readline()

		while y != b'ok':
			y = y.split(b' ')

			if y[0] != b'M':
				print('[FAIL] Expected folder M. Received %s' %y)
				return 1

			folder = y[1][:-1]
			print('FOLDER: %s' %folder)

			dirs, subdirs, files, = next(os.walk(folder))

			responses = []

			## Generate the list of expected responses
			for sd in subdirs:
				if sd == b'CVS':
					continue

				responses.append(b'M %s' %sd)

			for f in files:
				responses.append(b'M %s' %f)

			while len(responses):
				y = self.readline()

				if y not in responses:
					print('[FAIL] Expected an ls response. Received %s' %y)
					return 1

				responses.remove(y)

			## Eat the blank
			y = self.readline()
			if y == b'M ':
				print('reading m again')
				y = self.readline()

		print('[SUCCESS] cmd_rls')
		return 0

	def send_entry(self, file, version):
		if self.connected == 0:
			print("Cannot send Entry command. Not connected")
			return 1

		self.server_fd.send(b'Entry /%s/%s///\n' %(file, version))

		## No response expected
		return 0

	def cmd_watch(self, repo):
		print('[TEST] cmd_watch')

		## Select the directories to watch
		paths = self.gen_dir_list(repo)

		if self.send_command_prep(b'watch'):
			return 1

		if self.send_argument(b'-a'):
			return 1

		if self.send_argument(b'edit'):
			return 1

		if self.send_argument(b'-a'):
			return 1

		if self.send_argument(b'unedit'):
			return 1

		if self.send_argument(b'-a'):
			return 1

		if self.send_argument(b'commit'):
			return 1

		if self.send_argument(b'--'):
			return 1

		## Select a path
		p = random.choice(paths)

		print('[INFO] path: %s' %p)

		if self.send_directory(p, 1):
			return 1

		self.server_fd.send(p + b'\n')

		## Get the files in that directory
		_, _, files = next(os.walk(p))

		for f in files:
			if self.send_entry(f, self.get_version(p + b'/' + f)):
				return 1

			self.server_fd.send(b'Unchanged %s\n' %(f))

		if self.send_directory(b'.'):
			return 1

		if self.send_argument(p):
			return 1

		self.server_fd.send(b'watch-add\n')

		y = self.readline()

		if y != b'ok':
			print('[FAIL] Expected ok. Received %s' %y)
			return 1


		## Remove the watches
		if self.send_command_prep(b'watch'):
			return 1

		if self.send_argument(b'-a'):
			return 1

		if self.send_argument(b'edit'):
			return 1

		if self.send_argument(b'-a'):
			return 1

		if self.send_argument(b'unedit'):
			return 1

		if self.send_argument(b'-a'):
			return 1

		if self.send_argument(b'commit'):
			return 1

		if self.send_argument(b'--'):
			return 1

		if self.send_directory(p, 1):
			return 1

		self.server_fd.send(p + b'\n')

		## Get the files in that directory
		_, _, files = next(os.walk(p))

		for f in files:
			if self.send_entry(f, self.get_version(p + b'/' + f)):
				return 1

			self.server_fd.send(b'Unchanged %s\n' %(f))

		if self.send_directory(b'.'):
			return 1

		if self.send_argument(p):
			return 1

		self.server_fd.send(b'watch-remove\n')

		y = self.readline()

		if y != b'ok':
			print('[FAIL] Expected ok. Received %s' %y)
			return 1

		return 0

	def test_rtag(self, repo):
		print('[TEST] test_rtag')

		## create the tag
		tag = ''.join(random.choice(string.ascii_lowercase) for _ in range(20))
		tag = tag.encode('UTF-8')

		if self.cmd_add_tag(repo, tag):
			return 1

		if self.cmd_rem_tag(repo, tag):
			return 1

		return 0


	def gen_file_list(self, base_dir):
		paths = []

		for rootdir, subdir, files in os.walk(base_dir):
			if rootdir.find(b'CVS') != -1:
				continue
			for f in files:
				paths.append(os.path.join(rootdir, f))

		return paths

	def gen_dir_list(self, base_dir):
		paths = []

		for rootdir, subdir, files in os.walk(base_dir):
			if rootdir.find(b'CVS') != -1:
				continue
			paths.append(rootdir)

		return paths


	def modify_and_commit(self, repo):
		## Select a file
		paths = self.gen_file_list(repo)

		modfile = random.choice(paths)

		## modify it. Don't worry about the other data just generate new stuff
		data = ''.join(random.choice(string.ascii_lowercase + ' ') for _ in range(20))

		## command commit
		if self.cmd_commit(repo, modfile, data.encode('UTF-8')):
			return 1

		## write it to disk locally after
		f = open(modfile, 'wb')
		f.write(data.encode('UTF-8'))
		f.close()

		print('[INFO] Modified and committed %s' %modfile)
		return 0


	def scramble( self, password ):
		""" Scrambles the password according to the cvs protocol. It isn't actually
			secure but I guess at least it isn't cleartext.

			Input: The password to be scrambled
			Output: Returns the scrambled password in the same type (byte or string) that it was provided.
		"""

		if type(password) == type(''):
			pass_temp = password.encode('UTF-8')
		elif type(password) == type(b''):
			pass_temp = password
		else:
			raise TypeError(f'password must be <class \'bytes\'> or <class \'str\'> gave: {type(password)}')

		## The 'A' indicates the type of scrambling. There is only one type
		s = b'A'

		for x in pass_temp:
			s += chr(self.shifts[x]).encode('UTF-8')

		return s

if __name__ == '__main__':
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = os.environ['PORT']
        else:
            print('[USAGE] %s <target_host> <target_port>' %(sys.argv[0]))
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] Poller for the Cherry Challenge')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    try:
    	shutil.rmtree('./cvstesta')
    except:
    	pass

    ## Do the initial connection
    a = cvs(HOST, 'vagrant', 'vagrant', '/srv/cvsroot')

    REPO = b'cvstesta'

    ## Inital check out of the repo
    a.checkout(REPO)

    funcs = [a.create_and_commit, a.modify_and_commit, a.cmd_version, a.test_rtag, a.cmd_watch]

    for i in range(10):
    	f = random.choice(funcs)
    	
    	result = f(REPO)

    	if result:
    		print('[FAIL] Poller was unsuccessful')
    		a.server_fd.close()
    		exit(1)

    print('[SUCCESS] Poller completed successfully')
    a.server_fd.close()

    exit(0)