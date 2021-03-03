#include "client.hpp"
Client::Client(int fd) {
  this->fd = fd;
  this->NeedsShutdown = 0;
  this->needsResolving = 0;
  this->user = "";
  this->nick = "";
  this->hostname = "";
  this->Queue = "";
  this->ResponseQueue.clear();
  this->NickSet = 0;
  this->UserSet = 0;
  this->IsRegistered = 0;
  this->pingsent = 0;
  this->caps = 0;
  this->mode = 0;
  this->away = 0;
  this->awaymsg = "";
  this->lastknocktime = 0;
  return;
}
Client::~Client() {
  this->ResponseQueue.clear();
  this->invites_to.clear();
  this->silenced.clear();
  return;
}
int Client::SendResponseQueue() {
  std::string t;
  if (this->ResponseQueue.size() <= 0) {
    return SUCCESS;
  }
  for (auto it = this->ResponseQueue.begin(); it != this->ResponseQueue.end();
       ++it) {
    t = *it;
    std::string u = t;
    rtrim(u);
    write(this->GetFd(), t.c_str(), t.size());
  }
  this->ResponseQueue.clear();
  return SUCCESS;
}
int Client::AddLineToResponseQueue(char *line) {
  if (line == NULL) {
    return SUCCESS;
  }
  this->ResponseQueue.push_back(std::string(line));
  return SUCCESS;
}
int Client::AddLineToResponseQueue(std::string line) {
  if (line == "") {
    return SUCCESS;
  }
  this->ResponseQueue.push_back(line);
  return SUCCESS;
}
int Client::ShutdownClient() {
  close(this->fd);
  return SUCCESS;
}
int Client::Read() {
  char c = 0;
  std::string cmd = "";
  int result = 0;
  do {
    result = read(this->GetFd(), &c, 1);
    if (result < 0) {
      this->SetNeedsShutdown(1);
      return result;
    } else if (result == 0) {
      this->SetNeedsShutdown(1);
      return result;
    }
    cmd += c;
  } while (c != '\x0a' && cmd.size() < 512);
  this->SetQueue(std::regex_replace(cmd, std::regex("[\r\n]+$"), ""));
  this->SetNeedsResolving(1);
  return result;
}
int Client::Write(std::string line) {
  return write(this->GetFd(), line.c_str(), line.size());
}
int Client::ClearQueue() {
  this->SetQueue("");
  this->SetNeedsResolving(0);
  return SUCCESS;
}
int Client::NickUserSet() { return this->NickSet & this->UserSet; }
int Client::IsSilenced(std::string nick) {
  if (std::find(this->silenced.begin(), this->silenced.end(), nick) !=
      this->silenced.end()) {
    return 1;
  }
  return 0;
}
void Client::AddSilenced(std::string nick) {
  if (!this->IsSilenced(nick)) {
    this->silenced.push_back(nick);
  }
  return;
}
void Client::RemoveSilenced(std::string nick) {
  for (auto it = this->silenced.begin(); it != this->silenced.end(); ++it) {
    if (*it == nick) {
      this->silenced.erase(it);
      return;
    }
  }
  return;
}