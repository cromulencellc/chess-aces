#include "server.hpp"
int Server::ResolveCommands() {
  connection *walker = NULL;
  std::vector<connection *> temp_clients(this->clients);
  for (std::vector<connection *>::iterator it = temp_clients.begin();
       it != temp_clients.end(); ++it) {
    walker = *it;
    if (walker->CommandWaiting()) {
      if (walker->ParseCommand() == SUCCESS) {
        walker->HandleCommand();
      }
    }
  }
  return SUCCESS;
}
int Server::Poll() {
  connection *walker = NULL;
  int index = 0;
  int connfd, result;
  struct pollfd *evt;
  struct sockaddr_in ca;
  struct sockaddr_in sa;
  socklen_t ca_len = sizeof(struct sockaddr_in);
  socklen_t sa_len = sizeof(struct sockaddr_in);
  std::vector<int> needsAccept;
  memset(&ca, 0, sizeof(struct sockaddr_in));
  std::array<struct pollfd, 1024> evs;
  for (auto it = evs.begin(); it != evs.end(); ++it) {
    memset(&(*it), 0, sizeof(struct pollfd));
  }
  evs[0].fd = this->sockfd;
  evs[0].events = POLLIN;
  needsAccept.clear();
  needsAccept.push_back(this->sockfd);
  for (std::vector<connection *>::iterator it = this->clients.begin();
       it != this->clients.end(); ++it) {
    walker = *it;
    std::vector<connection *>::iterator index_it =
        std::find(this->clients.begin(), this->clients.end(), walker);
    index = std::distance(this->clients.begin(), index_it) + 1;
    evs[index].fd = walker->get_fd();
    evs[index].events = POLLIN;
    if (walker->getNeedsAccept() & 1) {
      needsAccept.push_back(walker->get_fd());
    }
    if (walker->getSingleUse() && !(walker->getNeedsAccept() & 1)) {
      evs[index].events = POLLOUT;
    }
  }
  result = poll(evs.data(), this->clients.size() + 1, 5000);
  if (result < 0) {
    return FAIL;
  } else if (result == 0) {
    return SUCCESS;
  }
  int handled;
  for (auto it = evs.begin(); it != evs.end(); ++it) {
    evt = it;
    if (evt->revents == POLLIN) {
      handled = 0;
      for (auto fdit = needsAccept.begin(); fdit != needsAccept.end(); ++fdit) {
        if (evt->fd == *fdit) {
          connfd = accept(evt->fd, (struct sockaddr *)&ca, &ca_len);
          if (connfd < 0) {
            fprintf(stderr, "accept() failed: %s\n", strerror(errno));
            return FAIL;
          }
          memset(&sa, 0, sizeof(sa));
          getsockname(evt->fd, (struct sockaddr *)&sa, &sa_len);
          if (this->port == htons(sa.sin_port)) {
            this->AddConnection(connfd, &ca);
          } else {
            index = std::distance(evs.begin(), it) - 1;
            close(evt->fd);
            if (this->clients.size() <= index) {
              exit(0);
            }
            this->clients[index]->setNeedsAccept(
                this->clients[index]->getNeedsAccept() ^ 1);
            this->clients[index]->setFD(connfd);
          }
          handled = 1;
        } else {
          continue;
        }
      }
      if (!handled) {
        index = std::distance(evs.begin(), it) - 1;
        if (this->clients.size() <= index) {
          exit(0);
        }
        this->poll_list.push_back(this->clients[index]);
      }
    } else if (evt->revents == POLLOUT) {
      index = std::distance(evs.begin(), it) - 1;
      this->poll_list.push_back(this->clients[index]);
    }
  }
  return SUCCESS;
}
int Server::ResolveShutdown() {
  connection *walker = NULL;
  int index;
  for (std::vector<connection *>::iterator it = this->clients.begin();
       it != this->clients.end(); ++it) {
    walker = *it;
    if (walker->NeedsRemoval()) {
      this->to_remove.push_back(walker);
    }
  }
  for (std::vector<connection *>::iterator it = this->to_remove.begin();
       it != this->to_remove.end(); ++it) {
    walker = *it;
    std::vector<connection *>::iterator index_it =
        std::find(this->clients.begin(), this->clients.end(), walker);
    index = std::distance(this->clients.begin(), index_it);
    this->clients.erase(this->clients.begin() + index);
    delete walker;
  }
  this->to_remove.clear();
  return SUCCESS;
}
int Server::ResolvePolls() {
  connection *walker = NULL;
  int index;
  for (std::vector<connection *>::iterator it = this->poll_list.begin();
       it != this->poll_list.end(); ++it) {
    walker = *it;
    if (walker->getSingleUse()) {
      if (walker->getNeedsAccept() & 1) {
        if (walker->getDataToSend()) {
          walker->AcceptToSend();
        } else {
          if (walker->getNeedsAccept() & 2) {
            walker->AcceptToAppend();
          } else {
            walker->AcceptToRead();
          }
        }
      } else {
        if (walker->getDataToSend()) {
          walker->FlushToSend();
        } else {
          if (walker->getNeedsAccept() & 2) {
            walker->FlushFromAppend();
          } else {
            walker->FlushFromRead();
          }
        }
      }
      continue;
    }
    walker->Read();
  }
  for (std::vector<connection *>::iterator it = this->poll_list.begin();
       it != this->poll_list.end(); ++it) {
    walker = *it;
    if (walker->NeedsRemoval()) {
      std::vector<connection *>::iterator it =
          std::find(this->clients.begin(), this->clients.end(), walker);
      index = std::distance(this->clients.begin(), it);
      this->clients.erase(this->clients.begin() + index);
      delete walker;
    }
  }
  this->poll_list.clear();
  return SUCCESS;
}
int Server::AddConnection(int fd, struct sockaddr_in *clnt) {
  if (this->clients.size() >= 1023) {
    close(fd);
    return SUCCESS;
  }
  connection *nc = new connection(fd, clnt);
  if (nc == NULL) {
    return FAIL;
  }
  nc->SendBanner();
  this->clients.push_back(nc);
  return SUCCESS;
}
int Server::AddSingleShotRETR(int fd, int controlfd, char *buffer, int length,
                              std::string fp, std::string success,
                              int needsAccept) {
  if (this->clients.size() >= 1023) {
    close(fd);
    return SUCCESS;
  }
  connection *nc = new connection(fd, NULL);
  if (nc == NULL) {
    return FAIL;
  }
  nc->setSingleUse(1);
  nc->setDataToSend(buffer);
  nc->setDataLength(length);
  nc->setNeedsAccept(needsAccept);
  nc->setControlfd(controlfd);
  nc->setOutfile(fp);
  nc->setSuccessString(success);
  this->clients.push_back(nc);
  return SUCCESS;
}
Server::Server(int port) {
  this->port = port;
  this->stdin_only = 0;
  this->clients.clear();
  if (this->setup_socket()) {
    exit(1);
    return;
  }
  return;
}
Server::Server() {
  this->stdin_only = 1;
  return;
}
int Server::SetFds() {
  connection *walker = NULL;
  int maxfd = -1;
  FD_ZERO(&this->readfds);
  FD_SET(this->sockfd, &this->readfds);
  maxfd = this->sockfd;
  for (std::vector<connection *>::iterator it = this->clients.begin();
       it != this->clients.end(); ++it) {
    walker = *it;
    FD_SET(walker->get_fd(), &this->readfds);
    if (maxfd < walker->get_fd()) {
      maxfd = walker->get_fd();
    }
  }
  return maxfd + 1;
}
int Server::select_loop() {
  struct sockaddr_in ca;
  memset(&ca, 0, sizeof(struct sockaddr_in));
  while (1) {
    this->Poll();
    this->ResolvePolls();
    this->ResolveCommands();
    this->ResolveShutdown();
  }
  return 0;
}
int Server::setup_socket() {
  int enable = 1;
  this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (this->sockfd < 0) {
    fprintf(stderr, "socket() failed: %s\n", strerror(errno));
    return 1;
  }
  memset(&this->srvr, 0, sizeof(struct sockaddr_in));
  this->srvr.sin_family = AF_INET;
  this->srvr.sin_addr.s_addr = INADDR_ANY;
  this->srvr.sin_port = htons(this->port);
  if (bind(this->sockfd, (struct sockaddr *)&this->srvr, sizeof(this->srvr)) <
      0) {
    fprintf(stderr, "bind() failed: %s\n", strerror(errno));
    close(this->sockfd);
    return 1;
  }
  if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &enable,
                 sizeof(enable)) < 0) {
    fprintf(stderr, "setsockopt() failed: %s\n", strerror(errno));
    close(this->sockfd);
    return 1;
  }
  if (listen(this->sockfd, 1024) < 0) {
    fprintf(stderr, "listen() failed: %s\n", strerror(errno));
    close(this->sockfd);
    return 1;
  }
  fprintf(stderr, "[INFO] Listener socket on port: %d\n", this->port);
  return 0;
}