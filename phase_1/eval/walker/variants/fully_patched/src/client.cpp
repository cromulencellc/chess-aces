#include "client.hpp"
#include "utils.hpp"
Client::Client(int fd) {
  this->fd = fd;
  this->NeedsShutdown = 0;
  this->needsResolving = 0;
  this->selected = NULL;
  this->maildir = "";
  this->Queue = "";
  this->username = "";
  this->ResponseQueue.clear();
  this->loggedin = 0;
  return;
}
Client::~Client() {
  this->ResponseQueue.clear();
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
  free_mailbox(this->selected);
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
void Client::SetNeedsResolving(int needsResolving) {
  this->needsResolving = needsResolving;
}
int Client::GetNeedsResolving(void) { return this->needsResolving; }
void Client::SetQueue(std::string Queue) { this->Queue = Queue; }
std::string Client::GetQueue(void) { return this->Queue; }
void Client::SetUid(int value) { this->uid = value; }
int Client::GetUid(void) { return this->uid; }
void Client::SetGid(int value) { this->gid = value; }
int Client::GetGid(void) { return this->gid; }
int Client::GetFd(void) { return this->fd; }
void Client::SetFd(int fd) { this->fd = fd; }
void Client::SetNeedsShutdown(int NeedsShutdown) {
  this->NeedsShutdown = NeedsShutdown;
}
int Client::GetNeedsShutdown(void) { return this->NeedsShutdown; }
void Client::SetLoggedIn(int value) { this->loggedin = value; }
int Client::GetLoggedIn(void) { return this->loggedin; }
void Client::SetMailDir(std::string MailDir) { this->maildir = MailDir; }
std::string Client::GetMailDir(void) { return this->maildir; }
void Client::SetSelected(mailbox *Selected) { this->selected = Selected; }
mailbox *Client::GetSelected(void) { return this->selected; }
void Client::SetUsername(std::string username) { this->username = username; }
std::string Client::GetUsername(void) { return this->username; }
