#!/usr/bin/python

'''apport hook for vsftpd

(c) 2010 Andres Rodriguez.
Author: Andres Rodriguez <andreserl@ubuntu.com>

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.  See http://www.gnu.org/copyleft/gpl.html for
the full text of the license.
'''

from apport.hookutils import *

def add_info(report, ui):
    response = ui.yesno("The contents of your /etc/vsftpd.conf file "
                        "may help developers diagnose your bug more "
                        "quickly.  However, it may contain sensitive "
                        "information.  Do you want to include it in your "
                        "bug report?")

    if response == None: # user cancelled
        raise StopIteration

    elif response == True:
        attach_conffiles(report,'vsftpd')

    attach_file_if_exists(report, os.path.expanduser('/var/log/vsftpd.log'), 'vsftpd.log')
