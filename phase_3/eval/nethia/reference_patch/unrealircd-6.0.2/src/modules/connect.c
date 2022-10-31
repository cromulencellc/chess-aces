/*
 *   IRC - Internet Relay Chat, src/modules/out.c
 *   (C) 2004 The UnrealIRCd Team
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

CMD_FUNC(cmd_connect);

#define MSG_CONNECT 	"CONNECT"	

ModuleHeader MOD_HEADER
  = {
	"connect",
	"5.0",
	"command /connect", 
	"UnrealIRCd Team",
	"unrealircd-6",
    };

MOD_INIT()
{
	CommandAdd(modinfo->handle, MSG_CONNECT, cmd_connect, MAXPARA, CMD_USER|CMD_SERVER); /* hmm.. server.. really? */
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

/***********************************************************************
 * cmd_connect() - Added by Jto 11 Feb 1989
 ***********************************************************************//*
   ** cmd_connect
   **  parv[1] = servername
 */
CMD_FUNC(cmd_connect)
{
	int  retval;
	ConfigItem_link	*aconf;
	ConfigItem_deny_link *deny;
	Client *server;

	if (!IsServer(client) && MyConnect(client) && !ValidatePermissionsForPath("route:global",client,NULL,NULL,NULL) && parc > 3)
	{			/* Only allow LocOps to make */
		/* local CONNECTS --SRB      */
		sendnumeric(client, ERR_NOPRIVILEGES);
		return;
	}
	if (!IsServer(client) && MyUser(client) && !ValidatePermissionsForPath("route:local",client,NULL,NULL,NULL) && parc <= 3)
	{
		sendnumeric(client, ERR_NOPRIVILEGES);
		return;
	}
	if (hunt_server(client, recv_mtags, "CONNECT", 3, parc, parv) != HUNTED_ISME)
		return;

	if (parc < 2 || *parv[1] == '\0')
	{
		sendnumeric(client, ERR_NEEDMOREPARAMS, "CONNECT");
		return;
	}

	if ((server = find_server_quick(parv[1])))
	{
		sendnotice(client, "*** Connect: Server %s already exists from %s.",
		    parv[1], server->direction->name);
		return;
	}

	for (aconf = conf_link; aconf; aconf = aconf->next)
		if (match_simple(parv[1], aconf->servername))
			break;

	/* Checked first servernames, then try hostnames. */

	if (!aconf)
	{
		sendnotice(client,
		    "*** Connect: Server %s is not configured for linking",
		    parv[1]);
		return;
	}

	if (!aconf->outgoing.hostname)
	{
		sendnotice(client,
		    "*** Connect: Server %s is not configured to be an outgoing link (has a link block, but no link::outgoing::hostname)",
		    parv[1]);
		return;
	}

	/* Evaluate deny link */
	for (deny = conf_deny_link; deny; deny = deny->next)
	{
		if (deny->flag.type == CRULE_ALL && unreal_mask_match_string(aconf->servername, deny->mask)
			&& crule_eval(deny->rule))
		{
			sendnotice(client, "*** Connect: Disallowed by connection rule");
			return;
		}
	}

	/* Notify all operators about remote connect requests */
	if (!MyUser(client))
	{
		sendto_server(NULL, 0, 0, NULL,
		    ":%s SENDUMODE o :Remote CONNECT %s %s from %s",
		    me.id, parv[1], parv[2] ? parv[2] : "",
		    get_client_name(client, FALSE));
	}

	connect_server(aconf, client, NULL);
}
