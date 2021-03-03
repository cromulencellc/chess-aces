#include "server.hpp"
Server::Server(int port) {
  char hp[MAXHNLEN];
  memset(hp, 0, MAXHNLEN);
  this->port = port;
  gethostname(hp, MAXHNLEN);
  this->SetServerHostname(hp);
  this->clients.clear();
  this->toread.clear();
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
  return 1;
}
int Server::AddClient(int clientfd, std::string ip) {
  Client *nc = new Client(clientfd);
  if (nc == NULL) {
    return FAIL;
  }
  this->clients.push_back(nc);
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
        return FAIL;
      }
      memset(&sa, 0, sizeof(sa));
      getsockname(evt->fd, (struct sockaddr *)&sa, &sa_len);
      memset(ipaddr, 0, INET_ADDRSTRLEN);
      inet_ntop(AF_INET, &(sa.sin_addr), ipaddr, INET_ADDRSTRLEN);
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
  for (std::vector<Client *>::iterator it = this->toread.begin();
       it != this->toread.end(); ++it) {
    walker = *it;
    walker->Read();
  }
  for (std::vector<Client *>::iterator it = this->toread.begin();
       it != this->toread.end(); it++) {
    walker = *it;
    if (walker->GetNeedsShutdown()) {
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
    this->HandleCommand(tokens, walker);
    walker->SetNeedsResolving(0);
    walker->ClearQueue();
    if (walker->GetNeedsShutdown()) {
      todelete.push_back(walker);
    }
  }
  this->toread.clear();
  for (auto it = todelete.begin(); it != todelete.end(); ++it) {
    walker = *it;
    for (auto w_it = this->clients.begin(); w_it != this->clients.end();
         ++w_it) {
      if (*w_it == walker) {
        this->clients.erase(w_it);
        break;
      }
    }
    walker->ShutdownClient();
    delete walker;
  }
  return SUCCESS;
}
int Server::ResolveResponses() {
  Client *walker = NULL;
  for (auto it = this->clients.begin(); it != this->clients.end(); ++it) {
    walker = *it;
    if (walker != NULL) {
      walker->SendResponseQueue();
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
  }
  return 1;
}
void Server::SetPort(int port) { this->port = port; }
int Server::GetPort(void) { return this->port; }
void Server::SetFd(int serverfd) { this->serverfd = serverfd; }
int Server::GetFd(void) { return this->serverfd; }
void Server::SetServerHostname(std::string serverHostname) {
  this->serverHostname = serverHostname;
}
std::string Server::GetServerHostname(void) { return this->serverHostname; }
