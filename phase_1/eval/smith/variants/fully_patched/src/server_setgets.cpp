#include "server.hpp"
void Server::SetPort(int port) { this->port = port; }
int Server::GetPort(void) { return this->port; }
void Server::SetFd(int serverfd) { this->serverfd = serverfd; }
int Server::GetFd(void) { return this->serverfd; }
void Server::SetServerHostname(std::string serverHostname) {
  this->serverHostname = serverHostname;
}
std::string Server::GetServerHostname(void) { return this->serverHostname; }
void Server::SetLogfile(std::string logfile) { this->logfile = logfile; }
std::string Server::GetLogfile(void) { return this->logfile; }
void Server::SetLogging(int logging) { this->logging = logging; }
int Server::GetLogging(void) { return this->logging; }