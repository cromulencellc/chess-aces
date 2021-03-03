#include "channel.hpp"
#include "server.hpp"
Channel::Channel(std::string name) {
  this->channel_name = name;
  this->members.clear();
  this->topic = "";
  operators.clear();
  hasvoice.clear();
  key = "";
  isprivate = 0;
  issecret = 0;
  isinviteonly = 0;
  opertopic = 0;
  onlymembers = 0;
  ismoderated = 0;
  hasmax = 0;
  maxusers = 0;
  haskey = 0;
  return;
}
Channel::~Channel() {
  this->members.clear();
  this->operators.clear();
  this->invited.clear();
  this->hasvoice.clear();
  this->banlist.clear();
  return;
}
int Channel::GetMemberCount() { return this->members.size(); }
int Channel::IsBanned(std::string nick) {
  ban *b;
  std::smatch m;
  for (auto it = this->banlist.begin(); it != this->banlist.end(); ++it) {
    b = *it;
    if (std::regex_search(nick, m, b->banreg)) {
      return 1;
    }
  }
  return 0;
}
int Channel::AddBanExp(Client *c, std::string banexp) {
  ban *b = new ban;
  std::string response;
  if (!b) {
    return FAIL;
  }
  b->nick = c->GetNick();
  b->regexp = banexp;
  b->t = std::time(nullptr);
  try {
    b->banreg = std::regex(banexp);
  } catch (const std::exception &e) {
    delete b;
    return FAIL;
  }
  response = ":" + c->GetNick() + "!~" + c->GetUser() + "@" + c->GetHostname() +
             " MODE ";
  response += this->GetChannelName() + " +b " + b->regexp + "\r\n";
  this->banlist.push_back(b);
  this->SendAllClients(response);
  return SUCCESS;
}
void Channel::PrintBanList(Client *c, std::string serverhost) {
  std::string response;
  for (auto it = this->banlist.begin(); it != this->banlist.end(); ++it) {
    response = ":irc." + serverhost + " 367 " + c->GetNick() + " " +
               this->GetChannelName() + " " + (*it)->regexp;
    response += " " + (*it)->nick + " " + std::to_string((*it)->t) + "\r\n";
    c->AddLineToResponseQueue(response);
  }
  response = ":irc." + serverhost + " 368 " + c->GetNick() + " " +
             this->GetChannelName() + " :End of Channel Ban List\r\n";
  c->AddLineToResponseQueue(response);
  return;
}
std::string Channel::GetMemberList(void) {
  std::string response = "";
  std::string nick;
  Client *walker;
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    nick = "";
    if (this->HasVoice(walker->GetNick())) {
      nick += "+";
    }
    if (this->IsOperator(walker->GetNick())) {
      if (walker->CapSet(CAP_MULTI)) {
        nick = "@" + nick;
      } else {
        nick = "@";
      }
    }
    nick += walker->GetNick();
    response += nick + " ";
  }
  rtrim(response);
  return response;
}
void Channel::AddOperator(std::string nick) {
  if (std::find(this->operators.begin(), this->operators.end(), nick) ==
      this->operators.end()) {
    this->operators.push_back(nick);
  }
  if (!this->HasVoice(nick)) {
    this->hasvoice.push_back(nick);
  }
  return;
}
int Channel::IsOperator(std::string nick) {
  if (std::find(this->operators.begin(), this->operators.end(), nick) !=
      this->operators.end()) {
    return 1;
  }
  return 0;
}
void Channel::RemoveOperator(std::string nick) {
  std::string walker;
  for (auto it = this->operators.begin(); it != this->operators.end(); ++it) {
    walker = *it;
    if (walker == nick) {
      this->operators.erase(it);
      if (this->HasVoice(nick)) {
        this->RemoveVoice(nick);
      }
      return;
    }
  }
  return;
}
int Channel::IsMember(std::string nick) {
  Client *walker;
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    if (walker->GetNick() == nick) {
      return 1;
    }
  }
  return 0;
}
int Channel::RemoveNick(std::string nick) {
  Client *walker;
  std::string stalker;
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    if (walker->GetNick() == nick) {
      this->members.erase(it);
      break;
    }
  }
  if (this->HasVoice(nick)) {
    this->RemoveVoice(nick);
  }
  if (this->IsOperator(nick)) {
    this->RemoveOperator(nick);
  }
  if (this->IsInvited(nick)) {
    this->RemoveInvited(nick);
  }
  return 0;
}
int Channel::SendAllClients(std::string message) {
  Client *walker;
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    walker->AddLineToResponseQueue(message);
  }
  return SUCCESS;
}
int Channel::SendAllOperators(std::string message) {
  Client *walker;
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    if (this->IsOperator(walker->GetNick())) {
      walker->AddLineToResponseQueue(message);
    }
  }
  return SUCCESS;
}
int Channel::SendChannelNotice(Client *c, std::string message,
                               std::string serverhost) {
  Client *walker;
  std::string response;
  if (this->GetOnlyMembers() && !this->IsMember(c->GetNick())) {
    return FAIL;
  }
  if (this->GetIsModerated() && !this->HasVoice(c->GetNick())) {
    return FAIL;
  }
  std::string outmessage = ":" + c->GetNick() + "!~" + c->GetUser() + "@" +
                           c->GetHostname() + " NOTICE " +
                           this->GetChannelName() + " :" + message + "\r\n";
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    if (walker == c) {
      continue;
    }
    walker->AddLineToResponseQueue(outmessage);
  }
  return SUCCESS;
}
int Channel::SendChannelMessage(Client *c, std::string message,
                                std::string serverhost) {
  Client *walker;
  std::string response;
  if (this->GetOnlyMembers() && !this->IsMember(c->GetNick())) {
    response = ":irc." + serverhost + " 404 " + c->GetNick() + " " +
               this->GetChannelName() + " :Cannot send to channel\r\n";
    c->AddLineToResponseQueue(response);
    return FAIL;
  }
  if (this->GetIsModerated() && !this->HasVoice(c->GetNick())) {
    response = ":irc." + serverhost + " 404 " + c->GetNick() + " " +
               this->GetChannelName() + " :Cannot send to channel\r\n";
    c->AddLineToResponseQueue(response);
    return FAIL;
  }
  std::string outmessage = ":" + c->GetNick() + "!~" + c->GetUser() + "@" +
                           c->GetHostname() + " PRIVMSG " +
                           this->GetChannelName() + " :" + message + "\r\n";
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    if (walker == c) {
      continue;
    }
    walker->AddLineToResponseQueue(outmessage);
  }
  return SUCCESS;
}
int Channel::RemoveNickQuit(std::string nick, std::string msg) {
  Client *walker = NULL;
  Client *wt = NULL;
  if (!this->IsMember(nick)) {
    return FAIL;
  }
  if (this->HasVoice(nick)) {
    this->RemoveVoice(nick);
  }
  if (this->IsInvited(nick)) {
    this->RemoveInvited(nick);
  }
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    if (walker->GetNick() == nick) {
      this->members.erase(it);
      wt = *it;
      break;
    }
  }
  if (!wt) {
    return FAIL;
  }
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    walker->AddLineToResponseQueue(":" + wt->GetNick() + "!~" + wt->GetUser() +
                                   "@" + wt->GetHostname() + " QUIT :\"" + msg +
                                   "\"\r\n");
  }
  return SUCCESS;
}
int Channel::RemoveNickPart(std::string nick, std::string msg) {
  Client *walker = NULL;
  Client *wt = NULL;
  std::string response;
  if (!this->IsMember(nick)) {
    return FAIL;
  }
  if (this->HasVoice(nick)) {
    this->RemoveVoice(nick);
  }
  if (this->IsInvited(nick)) {
    this->RemoveInvited(nick);
  }
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    if (walker->GetNick() == nick) {
      this->members.erase(it);
      wt = *it;
      break;
    }
  }
  if (!wt) {
    return FAIL;
  }
  response = ":" + wt->GetNick() + "!~" + wt->GetUser() + "@" +
             wt->GetHostname() + " PART " + this->GetChannelName();
  response += " :";
  if (msg != "") {
    response += msg;
  }
  response += "\r\n";
  wt->AddLineToResponseQueue(response);
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    walker->AddLineToResponseQueue(response);
  }
  return SUCCESS;
}
int Channel::MakeWhoString(Client *c, std::string serverhost) {
  std::string whostring = "";
  std::string base = "";
  Client *walker = NULL;
  std::string modes = "";
  base = ":" + serverhost + " 352 " + c->GetNick() + " " +
         this->GetChannelName() + " ";
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    if (this->HasVoice(walker->GetNick())) {
      modes = "+";
    }
    if (this->IsOperator(walker->GetNick())) {
      modes = "@";
    }
    whostring = base;
    whostring += "~" + walker->GetUser() + " " + walker->GetHostname() + " " +
                 serverhost + walker->GetNick() + " H" + modes + " :0 " +
                 walker->GetUser() + "\r\n";
    c->AddLineToResponseQueue(whostring);
  }
  c->AddLineToResponseQueue(base = ":" + serverhost + " 315 " + c->GetNick() +
                                   " " + this->GetChannelName() +
                                   " :End of /WHO list.\r\n");
  return SUCCESS;
}