#include "client.hpp"
#include "server.hpp"
#include "utils.hpp"
#include <stdarg.h>
#include <stdio.h>
int Server::HandleAWAY() {
  char response[256];
  std::string msg = "";
  if (this->tokens.size() == 1) {
    makeresponse(response,
                 ":irc.%s 305 %s :You are no longer marked as being away\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->SetAway(0);
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  for (auto it = this->tokens.begin() + 1; it != this->tokens.end(); ++it) {
    msg += *it + " ";
  }
  rtrim(msg);
  if (msg[0] == ':') {
    msg.erase(0, 1);
  }
  if (msg.size() > MAXAWAYMSGLEN) {
    msg.erase(msg.begin() + MAXAWAYMSGLEN, msg.end());
  }
  this->c->SetAway(1);
  this->c->SetAwayMsg(msg);
  makeresponse(response,
               ":irc.%s 306 %s :You have been marked as being away\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str());
  this->c->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::HandleCAP() {
  char response[224];
  std::string subcommand;
  std::string cap;
  std::string n = "*";
  if (this->c->IsNickSet()) {
    n = this->c->GetNick();
  }
  if (this->tokens.size() < 2) {
    makeresponse(response, ":irc.%s 461 %s CAP :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), n.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  subcommand = this->tokens[1];
  if (subcommand.size() > MAXCAPLEN) {
    subcommand.erase(subcommand.begin() + MAXCAPLEN, subcommand.end());
  }
  if (subcommand == "LS") {
    makeresponse(response, ":irc.%s CAP %s LS :away-notify multi-prefix\r\n",
                 this->GetServerHostname().c_str(), n.c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  } else if (subcommand == "REQ") {
    if (this->tokens.size() < 3) {
      makeresponse(response, ":irc.%s 461 %s CAP :Not enough parameters\r\n",
                   this->GetServerHostname().c_str(), n.c_str());
      this->c->AddLineToResponseQueue(response);
      return FAIL;
    }
    cap = this->tokens[2];
    if (cap[0] == ':') {
      cap.erase(0, 1);
    }
    if (cap.size() > MAXCAPLEN) {
      cap.erase(cap.begin() + MAXCAPLEN, cap.end());
    }
    if (cap == "multi-prefix") {
      this->c->SetCapFlag(CAP_MULTI);
    } else if (cap == "away-notify") {
      this->c->SetCapFlag(CAP_AWAYN);
    } else {
      makeresponse(response, ":irc.%s CAP %s NAK :%s\r\n",
                   this->GetServerHostname().c_str(), n.c_str(), cap.c_str());
      this->c->AddLineToResponseQueue(response);
      return FAIL;
    }
    makeresponse(response, ":irc.%s CAP * ACK :%s\r\n",
                 this->GetServerHostname().c_str(), cap.c_str());
    this->c->AddLineToResponseQueue(response);
  } else if (subcommand == "END") {
    return SUCCESS;
  } else {
    makeresponse(response, ":irc.%s 410 %s %s :Invalid CAP subcommand\r\n",
                 this->GetServerHostname().c_str(), n.c_str(),
                 subcommand.c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  return SUCCESS;
}
int Server::HandleINVITE() {
  Client *ti;
  Channel *ch;
  std::string nick;
  std::string channel;
  char response[300];
  if (this->tokens.size() < 3) {
    makeresponse(response, ":irc.%s 461 %s INVITE :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  nick = this->tokens[1];
  if (nick.size() > MAXNICKLEN) {
    nick.erase(nick.begin() + MAXNICKLEN, nick.end());
  }
  channel = this->tokens[2];
  if (channel.size() > CHANNELLEN) {
    channel.erase(channel.begin() + CHANNELLEN, channel.end());
  }
  ti = this->GetClientByNick(nick);
  if (ti == NULL) {
    makeresponse(response, ":irc.%s 401 %s %s :No such nick/channel\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 nick.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  ch = this->GetChannelByName(channel);
  if (ch == NULL) {
    makeresponse(response, ":irc.%s 403 %s %s :No such channel\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 nick.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  ch->AddInvited(nick);
  ti->AddInviteTo(channel);
  makeresponse(response, ":irc.%s 341 %s %s %s\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
               nick.c_str(), channel.c_str());
  this->c->AddLineToResponseQueue(response);
  makeresponse(response, ":%s!~%s@%s INVITE %s :%s\r\n",
               this->c->GetNick().c_str(), this->c->GetUser().c_str(),
               this->c->GetHostname().c_str(), ti->GetNick().c_str(),
               channel.c_str());
  ti->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::HandleISON() {
  char response[800];
  std::string nick;
  Client *nc;
  if (this->tokens.size() == 1) {
    makeresponse(response, ":irc.%s 461 %s ISON :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  makeresponse(response, ":irc.%s 303 %s :", this->GetServerHostname().c_str(),
               this->c->GetNick().c_str());
  for (auto it = this->tokens.begin() + 1; it != this->tokens.end(); ++it) {
    nick = *it;
    nc = this->GetClientByNick(nick);
    if (nc) {
      strcat(response, (nick + " ").c_str());
    }
  }
  strcat(response, "\r\n");
  this->c->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::HandleJOIN() {
  Channel *nc = NULL;
  std::string name;
  char response[300];
  std::vector<std::string> rooms;
  std::vector<std::string> keys;
  std::string tk;
  std::vector<std::string> nameroom;
  int keyindex = 0;
  int count = 0;
  if (this->tokens.size() < 2) {
    makeresponse(response, ":irc.%s 461 * JOIN :Not enough parameters\r\n",
                 this->GetServerHostname().c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  for (auto it = this->channels.begin(); it != this->channels.end(); ++it) {
    if ((*it)->IsMember(this->c->GetNick())) {
      count++;
    }
  }
  if (count >= 15) {
    makeresponse(response,
                 ":irc.%s 472 %s %s :Cannot join channel - limit hit\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 name.c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  rooms = tokenize_line(this->tokens[1], ',');
  if (this->tokens.size() == 3) {
    keys = tokenize_line(this->tokens[2], ',');
  } else {
    keys.clear();
  }
  for (auto it = rooms.begin(); it != rooms.end(); ++it) {
    name = *it;
    if (name.size() > CHANNELLEN) {
      name.erase(name.begin() + CHANNELLEN, name.end());
    }
    if (name[0] != '#' && name[0] != '&') {
      makeresponse(response, ":irc.%s 403 %s %s :No such channel\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), name.c_str());
      this->c->AddLineToResponseQueue(response);
      continue;
    }
    nc = this->GetChannelByName(name);
    if (nc == NULL) {
      nc = new Channel(name);
      if (!nc) {
        std::cout << "[ERROR] allocation fail" << std::endl;
        return FAIL;
      }
      nc->SetOperTopic(1);
      nc->SetOnlyMembers(1);
      this->channels.push_back(nc);
      nc->AddMember(this->c, this->GetServerHostname());
      nameroom.clear();
      nameroom.push_back("NAMES");
      nameroom.push_back(name);
      this->tokens = nameroom;
      this->HandleNAMES();
    } else {
      if (nc->GetHasMax()) {
        if (nc->GetMaxUsers() <= nc->GetUserCount()) {
          makeresponse(response, ":irc.%s 471 %s %s :Cannot join channel (+l) "
                                 "- channel is full, try again later\r\n",
                       this->GetServerHostname().c_str(),
                       this->c->GetNick().c_str(), name.c_str());
          this->c->AddLineToResponseQueue(response);
          continue;
        }
      } else if (nc->GetUserCount() > MAXINCHAN) {
        makeresponse(response, ":irc.%s 471 %s %s :Cannot join channel (+l) - "
                               "channel is full, try again later\r\n",
                     this->GetServerHostname().c_str(),
                     this->c->GetNick().c_str(), name.c_str());
        this->c->AddLineToResponseQueue(response);
        continue;
      }
      if (nc->IsMember(c->GetNick())) {
        continue;
      }
      if (nc->GetHasKey()) {
        if (keyindex == keys.size()) {
          makeresponse(
              response,
              ":irc.%s 475 %s %s :Cannot join channel (+k) - bad key\r\n",
              this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
              name.c_str());
          this->c->AddLineToResponseQueue(response);
          continue;
        }
        tk = keys[keyindex];
        keyindex++;
        if (!nc->IsCorrectKey(tk)) {
          makeresponse(
              response,
              ":irc.%s 475 %s %s :Cannot join channel (+k) - bad key\r\n",
              this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
              name.c_str());
          this->c->AddLineToResponseQueue(response);
          continue;
        }
      }
      if (nc->GetIsInviteOnly() && !nc->IsInvited(this->c->GetNick())) {
        makeresponse(response, ":irc.%s 473 %s %s :Cannot join channel (+i) - "
                               "you must be invited\r\n",
                     this->GetServerHostname().c_str(),
                     this->c->GetNick().c_str(), name.c_str());
        this->c->AddLineToResponseQueue(response);
        continue;
      }
      if (nc->IsBanned(c->GetNick())) {
        makeresponse(response,
                     ":irc.%s 474 %s %s :Cannot join channel (+b)\r\n",
                     this->GetServerHostname().c_str(),
                     this->c->GetNick().c_str(), name.c_str());
        this->c->AddLineToResponseQueue(response);
        continue;
      }
      nc->AddMember(this->c, this->GetServerHostname());
      nameroom.clear();
      nameroom.push_back("NAMES");
      nameroom.push_back(name);
      this->tokens = nameroom;
      this->HandleNAMES();
    }
  }
  return SUCCESS;
}
int Server::HandleKICK() {
  std::string nick;
  std::string room;
  std::string message = "";
  char response[360];
  Client *target_client;
  Channel *target_channel;
  if (this->tokens.size() < 3) {
    makeresponse(response, ":irc.%s 461 %s KICK :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  room = this->tokens[1];
  if (room.size() > CHANNELLEN) {
    room.erase(room.begin() + CHANNELLEN, room.end());
  }
  nick = this->tokens[2];
  if (nick.size() > MAXNICKLEN) {
    nick.erase(nick.begin() + MAXNICKLEN, nick.end());
  }
  target_channel = this->GetChannelByName(room);
  if (target_channel == NULL) {
    makeresponse(response, ":irc.%s 403 %s %s :No such channel\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 room.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  target_client = this->GetClientByNick(nick);
  if (target_client == NULL) {
    makeresponse(response, ":irc.%s 401 %s %s :No such nick/channel\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 nick.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  if (!target_channel->IsOperator(this->c->GetNick())) {
    makeresponse(response,
                 ":irc.%s 482 %s %s :You're not a channel operator\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 room.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  if (this->tokens.size() > 3) {
    for (auto it = this->tokens.begin() + 3; it != this->tokens.end(); ++it) {
      message += *it;
    }
    if (message[0] == ':') {
      message.erase(0, 1);
    }
  } else {
    message = nick;
  }
  if (message.size() > MAXAWAYMSGLEN) {
    message.erase(message.begin() + MAXAWAYMSGLEN, message.end());
  }
  makeresponse(response, ":%s!~%s@%s KICK %s %s :%s\r\n",
               this->c->GetNick().c_str(), this->c->GetUser().c_str(),
               this->c->GetHostname().c_str(), room.c_str(), nick.c_str(),
               message.c_str());
  target_channel->SendAllClients(response);
  target_channel->RemoveNick(nick);
  return SUCCESS;
}
int Server::HandleKNOCK() {
  char response[512];
  std::string msg;
  std::string room;
  time_t nowtime;
  Channel *ch;
  if (this->tokens.size() == 1) {
    makeresponse(response, ":irc.%s 461 %s KNOCK :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  room = this->tokens[1];
  if (room.size() > CHANNELLEN) {
    room.erase(room.begin() + CHANNELLEN, room.end());
  }
  nowtime = std::time(nullptr);
  if (nowtime - c->GetLastKnockTime() < 60) {
    makeresponse(response, ":irc.%s 712 %s %s :Too many KNOCKs (user).\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 room.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  ch = this->GetChannelByName(room);
  if (!ch) {
    makeresponse(response, ":irc.%s 403 %s %s :No such channel\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 room.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  if (!ch->GetIsInviteOnly()) {
    makeresponse(response, ":irc.%s 713 %s %s :Channel is open.\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 room.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  if (ch->GetIsPrivate()) {
    makeresponse(response, ":irc.%s 404 %s %s :Cannot send to channel\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 room.c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  if (this->tokens.size() == 2) {
    msg = "has asked for an invite.";
  } else {
    for (auto it = this->tokens.begin() + 2; it != this->tokens.end(); ++it) {
      msg += *it + " ";
    }
    rtrim(msg);
    if (msg[0] == ':') {
      msg.erase(0, 1);
    }
  }
  if (msg.size() > MAXAWAYMSGLEN) {
    msg.erase(msg.begin() + MAXAWAYMSGLEN, msg.end());
  }
  makeresponse(response, ":irc.%s 710 %s %s %s!~%s@%s :%s\r\n",
               this->GetServerHostname().c_str(), room.c_str(), room.c_str(),
               this->c->GetNick().c_str(), this->c->GetUser().c_str(),
               c->GetHostname().c_str(), msg.c_str());
  ch->SendAllOperators(response);
  makeresponse(response,
               ":irc.%s 711 %s %s :Your KNOCK has been delivered.\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
               room.c_str());
  this->c->AddLineToResponseQueue(response);
  this->c->SetLastKnockTime(time(nullptr));
  return SUCCESS;
}
int Server::HandleLIST() {
  char response[360];
  Channel *ch;
  std::vector<std::string> rooms;
  makeresponse(response, ":irc.%s 321 %s Channel :Users  Name\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str());
  this->c->AddLineToResponseQueue(response);
  if (tokens.size() == 1) {
    for (auto it = this->channels.begin(); it != this->channels.end(); ++it) {
      ch = *it;
      if (ch->GetIsSecret() && !ch->IsMember(this->c->GetNick())) {
        continue;
      }
      makeresponse(response, ":irc.%s 322 %s ",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str());
      if (ch->GetIsPrivate() && !ch->IsMember(this->c->GetNick())) {
        strcat(response, "Prv ");
      } else {
        strcat(response, (ch->GetChannelName() + " ").c_str());
      }
      strcat(response, std::to_string(ch->GetMemberCount()).c_str());
      if (ch->GetIsPrivate() && !ch->IsMember(this->c->GetNick())) {
        strcat(response, " :");
      } else {
        strcat(response, (" :" + ch->GetTopic()).c_str());
      }
      strcat(response, "\r\n");
      this->c->AddLineToResponseQueue(response);
    }
  } else {
    rooms = tokenize_line(this->tokens[1], ',');
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
      ch = this->GetChannelByName(*it);
      if (!ch) {
        continue;
      }
      if (ch->GetIsSecret() && !ch->IsMember(this->c->GetNick())) {
        continue;
      }
      makeresponse(response, ":irc.%s 322 %s ",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str());
      if (ch->GetIsPrivate() && !ch->IsMember(this->c->GetNick())) {
        strcat(response, "Prv ");
      } else {
        strcat(response, (ch->GetChannelName() + " ").c_str());
      }
      strcat(response, std::to_string(ch->GetMemberCount()).c_str());
      if (ch->GetIsPrivate() && !ch->IsMember(c->GetNick())) {
        strcat(response, " :");
      } else {
        strcat(response, (" :" + ch->GetTopic()).c_str());
      }
      strcat(response, "\r\n");
      this->c->AddLineToResponseQueue(response);
    }
  }
  makeresponse(response, ":irc.%s 323 %s :End of /LIST*/\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str());
  this->c->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::HandleMODE() {
  char response[300];
  std::string target;
  std::string tnick;
  std::string mode;
  std::string sign;
  std::string be;
  char new_mode;
  int flag = 0;
  int newmax = 0;
  Channel *ch = NULL;
  if (this->tokens.size() < 2) {
    makeresponse(response, ":irc.%s 461 %s MODE :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  target = this->tokens[1];
  if (target.size() > CHANNELLEN) {
    target.erase(target.begin() + CHANNELLEN, target.end());
  }
  if (target[0] == '#' || target[0] == '&') {
    ch = this->GetChannelByName(target);
    if (ch == NULL) {
      makeresponse(response, ":irc.%s 403 %s %s :No such channel\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), target.c_str());
      this->c->AddLineToResponseQueue(response);
      return FAIL;
    }
    if (this->tokens.size() == 2) {
      makeresponse(response, ":irc.%s 324 %s %s %s\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), target.c_str(),
                   ch->GetModeString().c_str());
      this->c->AddLineToResponseQueue(response);
      return SUCCESS;
    }
    mode = this->tokens[2];
    if (mode == "b") {
      ch->PrintBanList(this->c, this->GetServerHostname());
      return SUCCESS;
    }
    if (!ch->IsOperator(this->c->GetNick())) {
      makeresponse(response,
                   ":irc.%s 482 %s %s :You're not a channel operator\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), target.c_str());
      this->c->AddLineToResponseQueue(response);
      return FAIL;
    }
    if (mode[0] != '+' && mode[0] != '-') {
      makeresponse(response,
                   ":irc.%s 472 %s %c :is an unknown mode char to me\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), mode[0]);
      this->c->AddLineToResponseQueue(response);
      return FAIL;
    }
    sign = mode[0];
    if (sign == "+") {
      flag = 1;
    } else {
      flag = 0;
    }
    mode.erase(0, 1);
    while (mode.size() > 0) {
      new_mode = mode[0];
      switch (new_mode) {
      case 'p':
        if (ch->GetIsPrivate() == flag) {
          return SUCCESS;
        }
        ch->SetIsPrivate(flag);
        makeresponse(response, ":%s!~%s@%s MODE %s %c%c\r\n",
                     this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                     this->c->GetHostname().c_str(), target.c_str(), sign[0],
                     new_mode);
        ch->SendAllClients(response);
        return SUCCESS;
        break;
      case 's':
        if (ch->GetIsSecret() == flag) {
          return SUCCESS;
        }
        ch->SetIsSecret(flag);
        makeresponse(response, ":%s!~%s@%s MODE %s %c%c\r\n",
                     this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                     this->c->GetHostname().c_str(), target.c_str(), sign[0],
                     new_mode);
        ch->SendAllClients(response);
        return SUCCESS;
        break;
      case 'i':
        if (ch->GetIsInviteOnly() == flag) {
          return SUCCESS;
        }
        ch->SetIsInviteOnly(flag);
        makeresponse(response, ":%s!~%s@%s MODE %s %c%c\r\n",
                     this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                     this->c->GetHostname().c_str(), target.c_str(), sign[0],
                     new_mode);
        ch->SendAllClients(response);
        return SUCCESS;
        break;
      case 't':
        if (ch->GetOperTopic() == flag) {
          return SUCCESS;
        }
        ch->SetOperTopic(flag);
        makeresponse(response, ":%s!~%s@%s MODE %s %c%c\r\n",
                     this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                     this->c->GetHostname().c_str(), target.c_str(), sign[0],
                     new_mode);
        ch->SendAllClients(response);
        return SUCCESS;
        break;
      case 'n':
        if (ch->GetOnlyMembers() == flag) {
          return SUCCESS;
        }
        ch->SetOnlyMembers(flag);
        makeresponse(response, ":%s!~%s@%s MODE %s %c%c\r\n",
                     this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                     this->c->GetHostname().c_str(), target.c_str(), sign[0],
                     new_mode);
        ch->SendAllClients(response);
        return SUCCESS;
        break;
      case 'm':
        if (ch->GetIsModerated() == flag) {
          return SUCCESS;
        }
        ch->SetIsModerated(flag);
        makeresponse(response, ":%s!~%s@%s MODE %s %c%c\r\n",
                     this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                     this->c->GetHostname().c_str(), target.c_str(), sign[0],
                     new_mode);
        ch->SendAllClients(response);
        return SUCCESS;
        break;
      case 'v':
        if (this->tokens.size() != 4) {
          return FAIL;
        }
        tnick = this->tokens[3];
        if (tnick.size() > MAXNICKLEN) {
          tnick.erase(tnick.begin() + MAXNICKLEN, tnick.end());
        }
        if (flag) {
          ch->AddVoice(tnick);
        } else {
          ch->RemoveVoice(tnick);
        }
        makeresponse(response, ":%s!~%s@%s MODE %s %c%c %s\r\n",
                     this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                     this->c->GetHostname().c_str(), target.c_str(), sign[0],
                     new_mode, tnick.c_str());
        ch->SendAllClients(response);
        return SUCCESS;
      case 'o':
        if (this->tokens.size() != 4) {
          return FAIL;
        }
        tnick = this->tokens[3];
        if (tnick.size() > MAXNICKLEN) {
          tnick.erase(tnick.begin() + MAXNICKLEN, tnick.end());
        }
        if (flag) {
          ch->AddOperator(tnick);
        } else {
          ch->RemoveOperator(tnick);
        }
        makeresponse(response, ":%s!~%s@%s MODE %s %c%c %s\r\n",
                     this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                     this->c->GetHostname().c_str(), target.c_str(), sign[0],
                     new_mode, tnick.c_str());
        ch->SendAllClients(response);
        return SUCCESS;
      case 'l':
        if (flag && tokens.size() != 4) {
          return SUCCESS;
        }
        ch->SetHasMax(flag);
        if (!flag) {
          makeresponse(response, ":%s!~%s@%s MODE %s %c%c\r\n",
                       this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                       this->c->GetHostname().c_str(), target.c_str(), sign[0],
                       new_mode);
          ch->SendAllClients(response);
        } else {
          try {
            newmax = std::stoi(this->tokens[3]);
          } catch (const std::exception &e) {
            return FAIL;
          }
          if (newmax > MAXINCHAN) {
            newmax = MAXINCHAN;
          }
          ch->SetMaxUsers(newmax);
          makeresponse(response, ":%s!~%s@%s MODE %s %c%c %d\r\n",
                       this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                       this->c->GetHostname().c_str(), target.c_str(), sign[0],
                       new_mode, newmax);
          ch->SendAllClients(response);
        }
        return SUCCESS;
        break;
      case 'b':
        if (this->tokens.size() < 4) {
          ch->PrintBanList(this->c, this->GetServerHostname());
          return SUCCESS;
        }
        be = "";
        for (auto it = this->tokens.begin() + 3; it != this->tokens.end();
             ++it) {
          be += *it + " ";
        }
        rtrim(be);
        ch->AddBanExp(this->c, be);
        return SUCCESS;
        break;
      case 'k':
        if (flag && this->tokens.size() != 4) {
          return SUCCESS;
        }
        ch->SetHasKey(flag);
        if (!flag) {
          makeresponse(response, ":%s!~%s@%s MODE %s %c%c *\r\n",
                       this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                       this->c->GetHostname().c_str(), target.c_str(), sign[0],
                       new_mode);
          ch->SendAllClients(response);
        } else {
          tnick = this->tokens[3];
          if (tnick.size() > MAXNICKLEN) {
            tnick.erase(tnick.begin() + MAXNICKLEN, tnick.end());
          }
          ch->SetChannelKey(tnick);
          makeresponse(response, ":%s!~%s@%s MODE %s %c%c %s\r\n",
                       this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                       this->c->GetHostname().c_str(), target.c_str(), sign[0],
                       new_mode, tnick.c_str());
          ch->SendAllClients(response);
        }
        return SUCCESS;
        break;
      default:
        makeresponse(response, ":irc.%s 501 %s :Unknown MODE flag\r\n",
                     this->GetServerHostname().c_str(),
                     this->c->GetNick().c_str());
        this->c->AddLineToResponseQueue(response);
        return FAIL;
      };
    }
  }
  return SUCCESS;
}
int Server::HandleMOTD() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream lf;
  char response[300];
  makeresponse(response, ":irc.%s 375 %s :- irc.%s Message of the Day -\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
               this->GetServerHostname().c_str());
  this->c->AddLineToResponseQueue(response);
  lf << std::put_time(&tm, "%d/%m/%Y %H:%M");
  makeresponse(response, ":irc.%s 372 %s :- %s\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
               lf.str().c_str());
  this->c->AddLineToResponseQueue(response);
  makeresponse(response,
               ":irc.%s 372 %s :-                          [ CHESS IRC ]\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str());
  this->c->AddLineToResponseQueue(response);
  makeresponse(response, ":irc.%s 372 %s :- "
                         "|----------------------------------------------------"
                         "--------------------|\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str());
  this->c->AddLineToResponseQueue(response);
  makeresponse(response, ":irc.%s 376 %s :End of MOTD command.\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str());
  this->c->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::HandleNAMES() {
  struct locals {
    Channel *ch = NULL;
    Client *cl = NULL;
    std::string goodroom;
    std::string no_chan_members;
    int has_channel = 0;
    std::vector<std::string> rooms;
    char response[800];
  } l;
  memset(l.response, 0, 800);
  if (this->tokens.size() == 1) {
    for (auto it = this->channels.begin(); it != this->channels.end(); ++it) {
      l.ch = *it;
      if ((l.ch->GetIsPrivate() || l.ch->GetIsSecret()) &&
          !l.ch->IsMember(this->c->GetNick())) {
        continue;
      }
      makeresponse(l.response, ":irc.%s 353 %s = %s :%s %s\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), l.ch->GetChannelName().c_str(),
                   l.ch->GetMemberList().c_str(),
                   l.ch->GetModeString().c_str());
      this->c->AddLineToResponseQueue(l.response);
    }
    l.no_chan_members = "";
    for (auto it = this->clients.begin(); it != this->clients.end(); ++it) {
      l.cl = *it;
      l.has_channel = 0;
      for (auto ch_it = this->channels.begin(); ch_it != this->channels.end();
           ++ch_it) {
        l.ch = *ch_it;
        if (l.ch->IsMember(l.cl->GetNick()) &&
            (l.ch->GetIsPrivate() || l.ch->GetIsSecret()) &&
            !l.ch->IsMember(this->c->GetNick())) {
          continue;
        }
        if (l.ch->IsMember(this->c->GetNick())) {
          l.has_channel = 1;
          break;
        }
      }
      if (!l.has_channel) {
        l.no_chan_members += l.cl->GetNick() + " ";
      }
    }
    rtrim(l.no_chan_members);
    if (l.no_chan_members != "") {
      l.no_chan_members += "\r\n";
      makeresponse(l.response, ":irc.%s 353 %s = * :%s\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), l.no_chan_members.c_str());
      this->c->AddLineToResponseQueue(l.response);
    }
    makeresponse(l.response, ":irc.%s 366 %s * :End of /NAMES list.\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(l.response);
  } else {
    l.rooms = tokenize_line(this->tokens[1], ',');
    l.goodroom = "*";
    for (auto it = l.rooms.begin(); it != l.rooms.end(); ++it) {
      l.ch = this->GetChannelByName(*it);
      if (l.ch == NULL) {
        continue;
      }
      if ((l.ch->GetIsPrivate() || l.ch->GetIsSecret()) &&
          !l.ch->IsMember(this->c->GetNick())) {
        continue;
      }
      makeresponse(l.response, ":irc.%s 353 %s = %s :%s %s\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), l.ch->GetChannelName().c_str(),
                   l.ch->GetMemberList().c_str(),
                   l.ch->GetModeString().c_str());
      this->c->AddLineToResponseQueue(l.response);
      l.goodroom = *it;
    }
    makeresponse(l.response, ":irc.%s 366 %s %s :End of /NAMES list.\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 l.goodroom.c_str());
    this->c->AddLineToResponseQueue(l.response);
  }
  return SUCCESS;
}
int Server::HandleNICK() {
  char response[256];
  std::string nick;
  std::string oldnick;
  Channel *ch = NULL;
  Client *cl = NULL;
  std::vector<Client *> to_notify;
  if (this->tokens.size() <= 1) {
    makeresponse(response, ":irc.%s 461 * NICK :Not enough parameters\r\n",
                 this->GetServerHostname().c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  nick = this->tokens[1];
  rtrim(nick);
  if (nick.size() > MAXNICKLEN) {
    nick.erase(nick.begin() + MAXNICKLEN, nick.end());
  }
  if (this->NickExists(nick)) {
    makeresponse(response, ":irc.%s 433 * %s :Nickname is already in use.\r\n",
                 this->GetServerHostname().c_str(), nick.c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  oldnick = this->c->GetNick();
  this->c->SetNick(nick);
  if (this->c->NickUserSet() && !this->c->GetRegistered()) {
    this->SendServerInfo(this->c);
    this->HandleMOTD();
    this->c->SetRegistered(1);
  } else if (this->c->GetRegistered()) {
    to_notify.clear();
    for (auto it = this->clients.begin(); it != this->clients.end(); ++it) {
      cl = *it;
      for (auto cit = this->channels.begin(); cit != this->channels.end();
           ++cit) {
        ch = *cit;
        if (ch->IsMember(cl->GetNick()) && ch->IsMember(this->c->GetNick())) {
          if (std::find(to_notify.begin(), to_notify.end(), cl) ==
              to_notify.end()) {
            to_notify.push_back(cl);
            makeresponse(response, ":%s!~%s@%s NICK :%s\r\n", oldnick.c_str(),
                         this->c->GetUser().c_str(),
                         this->c->GetHostname().c_str(),
                         this->c->GetNick().c_str());
            cl->AddLineToResponseQueue(response);
          }
        }
      }
    }
  }
  to_notify.clear();
  return SUCCESS;
}
int Server::HandleNOTICE() {
  Channel *ch = NULL;
  Client *cl = NULL;
  std::string name;
  char response[332];
  std::vector<std::string> rooms;
  std::string msg;
  std::string bit;
  if (this->tokens.size() < 3) {
    makeresponse(response, ":irc.%s 461 * NOTICE :Not enough parameters\r\n",
                 this->GetServerHostname().c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  rooms = tokenize_line(this->tokens[1], ',');
  for (auto it = rooms.begin(); it != rooms.end(); ++it) {
    name = *it;
    if (name.size() > CHANNELLEN) {
      name.erase(name.begin() + CHANNELLEN, name.end());
    }
    if (name[0] != '#' && name[0] != '&') {
      cl = this->GetClientByNick(name);
      if (cl == NULL) {
        return SUCCESS;
      }
      if (this->tokens[2][0] == ':') {
        this->tokens[2].erase(0, 1);
      }
      msg = "";
      for (auto msg_it = this->tokens.begin() + 2; msg_it != this->tokens.end();
           ++msg_it) {
        bit = *msg_it;
        msg += bit + " ";
      }
      rtrim(msg);
      if (msg.size() > MAXAWAYMSGLEN) {
        msg.erase(msg.begin() + MAXAWAYMSGLEN, msg.end());
      }
      makeresponse(response, ":%s!~%s@%s NOTICE %s :%s\r\n",
                   this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                   this->c->GetHostname().c_str(), name.c_str(), msg.c_str());
      cl->AddLineToResponseQueue(response);
    } else {
      ch = this->GetChannelByName(name);
      if (ch == NULL) {
        return SUCCESS;
      }
      if (this->tokens[2][0] == ':') {
        this->tokens[2].erase(0, 1);
      }
      msg = "";
      for (auto msg_it = tokens.begin() + 2; msg_it != this->tokens.end();
           ++msg_it) {
        bit = *msg_it;
        msg += bit + " ";
      }
      rtrim(msg);
      if (msg.size() > MAXAWAYMSGLEN) {
        msg.erase(msg.begin() + MAXAWAYMSGLEN, msg.end());
      }
      ch->SendChannelNotice(this->c, msg, this->GetServerHostname());
    }
  }
  return SUCCESS;
}
int Server::HandlePART() {
  char response[258];
  std::string message = "";
  std::string name;
  Channel *nc = NULL;
  std::vector<std::string> rooms;
  if (this->tokens.size() < 2) {
    makeresponse(response, ":irc.%s 461 %s PART :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
  }
  if (this->tokens.size() > 2) {
    for (auto it = this->tokens.begin() + 2; it != this->tokens.end(); ++it) {
      message += *it + " ";
    }
    rtrim(message);
    if (message[0] == ':') {
      message.erase(0, 1);
    }
  }
  rooms = tokenize_line(this->tokens[1], ',');
  for (auto it = rooms.begin(); it != rooms.end(); ++it) {
    name = *it;
    if (name.size() > CHANNELLEN) {
      name.erase(name.begin() + CHANNELLEN, name.end());
    }
    if (name[0] != '#' && name[0] != '&') {
      makeresponse(response, ":irc.%s 403 %s %s :No such channel\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), name.c_str());
      this->c->AddLineToResponseQueue(response);
      continue;
    }
    nc = this->GetChannelByName(name);
    if (nc == NULL) {
      makeresponse(response, ":irc.%s 403 %s %s :No such channel\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), name.c_str());
      this->c->AddLineToResponseQueue(response);
      continue;
    } else {
      if (!nc->IsMember(this->c->GetNick())) {
        makeresponse(response,
                     ":irc.%s 442 %s %s :You're not on that channel\r\n",
                     this->GetServerHostname().c_str(),
                     this->c->GetNick().c_str(), name.c_str());
        this->c->AddLineToResponseQueue(response);
        continue;
      } else {
        if (message.size() > MAXAWAYMSGLEN) {
          message.erase(message.begin() + MAXAWAYMSGLEN, message.end());
        }
        nc->RemoveNickPart(c->GetNick(), message);
      }
    }
  }
  return SUCCESS;
}
int Server::HandlePING() {
  char response[156];
  makeresponse(response, "PONG :irc.%s\r\n", this->GetServerHostname().c_str());
  this->c->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::HandlePRIVMSG() {
  Channel *ch = NULL;
  Client *cl = NULL;
  std::string name;
  char response[530];
  std::vector<std::string> rooms;
  std::string msg;
  std::string bit;
  if (this->tokens.size() < 3) {
    makeresponse(response, ":irc.%s 461 * PRIVMSG :Not enough parameters\r\n",
                 this->GetServerHostname().c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  rooms = tokenize_line(this->tokens[1], ',');
  for (auto it = rooms.begin(); it != rooms.end(); ++it) {
    name = *it;
    if (name[0] != '#' && name[0] != '&') {
      if (name.size() > MAXNICKLEN) {
        name.erase(name.begin() + MAXNICKLEN, name.end());
      }
      cl = this->GetClientByNick(name);
      if (cl == NULL) {
        makeresponse(response, ":irc.%s 401 %s %s :No such nick/channel\r\n",
                     this->GetServerHostname().c_str(),
                     this->c->GetNick().c_str(), name.c_str());
        this->c->AddLineToResponseQueue(response);
        return SUCCESS;
      }
      if (cl->GetAway()) {
        makeresponse(response, ":irc.%s 301 %s %s :%s\r\n",
                     this->GetServerHostname().c_str(),
                     this->c->GetNick().c_str(), name.c_str(),
                     cl->GetAwayMsg().c_str());
        this->c->AddLineToResponseQueue(response);
      }
      if (this->tokens[2][0] == ':') {
        this->tokens[2].erase(0, 1);
      }
      msg = "";
      for (auto msg_it = this->tokens.begin() + 2; msg_it != this->tokens.end();
           ++msg_it) {
        bit = *msg_it;
        msg += bit + " ";
      }
      rtrim(msg);
      if (msg.size() > MAXPRIVMSGLEN) {
        msg.erase(msg.begin() + MAXPRIVMSGLEN, msg.end());
      }
      if (cl->IsSilenced(this->c->GetNick())) {
        return SUCCESS;
      }
      makeresponse(response, ":%s!~%s@%s PRIVMSG %s :%s\r\n",
                   this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                   this->c->GetHostname().c_str(), name.c_str(), msg.c_str());
      cl->AddLineToResponseQueue(response);
    } else {
      if (name.size() > CHANNELLEN) {
        name.erase(name.begin() + CHANNELLEN, name.end());
      }
      ch = this->GetChannelByName(name);
      if (ch == NULL) {
        makeresponse(response,
                     ":irc.%s 401 %s %s :No such nick/channel\r\n\r\n",
                     this->GetServerHostname().c_str(),
                     this->c->GetNick().c_str(), name.c_str());
        this->c->AddLineToResponseQueue(response);
        return SUCCESS;
      }
      if (this->tokens[2][0] == ':') {
        this->tokens[2].erase(0, 1);
      }
      msg = "";
      for (auto msg_it = this->tokens.begin() + 2; msg_it != this->tokens.end();
           ++msg_it) {
        bit = *msg_it;
        msg += bit + " ";
      }
      rtrim(msg);
      if (msg.size() > MAXPRIVMSGLEN) {
        msg.erase(msg.begin() + MAXPRIVMSGLEN, msg.end());
      }
      ch->SendChannelMessage(this->c, msg, this->GetServerHostname());
    }
  }
  return SUCCESS;
}
int Server::HandleSILENCE() {
  char response[240];
  std::string nick;
  std::string f;
  if (this->tokens.size() < 2) {
    makeresponse(response, ":irc.%s 461 * SILENCE :Not enough parameters\r\n",
                 this->GetServerHostname().c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  nick = this->tokens[1];
  if (nick.size() > MAXNICKLEN) {
    nick.erase(nick.begin() + MAXNICKLEN, nick.end());
  }
  f = nick[0];
  nick.erase(0, 1);
  if (f != "+" && f != "-") {
    makeresponse(
        response, ":irc.%s 472 %s %c :is an unknown silence char to me\r\n",
        this->GetServerHostname().c_str(), this->c->GetNick().c_str(), f[0]);
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  if (f == "+") {
    if (!this->c->IsSilenced(nick)) {
      this->c->AddSilenced(nick);
      makeresponse(response, ":%s!~%s@%s SILENCE %s :is now silenced\r\n",
                   this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                   this->c->GetHostname().c_str(), nick.c_str());
      this->c->AddLineToResponseQueue(response);
      return SUCCESS;
    }
  } else {
    if (this->c->IsSilenced(nick)) {
      this->c->RemoveSilenced(nick);
      makeresponse(response, ":%s!~%s@%s SILENCE %s :is no longer silenced\r\n",
                   this->c->GetNick().c_str(), this->c->GetUser().c_str(),
                   this->c->GetHostname().c_str(), nick.c_str());
      this->c->AddLineToResponseQueue(response);
      return SUCCESS;
    }
  }
  return SUCCESS;
}
int Server::HandleTIME() {
  std::time_t t = std::time(nullptr);
  char datetimestr[512] = {0};
  char response[512];
  std::strftime(datetimestr, sizeof(datetimestr),
                "%A %B %d %Y -- %H:%M:%S +00:00", std::localtime(&t));
  makeresponse(response, ":irc.%s 391 %s irc.%s :%s\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
               this->GetServerHostname().c_str(),
               std::string(datetimestr).c_str());
  this->c->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::HandleTOPIC() {
  char response[410];
  std::string target;
  std::string topic;
  Channel *ch = NULL;
  if (this->tokens.size() < 2) {
    makeresponse(response, ":irc.%s 461 %s TOPIC :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  target = this->tokens[1];
  if (target.size() > CHANNELLEN) {
    target.erase(target.begin() + CHANNELLEN, target.end());
  }
  ch = this->GetChannelByName(target);
  if (ch == NULL) {
    makeresponse(response, ":irc.%s 403 %s %s :No such channel\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 target.c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  if (tokens.size() == 2) {
    if (!ch->GetTopicSet()) {
      makeresponse(response, ":irc.%s 331 %s %s :No topic is set\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), target.c_str());
      this->c->AddLineToResponseQueue(response);
      return SUCCESS;
    }
    topic = ch->GetTopic();
    makeresponse(response, ":irc.%s 332 %s %s :%s\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 target.c_str(), topic.c_str());
    this->c->AddLineToResponseQueue(response);
    makeresponse(response, ":irc.%s 333 %s %s %s %s\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 target.c_str(), ch->GetTopicSetBy().c_str(),
                 std::to_string(ch->GetTopicTime()).c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  if (!ch->IsOperator(this->c->GetNick())) {
    makeresponse(response,
                 ":irc.%s 482 %s %s :You're not a channel operator\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 target.c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  topic = "";
  for (auto it = this->tokens.begin() + 2; it != this->tokens.end(); ++it) {
    topic += *it;
  }
  rtrim(topic);
  if (topic[0] == ':') {
    topic.erase(0, 1);
  }
  if (topic.size() > MAXTOPICLEN) {
    topic.erase(topic.begin() + MAXTOPICLEN, topic.end());
  }
  ch->SetTopic(topic);
  ch->SetTopicSetBy(this->c->GetNick() + "!~" + this->c->GetUser() + "@" +
                    this->c->GetHostname());
  makeresponse(response, ":%s TOPIC %s :%s\r\n", ch->GetTopicSetBy().c_str(),
               target.c_str(), topic.c_str());
  ch->SendAllClients(response);
  return SUCCESS;
}
int Server::HandleUSER() {
  char response[166];
  std::string user;
  if (this->tokens.size() < 5) {
    makeresponse(response, ":irc.%s 461 * USER :Not enough parameters\r\n",
                 this->GetServerHostname().c_str());
    this->c->AddLineToResponseQueue(response);
    return SUCCESS;
  }
  user = this->tokens[1];
  rtrim(user);
  this->c->SetUser(user);
  this->c->SetHostname(this->tokens[3]);
  if (this->c->NickUserSet() && !this->c->GetRegistered()) {
    this->SendServerInfo(this->c);
    this->HandleMOTD();
    this->c->SetRegistered(1);
  }
  return SUCCESS;
}
int Server::HandleWHO() {
  char response[592];
  std::string target;
  Channel *ch = NULL;
  Client *tc = NULL;
  int who_success = 0;
  if (this->tokens.size() < 2) {
    makeresponse(response, ":irc.%s 461 %s WHO :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  target = this->tokens[1];
  if (target[0] == '#' || target[0] == '&') {
    if (target.size() > CHANNELLEN) {
      target.erase(target.begin() + CHANNELLEN, target.end());
    }
    ch = this->GetChannelByName(target);
    if (ch == NULL) {
      makeresponse(response, ":irc.%s 461 %s WHO :Not enough parameters\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), target.c_str());
      this->c->AddLineToResponseQueue(response);
      return FAIL;
    }
    if (!ch->IsMember(this->c->GetNick())) {
      makeresponse(response, ":irc.%s 315 %s %s :End of /WHO list.\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), target.c_str());
      this->c->AddLineToResponseQueue(response);
      return FAIL;
    }
    ch->MakeWhoString(c, "irc." + this->GetServerHostname());
  } else {
    if (target.size() > MAXNICKLEN) {
      target.erase(target.begin() + MAXNICKLEN, target.end());
    }
    tc = this->GetClientByNick(target);
    if (tc == NULL) {
      makeresponse(response, ":irc.%s 315 %s %s :End of /WHO list.\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), target.c_str());
      this->c->AddLineToResponseQueue(response);
      return FAIL;
    }
    for (auto it = this->channels.begin(); it != this->channels.end(); ++it) {
      ch = *it;
      std::string md = "";
      if (ch->HasVoice(tc->GetNick())) {
        md = "+";
      }
      if (ch->IsOperator(tc->GetNick())) {
        if (tc->CapSet(CAP_MULTI)) {
          md = "@" + md;
        } else {
          md = "@";
        }
      }
      if (ch->IsMember(target) &&
          (ch->IsMember(this->c->GetNick()) || !ch->GetIsSecret())) {
        makeresponse(
            response, ":irc.%s 352 %s %s ~%s %s irc.%s %s H%s :0 %s\r\n",
            this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
            ch->GetChannelName().c_str(), tc->GetUser().c_str(),
            tc->GetHostname().c_str(), this->GetServerHostname().c_str(),
            tc->GetNick().c_str(), md.c_str(), tc->GetUser().c_str());
        who_success = 1;
        break;
      }
    }
    if (!who_success) {
      makeresponse(response, ":irc.%s 352 %s * ~%s %s irc.%s %s H :0 %s\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), tc->GetUser().c_str(),
                   tc->GetHostname().c_str(), this->GetServerHostname().c_str(),
                   tc->GetNick().c_str(), tc->GetUser().c_str());
    }
    this->c->AddLineToResponseQueue(response);
    makeresponse(response, ":irc.%s 315 %s %s :End of /WHO list.\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 target.c_str());
    this->c->AddLineToResponseQueue(response);
  }
  return SUCCESS;
}
int Server::HandleWHOIS() {
  char response[1000];
  std::string target;
  std::vector<std::string> target_tokens;
  Client *tc = NULL;
  Channel *ch = NULL;
  if (this->tokens.size() < 2) {
    makeresponse(response, ":irc.%s 461 %s WHOIS :Not enough parameters\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str());
    this->c->AddLineToResponseQueue(response);
    return FAIL;
  }
  target_tokens = tokenize_line(this->tokens[1], ',');
  for (auto it = target_tokens.begin(); it != target_tokens.end(); ++it) {
    target = *it;
    if (target.size() > MAXNICKLEN) {
      target.erase(target.begin() + MAXNICKLEN, target.end());
    }
    if (target[0] == '#' || target[0] == '&') {
      makeresponse(response, ":irc.%s 401 %s :No such nick/channel\r\n",
                   this->GetServerHostname().c_str(), target.c_str());
      this->c->AddLineToResponseQueue(response);
      continue;
    }
    tc = this->GetClientByNick(target);
    if (tc == NULL) {
      makeresponse(response, ":irc.%s 401 %s :No such nick/channel\r\n",
                   this->GetServerHostname().c_str(), target.c_str());
      this->c->AddLineToResponseQueue(response);
      continue;
    }
    makeresponse(response, ":irc.%s 311 %s %s ~%s %s * :%s\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 target.c_str(), tc->GetUser().c_str(),
                 tc->GetHostname().c_str(), tc->GetUser().c_str());
    this->c->AddLineToResponseQueue(response);
    std::string rooms = "";
    for (auto chanit = this->channels.begin(); chanit != this->channels.end();
         ++chanit) {
      ch = *chanit;
      if (ch->IsMember(tc->GetNick())) {
        if (ch->GetIsPrivate() && !ch->IsMember(this->c->GetNick())) {
          continue;
        }
        std::string pfx = "";
        if (ch->HasVoice(tc->GetNick())) {
          pfx = "+";
        }
        if (ch->IsOperator(tc->GetNick())) {
          if (c->CapSet(CAP_MULTI)) {
            pfx = "@" + pfx;
          } else {
            pfx = "@";
          }
        }
        rooms += pfx + ch->GetChannelName() + " ";
      }
    }
    if (rooms != "") {
      rtrim(rooms);
      makeresponse(response, ":irc.%s 319 %s %s :%s\r\n",
                   this->GetServerHostname().c_str(),
                   this->c->GetNick().c_str(), target.c_str(), rooms.c_str());
      this->c->AddLineToResponseQueue(response);
    }
    makeresponse(response, ":irc.%s 312 %s %s irc.%s :chess test server\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 target.c_str(), this->GetServerHostname().c_str());
    this->c->AddLineToResponseQueue(response);
    makeresponse(response,
                 ":irc.%s 317 %s %s 0 0 :seconds idle, signon time\r\n",
                 this->GetServerHostname().c_str(), this->c->GetNick().c_str(),
                 target.c_str());
    this->c->AddLineToResponseQueue(response);
  }
  makeresponse(response, ":irc.%s 318 %s :End of /WHOIS list.\r\n",
               this->GetServerHostname().c_str(), this->c->GetNick().c_str());
  this->c->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::HandleQUIT() {
  Channel *cw = NULL;
  std::string msg = "";
  if (this->tokens.size() > 1) {
    for (auto it = this->tokens.begin() + 1; it != this->tokens.end(); ++it) {
      msg += *it + " ";
    }
    rtrim(msg);
    if (msg[0] == ':') {
      msg.erase(0, 1);
    }
  }
  for (auto it = this->channels.begin(); it != this->channels.end(); ++it) {
    cw = *it;
    if (cw->IsMember(this->c->GetNick())) {
      cw->RemoveNickQuit(this->c->GetNick(), msg);
    }
  }
  this->c->SetNeedsShutdown(1);
  return SUCCESS;
}