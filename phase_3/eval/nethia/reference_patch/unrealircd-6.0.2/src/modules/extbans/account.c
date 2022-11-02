/*
 * Extended ban to ban/exempt by services account (~b ~a:accountname)
 * (C) Copyright 2011-.. Bram Matthys (Syzop) and the UnrealIRCd team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "unrealircd.h"

ModuleHeader MOD_HEADER
= {
	"extbans/account",
	"4.2",
	"ExtBan ~a - Ban/exempt by services account name",
	"UnrealIRCd Team",
	"unrealircd-6",
};

/* Forward declarations */
const char *extban_account_conv_param(BanContext *b, Extban *extban);
int extban_account_is_banned(BanContext *b);

/** Called upon module init */
MOD_INIT()
{
	ExtbanInfo req;
	
	memset(&req, 0, sizeof(req));
	req.letter = 'a';
	req.name = "account";
	req.is_ok = NULL;
	req.conv_param = extban_account_conv_param;
	req.is_banned = extban_account_is_banned;
	req.is_banned_events = BANCHK_ALL|BANCHK_TKL;
	req.options = EXTBOPT_INVEX|EXTBOPT_TKL;
	if (!ExtbanAdd(modinfo->handle, req))
	{
		config_error("could not register extended ban type");
		return MOD_FAILED;
	}

	MARK_AS_OFFICIAL_MODULE(modinfo);
	
	return MOD_SUCCESS;
}

/** Called upon module load */
MOD_LOAD()
{
	return MOD_SUCCESS;
}

/** Called upon unload */
MOD_UNLOAD()
{
	return MOD_SUCCESS;
}

/** Account bans */
const char *extban_account_conv_param(BanContext *b, Extban *extban)
{
	char *mask, *acc;
	static char retbuf[NICKLEN + 4];

	strlcpy(retbuf, b->banstr, sizeof(retbuf)); /* truncate */

	acc = retbuf;
	if (!*acc)
		return NULL; /* don't allow "~a:" */

	return retbuf;
}

int extban_account_is_banned(BanContext *b)
{
	/* ~a:0 is special and matches all unauthenticated users */
	if (!strcmp(b->banstr, "0") && !IsLoggedIn(b->client))
		return 1;

	/* ~a:* matches all authenticated users
	 * (Yes this special code is needed because account
	 *  is 0 or * for unauthenticated users)
	 */
	if (!strcmp(b->banstr, "*") && IsLoggedIn(b->client))
		return 1;

	if (match_simple(b->banstr, b->client->user->account))
		return 1;

	return 0;
}
