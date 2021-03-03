#include "channel.hpp"
#include "server.hpp"
void Channel::SetTopicSetBy(std::string nick) { this->topicsetby = nick; }
std::string Channel::GetTopicSetBy(void) { return this->topicsetby; }
void Channel::SetTopicTime(std::time_t t) { this->topictime = t; }
std::time_t Channel::GetTopicTime(void) { return this->topictime; }
void Channel::SetTopicSet(int flag) { this->topicset = flag; }
int Channel::GetTopicSet(void) { return this->topicset; }
void Channel::SetTopic(std::string topic) {
  if (topic.size() > MAXTOPICLEN) {
    topic.erase(topic.begin() + MAXTOPICLEN, topic.end());
  }
  this->topic = topic;
  this->SetTopicSet(1);
  this->SetTopicTime(time(nullptr));
}
std::string Channel::GetTopic(void) { return this->topic; }
void Channel::AddMember(Client *nc, std::string host) {
  Client *walker = NULL;
  std::string joinstring;
  this->members.push_back(nc);
  if (this->members.size() == 1) {
    this->AddOperator(nc->GetNick());
    this->AddOperator(nc->GetNick());
  }
  joinstring = ":" + nc->GetNick() + "!~" + nc->GetUser() + "@" +
               nc->GetHostname() + " JOIN :" + this->GetChannelName() + "\r\n";
  for (auto it = this->members.begin(); it != this->members.end(); ++it) {
    walker = *it;
    walker->AddLineToResponseQueue(joinstring);
  }
  nc->AddLineToResponseQueue(":irc." + host + " MODE " +
                             this->GetChannelName() + " " +
                             this->GetModeString() + "\r\n");
  return;
}
int Channel::GetUserCount(void) { return this->members.size(); }
void Channel::SetChannelName(std::string channel_name) {
  if (channel_name.size() > CHANNELLEN) {
    channel_name.erase(channel_name.begin() + MAXTOPICLEN, channel_name.end());
  }
  this->channel_name = channel_name;
}
std::string Channel::GetChannelName(void) { return this->channel_name; }
std::string Channel::GetModeString(void) {
  std::string modestring = "+";
  if (this->GetIsPrivate()) {
    modestring += 'p';
  }
  if (this->GetIsSecret()) {
    modestring += 's';
  }
  if (this->GetIsInviteOnly()) {
    modestring += 'i';
  }
  if (this->GetOperTopic()) {
    modestring += 't';
  }
  if (this->GetOnlyMembers()) {
    modestring += 'n';
  }
  if (this->GetIsModerated()) {
    modestring += 'm';
  }
  if (this->GetHasMax()) {
    modestring += 'l';
  }
  if (this->GetHasKey()) {
    modestring += 'k';
  }
  return modestring;
}
int Channel::HasVoice(std::string nick) {
  std::string walker;
  for (auto it = this->hasvoice.begin(); it != this->hasvoice.end(); ++it) {
    walker = *it;
    if (walker == nick) {
      return 1;
    }
  }
  return 0;
}
int Channel::AddVoice(std::string nick) {
  std::string walker;
  for (auto it = this->hasvoice.begin(); it != this->hasvoice.end(); ++it) {
    walker = *it;
    if (walker == nick) {
      return 0;
    }
  }
  this->hasvoice.push_back(nick);
  return 1;
}
int Channel::IsInvited(std::string nick) {
  std::string walker;
  for (auto it = this->invited.begin(); it != this->invited.end(); ++it) {
    walker = *it;
    if (walker == nick) {
      return 1;
    }
  }
  return 0;
}
int Channel::AddInvited(std::string nick) {
  std::string walker;
  for (auto it = this->invited.begin(); it != this->invited.end(); ++it) {
    walker = *it;
    if (walker == nick) {
      return 0;
    }
  }
  this->invited.push_back(nick);
  return 1;
}
int Channel::RemoveInvited(std::string nick) {
  std::string walker;
  for (auto it = this->invited.begin(); it != this->invited.end(); ++it) {
    walker = *it;
    if (walker == nick) {
      this->invited.erase(it);
      return 1;
    }
  }
  return 0;
}
int Channel::RemoveVoice(std::string nick) {
  std::string walker;
  for (auto it = this->hasvoice.begin(); it != this->hasvoice.end(); ++it) {
    walker = *it;
    if (walker == nick) {
      this->hasvoice.erase(it);
      return 1;
    }
  }
  return 0;
}
void Channel::SetIsModerated(int ismoderated) {
  this->ismoderated = ismoderated;
}
int Channel::GetIsModerated(void) { return this->ismoderated; }
void Channel::SetIsPrivate(int flag) { this->isprivate = flag; }
int Channel::GetIsPrivate(void) { return this->isprivate; }
void Channel::SetIsSecret(int flag) { this->issecret = flag; }
int Channel::GetIsSecret(void) { return this->issecret; }
void Channel::SetIsInviteOnly(int flag) { this->isinviteonly = flag; }
int Channel::GetIsInviteOnly(void) { return this->isinviteonly; }
void Channel::SetOperTopic(int flag) { this->opertopic = flag; }
int Channel::GetOperTopic(void) { return this->opertopic; }
void Channel::SetOnlyMembers(int flag) { this->onlymembers = flag; }
int Channel::GetOnlyMembers(void) { return this->onlymembers; }
void Channel::SetHasMax(int flag) { this->hasmax = flag; }
int Channel::GetHasMax(void) { return this->hasmax; }
void Channel::SetMaxUsers(int count) { this->maxusers = count; }
int Channel::GetMaxUsers(void) { return this->maxusers; }
void Channel::SetHasKey(int flag) { this->haskey = flag; }
int Channel::GetHasKey(void) { return this->haskey; }
void Channel::SetChannelKey(std::string key) { this->key = key; }
int Channel::IsCorrectKey(std::string key) {
  if (this->key == key) {
    return 1;
  }
  return 0;
}
