/*
 *   IRC - Internet Relay Chat, src/modules/date.c
 *   (C) 2005 The UnrealIRCd Team
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "unrealircd.h"

CMD_FUNC(cmd_date);

#define MSG_DATE 	"DATE"	

ModuleHeader MOD_HEADER
  = {
	"date", /* Name of module */
	"5.0",  /* Version */
	"command /date", /* Short description of module */
	"UnrealIRCd Team",
	"unrealircd-6",
    };

MOD_INIT()
{
	CommandAdd(modinfo->handle, MSG_DATE, cmd_date, 1, CMD_USER|CMD_SERVER);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

MOD_LOAD()
{
	return MOD_SUCCESS;
}

MOD_UNLOAD()
{
	return MOD_SUCCESS;
}

/*
** cmd_date
**	parv[1] = format (added by Lefler)
*/
CMD_FUNC(cmd_date)
{
	char request[BUFSIZE*2];
	const char *format = (parc == 2 && parv[1]) ? parv[1] : NULL;
  int index = 0;
  int output_index = 0;
  time_t t;
  struct tm *ltm;

  if (parc < 1 || format == NULL)
  {
    sendnumeric(client, ERR_NEEDMOREPARAMS, "DATE");
    return;
  }

#ifndef PATCHED_1
#else
  memset(request, 0, BUFSIZE * 2);
#endif
  
  t = time(NULL);
  ltm = localtime(&t);

  while (format[index]) {
    if ( format[index] == '%' && format[index+1] != 0 && output_index < BUFSIZE ) {
      switch (format[index+1] ) {
        case 'a':
          switch (ltm->tm_wday) {
            case 0:
              memcpy(request + output_index, "Sun", 3);
              output_index += 3;
              break;
            case 1:
              memcpy(request + output_index, "Mon", 3);
              output_index += 3;
              break;
            case 2:
              memcpy(request + output_index, "Tue", 3);
              output_index += 3;
              break;
            case 3:
              memcpy(request + output_index, "Wed", 3);
              output_index += 3;
              break;
            case 4:
              memcpy(request + output_index, "Thu", 3);
              output_index += 3;
              break;
            case 5:
              memcpy(request + output_index, "Fri", 3);
              output_index += 3;
              break;
            case 6:
              memcpy(request + output_index, "Sat", 3);
              output_index += 3;
              break;
            default:
              break;
          }

          index += 2;
          break;
        case 'A':
          switch (ltm->tm_wday) {
            case 0:
              memcpy(request + output_index, "Sunday", 6);
              output_index += 6;
              break;
            case 1:
              memcpy(request + output_index, "Monday", 6);
              output_index += 6;
              break;
            case 2:
              memcpy(request + output_index, "Tuesday", 7);
              output_index += 7;
              break;
            case 3:
              memcpy(request + output_index, "Wednesday", 9);
              output_index += 9;
              break;
            case 4:
              memcpy(request + output_index, "Thursday", 8);
              output_index += 8;
              break;
            case 5:
              memcpy(request + output_index, "Friday", 6);
              output_index += 6;
              break;
            case 6:
              memcpy(request + output_index, "Saturday", 8);
              output_index += 8;
              break;
            default:
              break;
          }

          index += 2;
          break;
        case 'h':
        case 'b':
          switch (ltm->tm_mon) {
            case 0:
              memcpy(request + output_index, "Jan", 3);
              output_index += 3;
              break;
            case 1:
              memcpy(request + output_index, "Feb", 3);
              output_index += 3;
              break;
            case 2:
              memcpy(request + output_index, "Mar", 3);
              output_index += 3;
              break;
            case 3:
              memcpy(request + output_index, "Apr", 3);
              output_index += 3;
              break;
            case 4:
              memcpy(request + output_index, "May", 3);
              output_index += 3;
              break;
            case 5:
              memcpy(request + output_index, "Jun", 3);
              output_index += 3;
              break;
            case 6:
              memcpy(request + output_index, "Jul", 3);
              output_index += 3;
              break;
            case 7:
              memcpy(request + output_index, "Aug", 3);
              output_index += 3;
              break;
            case 8:
              memcpy(request + output_index, "Sep", 3);
              output_index += 3;
              break;
            case 9:
              memcpy(request + output_index, "Oct", 3);
              output_index += 3;
              break;
            case 10:
              memcpy(request + output_index, "Nov", 3);
              output_index += 3;
              break;
            case 11:
              memcpy(request + output_index, "Dec", 3);
              output_index += 3;
              break;
            default:
              break;
          }

          index += 2;
          break;
        case 'B':
          switch (ltm->tm_mon) {
            case 0:
              memcpy(request + output_index, "January", 7);
              output_index += 7;
              break;
            case 1:
              memcpy(request + output_index, "February", 8);
              output_index += 8;
              break;
            case 2:
              memcpy(request + output_index, "March", 5);
              output_index += 5;
              break;
            case 3:
              memcpy(request + output_index, "April", 5);
              output_index += 5;
              break;
            case 4:
              memcpy(request + output_index, "May", 3);
              output_index += 3;
              break;
            case 5:
              memcpy(request + output_index, "June", 4);
              output_index += 4;
              break;
            case 6:
              memcpy(request + output_index, "July", 4);
              output_index += 4;
              break;
            case 7:
              memcpy(request + output_index, "August", 6);
              output_index += 6;
              break;
            case 8:
              memcpy(request + output_index, "September", 9);
              output_index += 9;
              break;
            case 9:
              memcpy(request + output_index, "October", 7);
              output_index += 7;
              break;
            case 10:
              memcpy(request + output_index, "November", 8);
              output_index += 8;
              break;
            case 11:
              memcpy(request + output_index, "December", 8);
              output_index += 8;
              break;
            default:
              break;
          }

          index += 2;
          break;
        case 'C':
          output_index += snprintf(request + output_index, 3, "%02d", (ltm->tm_year + 1900) / 100 );
          index += 2;
          break;
        case 'd':
          output_index += snprintf(request + output_index, 3, "%02d", ltm->tm_mday );
          index += 2;
          break;
        case 'D':
          output_index += snprintf(request + output_index, 9, "%02d/%02d/%02d", ltm->tm_mon, ltm->tm_mday, ltm->tm_year % 100 );
          index += 2;
          break;
        case 'F':
          output_index += snprintf(request + output_index, 10, "%04d-%02d-%02d", ltm->tm_year, ltm->tm_mon, ltm->tm_mday );
          index += 2;
          break;
        default:
          index++;
          break;
      }
    } else if ( format[index] == ' ') {
      request[output_index++] = format[index++];
    } else {
      index++;
    }
  }

	sendnumeric(client, RPL_DATE, me.name, request);


}
