#include "client.hpp"
#include "server.hpp"
int Server::NickExists(std::string nick) {
  Client *walker = NULL;
  for (auto it = this->clients.begin(); it != this->clients.end(); ++it) {
    walker = *it;
    if (walker->GetNick() == nick) {
      return 1;
    }
  }
  return 0;
}
int Server::SendBanner(int fd) {
  std::string banner =
      ":irc." + this->GetServerHostname() +
      " 020 * :Please wait while we process your connection.\r\n";
  return write(fd, banner.c_str(), banner.size());
}
int Server::RemoveClientByNick(std::string nick) {
  Client *walker = NULL;
  for (auto it = this->clients.begin(); it != this->clients.end(); ++it) {
    walker = *it;
    if (walker->GetNick() == nick) {
      this->clients.erase(it);
      walker->ShutdownClient();
      return SUCCESS;
    }
  }
  return FAIL;
}
int Server::SendServerInfo(Client *c) {
  std::string response;
  std::string base = ":irc." + this->GetServerHostname() + " ";
  response = base + "001 " + c->GetNick() +
             " :Welcome to the CHESS Internet Relay Chat Server " +
             c->GetNick() + "\r\n";
  c->AddLineToResponseQueue(response);
  response = base + "002 " + c->GetNick() + " :Your host is irc." +
             this->GetServerHostname() + ":" + std::to_string(this->GetPort()) +
             ", running version 0.0.0.0.0.0.1-a\r\n";
  c->AddLineToResponseQueue(response);
  response = base + "003 " + c->GetNick() +
             " :This server was created Fri Jul 13 00:00:00 UTC 2001\r\n";
  c->AddLineToResponseQueue(response);
  response = base + "004 " + c->GetNick() + " irc." +
             this->GetServerHostname() + " 0.0.0.0.0.0.1-a aiov psntmilk\r\n";
  c->AddLineToResponseQueue(response);
  response = base + "005 " + c->GetNick() +
             " KNOCK CHANTYPES=&# CHANMODES=klntmps CHANLIMIT&#:15 "
             "PREFIX=(ov)@+ NICKLEN=30 MAXNICKLEN=31 CHANNELLEN=50 :are "
             "supported by this server\r\n";
  c->AddLineToResponseQueue(response);
  return SUCCESS;
}
Channel *Server::GetChannelByName(std::string name) {
  Channel *c = NULL;
  for (auto it = this->channels.begin(); it != this->channels.end(); ++it) {
    c = *it;
    if (c->GetChannelName() == name) {
      return c;
    }
  }
  return NULL;
}
Client *Server::GetClientByNick(std::string nick) {
  Client *c = NULL;
  for (auto it = this->clients.begin(); it != this->clients.end(); ++it) {
    c = *it;
    if (c->GetNick() == nick) {
      return c;
    }
  }
  return NULL;
}
int Server::HandleCommand() {
  std::string sender = "";
  if (this->tokens.size() <= 0) {
    return FAIL;
  }
  if (this->tokens[0][0] == ':') {
    sender = this->tokens[0];
    this->tokens.erase(this->tokens.begin());
  }
  if (cppstrncasecmp(this->tokens[0], "NICK")) {
    this->HandleNICK();
  } else if (cppstrncasecmp(this->tokens[0], "AWAY")) {
    this->HandleAWAY();
  } else if (cppstrncasecmp(this->tokens[0], "CAP")) {
    this->HandleCAP();
  } else if (cppstrncasecmp(this->tokens[0], "INVITE")) {
    this->HandleINVITE();
  } else if (cppstrncasecmp(this->tokens[0], "ISON")) {
    this->HandleISON();
  } else if (cppstrncasecmp(this->tokens[0], "JOIN")) {
    this->HandleJOIN();
  } else if (cppstrncasecmp(this->tokens[0], "KICK")) {
    this->HandleKICK();
  } else if (cppstrncasecmp(this->tokens[0], "KNOCK")) {
    this->HandleKNOCK();
  } else if (cppstrncasecmp(this->tokens[0], "LIST")) {
    this->HandleLIST();
  } else if (cppstrncasecmp(this->tokens[0], "MODE")) {
    this->HandleMODE();
  } else if (cppstrncasecmp(this->tokens[0], "NAMES")) {
    this->HandleNAMES();
  } else if (cppstrncasecmp(this->tokens[0], "NOTICE")) {
    this->HandleNOTICE();
  } else if (cppstrncasecmp(this->tokens[0], "PART")) {
    this->HandlePART();
  } else if (cppstrncasecmp(this->tokens[0], "PASS")) {
  } else if (cppstrncasecmp(this->tokens[0], "PING")) {
    this->HandlePING();
  } else if (cppstrncasecmp(this->tokens[0], "PONG")) {
  } else if (cppstrncasecmp(this->tokens[0], "PRIVMSG")) {
    this->HandlePRIVMSG();
  } else if (cppstrncasecmp(this->tokens[0], "QUIT")) {
    this->HandleQUIT();
  } else if (cppstrncasecmp(this->tokens[0], "SILENCE")) {
    this->HandleSILENCE();
  } else if (cppstrncasecmp(this->tokens[0], "TIME")) {
    this->HandleTIME();
  } else if (cppstrncasecmp(this->tokens[0], "TOPIC")) {
    this->HandleTOPIC();
  } else if (cppstrncasecmp(this->tokens[0], "USER")) {
    this->HandleUSER();
  } else if (cppstrncasecmp(this->tokens[0], "WHO")) {
    this->HandleWHO();
  } else if (cppstrncasecmp(this->tokens[0], "WHOIS")) {
    this->HandleWHOIS();
  }
  return SUCCESS;
}