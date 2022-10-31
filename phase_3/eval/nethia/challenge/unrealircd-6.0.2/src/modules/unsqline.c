/*
 *   Unreal Internet Relay Chat Daemon, src/modules/unsqline.c
 *   (C) 2000-2001 Carsten V. Munk and the UnrealIRCd Team
 *   Moved to modules by Fish (Justin Hammond)
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

CMD_FUNC(cmd_unsqline);

#define MSG_UNSQLINE    "UNSQLINE"      /* UNSQLINE */

ModuleHeader MOD_HEADER
  = {
	"unsqline",	/* Name of module */
	"5.0", /* Version */
	"command /unsqline", /* Short description of module */
	"UnrealIRCd Team",
	"unrealircd-6",
    };

/* This is called on module init, before Server Ready */
MOD_INIT()
{
	CommandAdd(modinfo->handle, MSG_UNSQLINE, cmd_unsqline, MAXPARA, CMD_SERVER);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

/* Is first run when server is 100% ready */
MOD_LOAD()
{
	return MOD_SUCCESS;
}

/* Called when module is unloaded */
MOD_UNLOAD()
{
	return MOD_SUCCESS;
}

/* cmd_unsqline
**	parv[1] = nickmask
*/
CMD_FUNC(cmd_unsqline)
{
	const char *tkllayer[6] = {
		me.name,           /*0  server.name */
		"-",               /*1  - */
		"Q",               /*2  Q   */
		"*",               /*3  unused */
		parv[1],           /*4  host */
		client->name       /*5  whoremoved */
	};

	if (parc < 2)
		return;

	cmd_tkl(&me, NULL, 6, tkllayer);
}
