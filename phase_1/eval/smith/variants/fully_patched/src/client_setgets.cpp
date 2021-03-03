#include "client.hpp"
#include "server.hpp"
void Client::SetLastKnockTime(std::time_t knocktime) {
  this->lastknocktime = knocktime;
}
std::time_t Client::GetLastKnockTime(void) { return this->lastknocktime; }
void Client::SetAwayMsg(std::string msg) {
  if (msg.size() > MAXAWAYMSGLEN) {
    msg.erase(msg.begin() + MAXAWAYMSGLEN, msg.end());
  }
  this->awaymsg = msg;
}
std::string Client::GetAwayMsg(void) { return this->awaymsg; }
int Client::GetAway(void) { return this->away; }
void Client::SetAway(int flag) { this->away = flag; }
void Client::SetPingSent(int flag) { this->pingsent = flag; }
int Client::GetPingSent(void) { return this->pingsent; }
void Client::AddInviteTo(std::string channel) {
  this->invites_to.push_back(channel);
}
int Client::IsInvited(std::string channel) {
  for (auto it = this->invites_to.begin(); it != this->invites_to.end(); ++it) {
    if (*it == channel) {
      return 1;
    }
  }
  return 0;
}
void Client::SetNeedsResolving(int needsResolving) {
  this->needsResolving = needsResolving;
}
int Client::GetNeedsResolving(void) { return this->needsResolving; }
void Client::SetQueue(std::string Queue) { this->Queue = Queue; }
std::string Client::GetQueue(void) { return this->Queue; }
void Client::SetUser(std::string user) {
  if (user.size() > MAXUSERLEN) {
    user.erase(user.begin() + MAXUSERLEN, user.end());
  }
  this->user = user;
  this->UserSet = 1;
}
std::string Client::GetUser(void) { return this->user; }
void Client::SetNick(std::string nick) {
  if (nick.size() > MAXNICKLEN) {
    nick.erase(nick.begin() + MAXNICKLEN, nick.end());
  }
  this->nick = nick;
  this->NickSet = 1;
}
std::string Client::GetNick(void) { return this->nick; }
void Client::SetHostname(std::string hostname) {
  if (hostname.size() > MAXHNLEN) {
    hostname.erase(hostname.begin() + MAXHNLEN, hostname.end());
  }
  this->hostname = hostname;
}
std::string Client::GetHostname(void) { return this->hostname; }
int Client::GetFd(void) { return this->fd; }
void Client::SetFd(int fd) { this->fd = fd; }
void Client::SetNeedsShutdown(int NeedsShutdown) {
  this->NeedsShutdown = NeedsShutdown;
}
int Client::GetNeedsShutdown(void) { return this->NeedsShutdown; }
void Client::SetRegistered(int IsRegistered) {
  this->IsRegistered = IsRegistered;
}
int Client::GetRegistered(void) { return this->IsRegistered; }
void Client::SetCapFlag(int flag) { this->caps |= flag; }
int Client::GetCapFlag(int flag) { return this->caps; }
int Client::CapSet(int flag) { return this->caps & flag; }
void Client::RemoveCap(int flag) { this->caps = caps ^ flag; }
int Client::IsNickSet(void) { return this->NickSet; }
void Client::SetConnectTime(std::time_t connect_time) {
  this->connect_time = connect_time;
}
std::time_t Client::GetConnectTime(void) { return this->connect_time; }
void Client::SetLastMsg(std::time_t last_msg) { this->last_msg = last_msg; }
std::time_t Client::GetLastMsg(void) { return this->last_msg; }