#include "server.hpp"
void Server::WriteLogEntry(std::string logline) {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ofstream *lf = NULL;
  if (!this->logging) {
    return;
  }
  try {
    lf = new std::ofstream(this->GetLogfile(),
                           std::ios::out | std::ios::binary | std::ios::app);
    ;
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Failed to write to logfile" << std::endl;
    return;
  }
  *lf << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
  *lf << ": " << logline << std::endl;
  lf->close();
  delete lf;
  return;
}