#ifndef __CHANNEL_HPP__
#define __CHANNEL_HPP__
#include <iostream>
#include <string>
#include <vector>
#include "client.hpp"
#include "utils.hpp"
typedef struct ban {
  std::regex banreg;
  std::string regexp;
  std::string nick;
  std::time_t t;
} ban;
class Channel {
  std::string channel_name;
  std::vector<Client *> members;
  std::vector<std::string> operators;
  std::vector<std::string> invited;
  std::vector<std::string> hasvoice;
  std::vector<struct ban *> banlist;
  std::time_t topictime;
  int topicset;
  std::string topicsetby;
  std::string key;
  std::string topic;
  int isprivate;
  int issecret;
  int isinviteonly;
  int opertopic;
  int onlymembers;
  int ismoderated;
  int hasmax;
  int maxusers;
  int haskey;
public:
  Channel(std::string);
  ~Channel();
  void PrintBanList(Client *c, std::string serverhost);
  int AddBanExp(Client *c, std::string banexp);
  int IsBanned(std::string nick);
  int GetMemberCount(void);
  std::string GetMemberList(void);
  int HasVoice(std::string nick);
  int AddVoice(std::string nick);
  int RemoveVoice(std::string nick);
  int RemoveInvited(std::string nick);
  int IsInvited(std::string nick);
  int AddInvited(std::string nick);
  void SetIsModerated(int ismoderated);
  int GetIsModerated(void);
  void SetTopicSetBy(std::string nick);
  std::string GetTopicSetBy(void);
  void SetTopicTime(std::time_t t);
  std::time_t GetTopicTime(void);
  void SetTopicSet(int flag);
  int GetTopicSet(void);
  void SetIsPrivate(int flag);
  int GetIsPrivate(void);
  void SetIsSecret(int flag);
  int GetIsSecret(void);
  void SetIsInviteOnly(int flag);
  int GetIsInviteOnly(void);
  void SetOperTopic(int flag);
  int GetOperTopic(void);
  void SetOnlyMembers(int flag);
  int GetOnlyMembers(void);
  void SetHasMax(int flag);
  int GetHasMax(void);
  void SetMaxUsers(int count);
  int GetMaxUsers(void);
  int GetUserCount(void);
  void SetHasKey(int flag);
  int GetHasKey(void);
  void SetChannelKey(std::string key);
  int IsCorrectKey(std::string key);
  void SetTopic(std::string topic);
  std::string GetTopic(void);
  void AddMember(Client *nc, std::string host);
  void AddOperator(std::string nick);
  void RemoveOperator(std::string nick);
  int IsOperator(std::string nick);
  void SetChannelName(std::string channel_name);
  std::string GetChannelName(void);
  int IsMember(std::string nick);
  int SendChannelMessage(Client *c, std::string message,
                         std::string serverhost);
  int SendChannelNotice(Client *c, std::string message, std::string serverhost);
  int SendAllClients(std::string message);
  int SendAllOperators(std::string message);
  int RemoveNickQuit(std::string nick, std::string message);
  int RemoveNickPart(std::string nick, std::string message);
  int RemoveNick(std::string nick);
  std::string GetModeString(void);
  int MakeWhoString(Client *c, std::string serverhost);
};
#endif