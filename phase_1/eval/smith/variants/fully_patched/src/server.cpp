#include "server.hpp"
Server::Server(int port) {
  char hp[MAXHNLEN];
  memset(hp, 0, MAXHNLEN);
  this->port = port;
  gethostname(hp, MAXHNLEN);
  this->SetServerHostname(hp);
  this->logging = 0;
  this->logfile = "log.txt";
  this->clients.clear();
  this->toread.clear();
  this->channels.clear();
  return;
}
int Server::initializeSocket() {
  struct sockaddr_in sa;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  memset(&sa, 0, sizeof(sa));
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    close(fd);
    return 0;
  }
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port = htons(this->GetPort());
  if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    close(fd);
    return 0;
  }
  if (listen(fd, 30) < 0) {
    close(fd);
    return 0;
  }
  this->SetFd(fd);
  this->WriteLogEntry("Started listener on port " +
                      std::to_string(this->GetPort()));
  return 1;
}
int Server::AddClient(int clientfd, std::string ip) {
  Client *nc = new Client(clientfd);
  char response[300];
  if (nc == NULL) {
    this->WriteLogEntry("error allocating new client");
    return FAIL;
  }
  this->clients.push_back(nc);
  nc->SetConnectTime(std::time(nullptr));
  nc->SetLastMsg(std::time(nullptr));
  makeresponse(
      response,
      ":irc.%s  020 * :Please wait while we process your connection.\r\n",
      this->GetServerHostname().c_str());
  nc->AddLineToResponseQueue(response);
  return SUCCESS;
}
int Server::Poll() {
  Client *walker = NULL;
  int index = 0;
  int connfd, result;
  struct pollfd *evt;
  struct sockaddr_in ca;
  struct sockaddr_in sa;
  socklen_t ca_len = sizeof(struct sockaddr_in);
  socklen_t sa_len = sizeof(struct sockaddr_in);
  char ipaddr[INET_ADDRSTRLEN];
  memset(&ca, 0, sizeof(struct sockaddr_in));
  std::array<struct pollfd, 1024> evs;
  for (auto it = evs.begin(); it != evs.end(); ++it) {
    memset(&(*it), 0, sizeof(struct pollfd));
  }
  evs[0].fd = this->GetFd();
  evs[0].events = POLLIN;
  for (std::vector<Client *>::iterator it = this->clients.begin();
       it != this->clients.end(); ++it) {
    walker = *it;
    std::vector<Client *>::iterator index_it =
        std::find(this->clients.begin(), this->clients.end(), walker);
    index = std::distance(this->clients.begin(), index_it) + 1;
    evs[index].fd = walker->GetFd();
    evs[index].events = POLLIN;
  }
  result = poll(evs.data(), this->clients.size() + 1, 1000);
  if (result < 0) {
    return FAIL;
  } else if (result == 0) {
    return SUCCESS;
  }
  for (auto it = evs.begin(); it != evs.begin() + this->clients.size() + 1;
       ++it) {
    evt = it;
    if (evt->revents == POLLIN && evt->fd == this->GetFd()) {
      connfd = accept(evt->fd, (struct sockaddr *)&ca, &ca_len);
      if (connfd < 0) {
        this->WriteLogEntry("accept() failed: " + std::string(strerror(errno)));
        return FAIL;
      }
      memset(&sa, 0, sizeof(sa));
      getsockname(evt->fd, (struct sockaddr *)&sa, &sa_len);
      memset(ipaddr, 0, INET_ADDRSTRLEN);
      inet_ntop(AF_INET, &(sa.sin_addr), ipaddr, INET_ADDRSTRLEN);
      this->WriteLogEntry("Accepted client from: " + std::string(ipaddr));
      this->AddClient(connfd, ipaddr);
    } else if (evt->revents == POLLIN) {
      index = std::distance(evs.begin(), it) - 1;
      this->toread.push_back(this->clients[index]);
    } else if (evt->revents & POLLERR) {
      index = std::distance(evs.begin(), it) - 1;
      this->clients[index]->SetNeedsShutdown(1);
      this->toread.push_back(this->clients[index]);
    }
  }
  return SUCCESS;
}
int Server::ResolvePolls() {
  Client *walker = NULL;
  Channel *ch = NULL;
  for (std::vector<Client *>::iterator it = this->toread.begin();
       it != this->toread.end(); ++it) {
    walker = *it;
    walker->Read();
  }
  for (std::vector<Client *>::iterator it = this->toread.begin();
       it != this->toread.end(); it++) {
    walker = *it;
    if (walker->GetNeedsShutdown()) {
      for (auto shutit = this->channels.begin(); shutit != this->channels.end();
           ++shutit) {
        ch = *shutit;
        if (ch->IsMember(walker->GetNick())) {
          ch->RemoveNickQuit(walker->GetNick(), "quit");
        }
      }
      walker->ShutdownClient();
      std::vector<Client *>::iterator tempit =
          std::find(this->clients.begin(), this->clients.end(), walker);
      if (tempit != this->clients.end()) {
        this->clients.erase(tempit);
      }
      this->toread.erase(it--);
      delete walker;
    }
  }
  return SUCCESS;
}
int Server::ParseReads() {
  Client *walker = NULL;
  std::vector<std::string> tokens;
  std::vector<Client *> todelete;
  todelete.clear();
  for (std::vector<Client *>::iterator it = this->toread.begin();
       it != this->toread.end(); ++it) {
    walker = *it;
    tokens = tokenize_line(walker->GetQueue(), ' ');
    this->tokens = tokens;
    this->c = walker;
    this->HandleCommand();
    walker->SetLastMsg(std::time(nullptr));
    walker->SetNeedsResolving(0);
    walker->SetPingSent(0);
    walker->ClearQueue();
    if (walker->GetNeedsShutdown()) {
      todelete.push_back(walker);
    }
  }
  this->toread.clear();
  for (auto it = todelete.begin(); it != todelete.end(); ++it) {
    walker = *it;
    walker->ShutdownClient();
    this->RemoveClientByNick(walker->GetNick());
    delete walker;
  }
  return SUCCESS;
}
int Server::ResolveResponses() {
  Client *walker = NULL;
  for (auto it = this->clients.begin(); it != this->clients.end(); ++it) {
    walker = *it;
    walker->SendResponseQueue();
  }
  return SUCCESS;
}
int Server::CleanupChannels() {
  Channel *walker;
  for (auto it = this->channels.begin(); it != this->channels.end(); it++) {
    walker = *it;
    if (walker->GetMemberCount() == 0) {
      this->channels.erase(it--);
      delete walker;
    }
  }
  return 1;
}
int Server::CheckIdles() {
  std::time_t ct;
  std::time_t idle;
  Client *walker = NULL;
  Channel *ch;
  char response[256];
  for (auto it = this->clients.begin(); it != this->clients.end(); it++) {
    ct = std::time(nullptr);
    walker = *it;
    idle = ct - walker->GetLastMsg();
    if (idle > 20 && (walker->GetPingSent() == 0)) {
      makeresponse(response, "PING :irc.%s\r\n",
                   this->GetServerHostname().c_str());
      walker->AddLineToResponseQueue(response);
      walker->SetPingSent(1);
    } else if (idle > 256) {
      walker->Write("ERROR :Closing Link: (Ping timeout: 256 seconds)\r\n");
      for (auto shutit = this->channels.begin(); shutit != this->channels.end();
           ++shutit) {
        ch = *shutit;
        if (ch->IsMember(walker->GetNick())) {
          ch->RemoveNickQuit(walker->GetNick(), "quit");
        }
      }
      walker->ShutdownClient();
      this->clients.erase(it--);
      delete walker;
    }
  }
  return SUCCESS;
}
int Server::Run() {
  if (!this->initializeSocket()) {
    std::cout << "[ERROR] Fail to setup the server socket" << std::endl;
    return 0;
  }
  while (1) {
    this->Poll();
    this->ResolvePolls();
    this->ParseReads();
    this->ResolveResponses();
    this->CheckIdles();
    this->CleanupChannels();
  }
  return 1;
}