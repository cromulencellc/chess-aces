#include "connection.hpp"
#include "server.hpp"
#include <fcntl.h>
#include <net/if.h>
extern std::vector<ftpuser *> users_g;
extern std::string userlist;
extern Server *srvr;
void connection::SetUser(std::string user) {
  this->user = user;
  this->userset = 1;
}
int connection::getIsLocal(void) { return this->islocal; }
void connection::setOutfile(std::string outfile) { this->outfile = outfile; }
std::string connection::getOutfile(void) { return this->outfile; }
void connection::setSuccessString(std::string success) {
  this->success = success;
}
std::string connection::getSuccessString(void) { return this->success; }
int connection::getControlfd(void) { return this->controlfd; }
void connection::setControlfd(int controlfd) { this->controlfd = controlfd; }
int connection::getDataLength(void) { return this->datalength; }
char *connection::getDataToSend(void) { return this->data_to_send; }
int connection::getNeedsAccept(void) { return this->needs_accept; }
int connection::getSingleUse(void) { return this->single_use; }
void connection::SetCwd(std::string cwd) { this->cwd = cwd; }
std::string connection::GetCwd(void) { return this->cwd; }
void connection::SetRestValue(unsigned int rv) { this->restvalue = rv; }
unsigned int connection::GetRestValue(void) { return this->restvalue; }
void connection::setFD(int fd) { this->fd = fd; }
void connection::SetPass(std::string pass) {
  this->pass = pass;
  this->passset = 1;
}
void connection::FlushToSend(void) {
  int cfd;
  if (this->data_to_send == NULL) {
    return;
  }
  write(this->fd, this->data_to_send, this->datalength);
  free(this->data_to_send);
  this->data_to_send = NULL;
  this->needs_shutdown = 1;
  cfd = this->getControlfd();
  write(cfd, this->success.c_str(), this->success.size());
  return;
}
void connection::AcceptToSend(void) {
  int datafd;
  int cfd = this->getControlfd();
  struct sockaddr_in ca;
  socklen_t ca_len = sizeof(ca);
  if (this->data_to_send == NULL) {
    return;
  }
  datafd = accept(this->fd, (struct sockaddr *)&ca, &ca_len);
  close(this->fd);
  this->needs_shutdown = 1;
  if (datafd < 0) {
    write(cfd, "425 Failed to establish connection.\r\n", 37);
    return;
  }
  write(datafd, this->data_to_send, this->datalength);
  free(this->data_to_send);
  this->data_to_send = NULL;
  close(datafd);
  write(cfd, this->success.c_str(), this->success.size());
  return;
}
void connection::FlushFromRead(void) {
  int length;
  char *filedata = NULL;
  int cfd = this->getControlfd();
  std::ofstream *file = NULL;
  length = read_data(this->fd, &filedata);
  this->needs_shutdown = 1;
  if (length == 0) {
    write(cfd, "553 Could not create file.\r\n", 28);
    return;
  }
  try {
    file =
        new std::ofstream(this->getOutfile(), std::ios::out | std::ios::binary);
  } catch (const std::exception &e) {
    write(cfd, "553 Could not create file.\r\n", 28);
    return;
  }
  file->write(filedata, length);
  file->close();
  write(cfd, this->success.c_str(), this->success.size());
  free(filedata);
  return;
}
void connection::AcceptToRead(void) {
  int datafd;
  int length;
  char *filedata = NULL;
  int cfd = this->getControlfd();
  std::ofstream *file = NULL;
  struct sockaddr_in ca;
  socklen_t ca_len = sizeof(ca);
  datafd = accept(this->fd, (struct sockaddr *)&ca, &ca_len);
  this->needs_shutdown = 1;
  if (datafd < 0) {
    write(cfd, "425 Failed to establish connection.\r\n", 37);
    return;
  }
  length = read_data(datafd, &filedata);
  close(datafd);
  if (length == 0) {
    write(cfd, "553 Could not create file.\r\n", 28);
    return;
  }
  try {
    file =
        new std::ofstream(this->getOutfile(), std::ios::out | std::ios::binary);
  } catch (const std::exception &e) {
    write(cfd, "553 Could not create file.\r\n", 28);
    return;
  }
  file->write(filedata, length);
  file->close();
  write(cfd, this->success.c_str(), this->success.size());
  free(filedata);
  return;
}
void connection::FlushFromAppend(void) {
  int length;
  char *filedata = NULL;
  int cfd = this->getControlfd();
  std::ofstream *file = NULL;
  length = read_data(this->fd, &filedata);
  close(this->fd);
  this->needs_shutdown = 1;
  if (length == 0) {
    write(cfd, "553 Could not create file.\r\n", 28);
    return;
  }
  try {
    file = new std::ofstream(this->getOutfile(),
                             std::ios::out | std::ios::binary | std::ios::app);
  } catch (const std::exception &e) {
    write(cfd, "553 Could not create file.\r\n", 28);
    return;
  }
  file->write(filedata, length);
  file->close();
  write(cfd, this->success.c_str(), this->success.size());
  free(filedata);
  return;
}
void connection::AcceptToAppend(void) {
  int datafd;
  int length;
  char *filedata = NULL;
  int cfd = this->getControlfd();
  std::ofstream *file = NULL;
  struct sockaddr_in ca;
  socklen_t ca_len = sizeof(ca);
  datafd = accept(this->fd, (struct sockaddr *)&ca, &ca_len);
  close(this->fd);
  this->needs_shutdown = 1;
  if (datafd < 0) {
    write(cfd, "425 Failed to establish connection.\r\n", 37);
    return;
  }
  length = read_data(datafd, &filedata);
  close(datafd);
  if (length == 0) {
    write(cfd, "553 Could not create file.\r\n", 28);
    return;
  }
  try {
    file = new std::ofstream(this->getOutfile(),
                             std::ios::out | std::ios::binary | std::ios::app);
  } catch (const std::exception &e) {
    write(cfd, "553 Could not create file.\r\n", 28);
    return;
  }
  file->write(filedata, length);
  file->close();
  write(cfd, this->success.c_str(), this->success.size());
  free(filedata);
  return;
}
int connection::GetMode(void) { return this->mode; }
void connection::SetMode(int mode) { this->mode = mode; }
char connection::GetStructType(void) { return this->structtype; }
void connection::SetStructType(char st) { this->structtype = st; }
int connection::CheckAuth() {
  if (!this->userset || !this->passset) {
    this->Write("530 Login incorrect.\r\n");
    return FAIL;
  }
  for (std::vector<ftpuser *>::iterator it = users_g.begin();
       it != users_g.end(); ++it) {
    if ((*it)->isyou(this->user)) {
      if ((*it)->goodpass(this->pass)) {
        if (this->clnt.sin_addr.s_addr != 16777343) {
          if (checkperms((*it)->getperms(), CANLOGINREMOTE) == 0) {
            this->Write("530 Cannot login with account.\r\n");
            return FAIL;
          }
        }
        this->authd = 1;
        this->Write("230 Login successful.\r\n");
        this->userclass = *it;
        if (this->islocal) {
          this->userclass->addperm(CANRETRPLIST | CANRETROUTSIDE);
        }
        this->SetCwd(this->userclass->gethomedir());
        return SUCCESS;
      }
    }
  }
  this->Write("530 Login incorrect.\r\n");
  return FAIL;
}
int connection::HandleABOR(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(ABOR)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleACCT(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(ACCT)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleALLO(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(ALLO)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleAPPE(void) {
  std::string dest;
  std::string storeFile;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path hd;
  std::filesystem::path pp;
  std::filesystem::path plist;
  std::filesystem::path token;
  int appefd;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->portset && !this->pasv) {
    this->Write("425 Use PORT or PASV first.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(APPE)) {
    this->Write("550 Permission denied.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  if (this->command_tokens.size() < 2) {
    this->Write("553 Could not create file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  storeFile = this->command_tokens[1];
  if (endswith(storeFile, "/")) {
    this->Write("553 Could not create file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  relpath.assign(storeFile);
  if (!relpath.has_root_path()) {
    storeFile = this->GetCwd() + "/" + storeFile;
  }
  fullpath.assign(storeFile);
  if (std::filesystem::exists(fullpath)) {
    if (!std::filesystem::is_regular_file(fullpath)) {
      this->Write("550 Failed to open file.\r\n");
      this->portset = 0;
      if (this->pasv) {
        close(this->pasvfd);
        this->pasv = 0;
      }
      return FAIL;
    }
  }
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Failed to open file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = fullpath.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == fullpath.end()) {
        this->Write("550 Failed to open file.\r\n");
        this->portset = 0;
        if (this->pasv) {
          close(this->pasvfd);
          this->pasv = 0;
        }
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        this->portset = 0;
        if (this->pasv) {
          close(this->pasvfd);
          this->pasv = 0;
        }
        return FAIL;
      }
    }
  }
  plist.assign(userlist);
  token.assign("/tmp/admin/token");
  if (std::filesystem::equivalent(plist, fullpath)) {
    this->Write("550 Permission denied.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  try {
    if (std::filesystem::equivalent(token, fullpath)) {
      this->Write("550 Permission denied.\r\n");
      this->portset = 0;
      if (this->pasv) {
        close(this->pasvfd);
        this->pasv = 0;
      }
      return FAIL;
    }
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Need access to the file" << std::endl;
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  this->Write("150 Ok to send data.\r\n");
  if (this->pasv) {
    srvr->AddSingleShotRETR(this->pasvfd, this->fd, NULL, 0, fullpath.string(),
                            "226 Transfer complete.\r\n", 3);
    this->pasv = 0;
  } else {
    appefd = connect_socket(this->targetip, this->targetport);
    this->portset = 0;
    if (appefd < 0) {
      this->Write("425 Failed to establish connection.\r\n");
      return FAIL;
    }
    srvr->AddSingleShotRETR(appefd, this->fd, NULL, 0, fullpath.string(),
                            "226 Transfer complete.\r\n", 2);
  }
  this->portset = 0;
  this->pasv = 0;
  return SUCCESS;
}
int connection::HandleCDUP(void) {
  std::string newDir;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CDUP)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  try {
    canon.assign(this->GetCwd());
    canon = std::filesystem::canonical(canon);
    canon = canon.parent_path();
  } catch (const std::exception &e) {
    this->Write("550 Failed to change directory.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    if (!startswith(canon.string(), this->userclass->gethomedir())) {
      this->Write("550 Permission denied.\r\n");
      return FAIL;
    }
  }
  this->Write("250 Directory successfully changed.\r\n");
  this->SetCwd(canon.string());
  return SUCCESS;
}
int connection::HandleCWD(void) {
  std::string newDir;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CWD)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("550 Failed to change directory.\r\n");
    return FAIL;
  }
  newDir = this->command_tokens[1];
  relpath.assign(newDir);
  if (!relpath.has_root_path()) {
    newDir = this->GetCwd() + "/" + newDir;
  }
  fullpath = std::filesystem::relative(newDir, this->GetCwd());
  try {
    canon.assign(this->GetCwd() + "/" + fullpath.string());
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    this->Write("550 Failed to change directory.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    if (!startswith(canon.string(), this->userclass->gethomedir())) {
      this->Write("550 Permission denied.\r\n");
      return FAIL;
    }
  }
  this->Write("250 Directory successfully changed.\r\n");
  this->SetCwd(canon.string());
  return SUCCESS;
}
int connection::HandleDELE(void) {
  std::string requestedPath;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path pp;
  std::filesystem::path hd;
  std::filesystem::path plist;
  std::filesystem::path token;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(DELE)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("550 Delete operation failed.\r\n");
    return FAIL;
  }
  requestedPath = this->command_tokens[1];
  relpath.assign(requestedPath);
  if (!relpath.has_root_path()) {
    requestedPath = this->GetCwd() + "/" + requestedPath;
  }
  fullpath.assign(requestedPath);
  if (!std::filesystem::exists(fullpath)) {
    this->Write("550 Delete operation failed.\r\n");
    return FAIL;
  }
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Delete operation failed.\r\n");
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!std::filesystem::exists(fullpath)) {
    this->Write("550 Delete operation failed.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = fullpath.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == fullpath.end()) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
    }
  }
  plist.assign(userlist);
  token.assign("/tmp/admin/token");
  if (std::filesystem::equivalent(plist, fullpath)) {
    this->Write("550 Permission denied.\r\n");
    this->portset = 0;
    return FAIL;
  }
  try {
    if (std::filesystem::equivalent(token, fullpath)) {
      this->Write("550 Permission denied.\r\n");
      this->portset = 0;
      return FAIL;
    }
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Need access to the file" << std::endl;
    return FAIL;
  }
  try {
    if (!std::filesystem::remove(fullpath)) {
      this->Write("550 Delete operation failed.\r\n");
      return FAIL;
    }
  } catch (const std::exception &e) {
    this->Write("550 Delete operation failed.\r\n");
    return FAIL;
  }
  this->Write("250 Delete operation successful.\r\n");
  return SUCCESS;
}
int connection::HandleEPRT(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(EPRT)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleEPSV(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(EPSV)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleFEAT(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(FEAT)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleHELP(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(HELP)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("214-The following commands are recognized.\r\n");
  this->Write(" ABOR ACCT ALLO APPE CDUP CWD  DELE EPRT EPSV FEAT HELP LIST "
              "MDTM MKD\r\n");
  this->Write(" MODE NLST NOOP OPTS PASS PASV PORT PWD  QUIT REIN REST RETR "
              "RMD  RNFR\r\n");
  this->Write(" RNTO SITE SIZE SMNT STAT STOR STOU STRU SYST TYPE USER XCUP "
              "XCWD XMKD\r\n");
  this->Write(" XPWD XRMD\r\n");
  this->Write("214 Help OK.\r\n");
  return SUCCESS;
}
int connection::HandleLIST(void) {
  std::vector<std::string> listing;
  std::string directory;
  std::string line;
  int listfd = 0;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->portset && !this->pasv) {
    this->Write("425 Use PORT or PASV first.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(LIST)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() <= 1) {
    directory = this->GetCwd();
  } else {
    directory = this->command_tokens[1];
  }
  listing = get_detailed_listing(directory, this->GetCwd());
  int max = 1024;
  int index = 0;
  char *data = static_cast<char *>(calloc(max, 1));
  if (!data) {
    this->Write("550 Failed to get listing.\r\n");
    return FAIL;
  }
  for (std::vector<std::string>::iterator it = listing.begin();
       it != listing.end(); ++it) {
    line = (*it);
    line += "\r\n";
    if (index + line.size() > max) {
      data = static_cast<char *>(realloc(data, max * 2));
      if (!data) {
        this->Write("550 Failed to get listing.\r\n");
        return FAIL;
      }
      max *= 2;
    }
    memcpy(data + index, line.c_str(), line.size());
    index += line.size();
  }
  this->Write("150 Here comes the directory listing.\r\n");
  if (this->pasv) {
    srvr->AddSingleShotRETR(this->pasvfd, this->fd, data, index, "",
                            "226 Directory send OK.\r\n", 1);
    this->pasv = 0;
  } else {
    listfd = connect_socket(this->targetip, this->targetport);
    this->portset = 0;
    if (listfd < 0) {
      this->Write("425 Failed to establish connection.\r\n");
      return FAIL;
    }
    srvr->AddSingleShotRETR(listfd, this->fd, data, index, "",
                            "226 Directory send OK.\r\n", 0);
  }
  this->portset = 0;
  this->pasv = 0;
  return SUCCESS;
}
int connection::HandleMDTM(void) {
  std::string requestedFile;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path token;
  std::filesystem::path plist;
  std::time_t cftime;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(MDTM)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("550 Could not get file modification time.\r\n");
    return FAIL;
  }
  requestedFile = this->command_tokens[1];
  relpath.assign(requestedFile);
  if (!relpath.has_root_path()) {
    requestedFile = this->GetCwd() + "/" + requestedFile;
  }
  fullpath = std::filesystem::relative(requestedFile, this->GetCwd());
  try {
    canon.assign(this->GetCwd() + "/" + fullpath.string());
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    this->Write("550 Could not get file modification time.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    if (!startswith(canon.string(), this->userclass->gethomedir())) {
      this->Write("550 Permission denied.\r\n");
      return FAIL;
    }
  }
  if (!std::filesystem::exists(canon)) {
    this->Write("550 Could not get file modification time.\r\n");
    return FAIL;
  }
  plist.assign(userlist);
  token.assign("/tmp/admin/token");
  if (std::filesystem::equivalent(plist, canon) &&
      !this->userclass->checkperm(CANRETRPLIST)) {
    this->Write("550 Permission denied.\r\n");
    this->portset = 0;
    return FAIL;
  }
  try {
    if (std::filesystem::equivalent(token, canon) &&
        !this->userclass->checkperm(CANRETRTOKEN)) {
      this->Write("550 Permission denied.\r\n");
      this->portset = 0;
      return FAIL;
    }
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Need access to the file" << std::endl;
    return FAIL;
  }
  std::filesystem::file_time_type mtime =
      std::filesystem::last_write_time(canon);
  struct tm *ptm = NULL;
  cftime = decltype(mtime)::clock::to_time_t(mtime);
  ptm = gmtime(&cftime);
  std::string dts = std::to_string(ptm->tm_year + 1900);
  if (ptm->tm_mon + 1 < 10) {
    dts += "0";
  }
  dts += std::to_string(ptm->tm_mon + 1);
  if (ptm->tm_mday < 10) {
    dts += "0";
  }
  dts += std::to_string(ptm->tm_mday);
  if (ptm->tm_hour < 10) {
    dts += "0";
  }
  dts += std::to_string(ptm->tm_hour);
  if (ptm->tm_min < 10) {
    dts += "0";
  }
  dts += std::to_string(ptm->tm_min);
  if (ptm->tm_sec < 10) {
    dts += "0";
  }
  dts += std::to_string(ptm->tm_sec);
  this->Write("213 " + dts + "\r\n");
  return SUCCESS;
}
int connection::HandleMKD(void) {
  std::string requestedPath;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path pp;
  std::filesystem::path hd;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(MKD)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("550 Create directory operation failed.\r\n");
    return FAIL;
  }
  requestedPath = this->command_tokens[1];
  relpath.assign(requestedPath);
  if (!relpath.has_root_path()) {
    requestedPath = this->GetCwd() + "/" + requestedPath;
  }
  fullpath.assign(requestedPath);
  if (std::filesystem::exists(fullpath)) {
    this->Write("550 Create directory operation failed.\r\n");
    return FAIL;
  }
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Create directory operation failed.\r\n");
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = fullpath.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == fullpath.end()) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
    }
  }
  try {
    if (!std::filesystem::create_directory(fullpath)) {
      this->Write("550 Create directory operation failed.\r\n");
      return FAIL;
    }
  } catch (const std::exception &e) {
    this->Write("550 Create directory operation failed.\r\n");
    return FAIL;
  }
  this->Write("257 \"" + fullpath.string() + "\" created\r\n");
  return SUCCESS;
}
int connection::HandleMODE(void) {
  std::string structure;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    this->portset = 0;
    return FAIL;
  }
  if (!this->userclass->checkperm(MODE)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("504 Bad MODE command.\r\n");
    return FAIL;
  }
  structure = this->command_tokens[1];
  if (structure == "S" || structure == "s") {
    this->SetStructType('F');
    this->Write("200 Mode set to S.\r\n");
    return SUCCESS;
  } else {
    this->Write("504 Bad MODE command.\r\n");
  }
  return SUCCESS;
}
int connection::HandleNLST(void) {
  std::vector<std::string> listing;
  std::string directory;
  int nlstfd;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(NLST)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (!this->portset && !this->pasv) {
    this->Write("425 Use PORT or PASV first.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() <= 1) {
    directory = this->GetCwd();
  } else {
    directory = this->command_tokens[1];
  }
  listing = get_listing(directory, this->GetCwd());
  int max = 1024;
  int index = 0;
  char *data = static_cast<char *>(calloc(max, 1));
  if (!data) {
    this->Write("550 Failed to get listing.\r\n");
    return FAIL;
  }
  for (std::vector<std::string>::iterator it = listing.begin();
       it != listing.end(); ++it) {
    (*it) += "\r\n";
    if (index + (*it).size() > max) {
      data = static_cast<char *>(realloc(data, max * 2));
      if (!data) {
        this->Write("550 Failed to get listing.\r\n");
        return FAIL;
      }
      max *= 2;
    }
    memcpy(data + index, (*it).c_str(), (*it).size());
    index += (*it).size();
  }
  this->Write("150 Here comes the directory listing.\r\n");
  if (this->pasv) {
    srvr->AddSingleShotRETR(this->pasvfd, this->fd, data, index, "",
                            "226 Directory send OK.\r\n", 1);
    this->pasv = 0;
  } else {
    nlstfd = connect_socket(this->targetip, this->targetport);
    this->portset = 0;
    if (nlstfd < 0) {
      this->Write("425 Failed to establish connection.\r\n");
      return FAIL;
    }
    srvr->AddSingleShotRETR(nlstfd, this->fd, data, index, "",
                            "226 Directory send OK.\r\n", 0);
  }
  this->portset = 0;
  this->pasv = 0;
  return SUCCESS;
}
int connection::HandleNOOP(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(NOOP)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("200 NOOP ok.\r\n");
  return SUCCESS;
}
int connection::HandleOPTS(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(OPTS)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandlePASV(void) {
  struct ifaddrs *ifap = NULL;
  struct ifaddrs *walker = NULL;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(PASV)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (getifaddrs(&ifap) < 0) {
    this->Write("500 Command failed\r\n");
    return FAIL;
  }
  if (this->pasv) {
    this->Write("500 Command failed\r\n");
    return FAIL;
  }
  walker = ifap;
  while (walker) {
    if (walker->ifa_flags & IFF_LOOPBACK && !this->islocal) {
      walker = walker->ifa_next;
      continue;
    } else if (walker->ifa_addr->sa_family == AF_INET6) {
      walker = walker->ifa_next;
      continue;
    } else if (!(walker->ifa_flags & IFF_RUNNING)) {
      walker = walker->ifa_next;
      continue;
    } else if (((struct sockaddr_in *)walker->ifa_addr)->sin_addr.s_addr < 15) {
      walker = walker->ifa_next;
      continue;
    } else {
      break;
    }
  }
  if (walker == NULL) {
    this->Write("510 Command failed\r\n");
    freeifaddrs(ifap);
    return FAIL;
  }
  char *s = NULL;
  struct sockaddr_in *addr_in = (struct sockaddr_in *)walker->ifa_addr;
  s = static_cast<char *>(malloc(INET_ADDRSTRLEN));
  if (!s) {
    this->Write("511 Command failed\r\n");
    freeifaddrs(ifap);
    return FAIL;
  }
  inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
  for (int i = 0; s[i]; i++) {
    if (s[i] == '.') {
      s[i] = ',';
    }
  }
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
    this->Write("512 Command failed\r\n");
    freeifaddrs(ifap);
    free(s);
    return FAIL;
  }
  unsigned int seed;
  read(fd, &seed, sizeof(seed));
  close(fd);
  srand(seed);
  int port = (rand() % 10000) + 2048;
  int top = port / 256;
  int bot = port % 256;
  std::string pasv = s;
  pasv += "," + std::to_string(top) + "," + std::to_string(bot);
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    this->Write("513 Command failed\r\n");
    freeifaddrs(ifap);
    free(s);
    return FAIL;
  }
  struct sockaddr_in srvr;
  memset(&srvr, 0, sizeof(struct sockaddr_in));
  srvr.sin_family = AF_INET;
  srvr.sin_addr.s_addr = addr_in->sin_addr.s_addr;
  srvr.sin_port = htons(port);
  if (bind(fd, (struct sockaddr *)&srvr, sizeof(srvr)) < 0) {
    this->Write("514 Command failed\r\n");
    close(fd);
    free(s);
    freeifaddrs(ifap);
    return FAIL;
  }
  if (listen(fd, 2) < 0) {
    this->Write("515 Command failed\r\n");
    close(fd);
    free(s);
    freeifaddrs(ifap);
    return FAIL;
  }
  this->Write("227 Entering Passive Mode (" + pasv + ").\r\n");
  freeifaddrs(ifap);
  free(s);
  this->portset = 0;
  this->pasv = 1;
  this->pasvfd = fd;
  return SUCCESS;
}
int connection::HandlePORT(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(PORT)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("500 Illegal PORT command.\r\n");
    return FAIL;
  }
  std::vector<std::string> cmdtoks = cpptok(this->command_tokens[1], ',');
  if (cmdtoks.size() != 6) {
    this->Write("500 Illegal PORT command.\r\n");
    return FAIL;
  }
  for (std::vector<std::string>::iterator it = cmdtoks.begin();
       it != cmdtoks.end(); it++) {
    if (!onlydigits(*it)) {
      this->Write("500 Illegal PORT command.\r\n");
      return FAIL;
    }
  }
  if (this->pasv) {
    close(this->pasvfd);
    this->pasv = 0;
  }
  std::string ip =
      cmdtoks[0] + "." + cmdtoks[1] + "." + cmdtoks[2] + "." + cmdtoks[3];
  int port = (std::stoi(cmdtoks[4]) * 256) + std::stoi(cmdtoks[5]);
  this->targetip = ip;
  this->targetport = port;
  this->portset = 1;
  this->Write("200 PORT command successful. Consider using PASV.\r\n");
  return SUCCESS;
}
int connection::HandleSITE(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(SITE)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleSIZE(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(SIZE)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleSTAT(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(STAT)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("520 Not implemented\r\n");
  return SUCCESS;
}
int connection::HandleSYST(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(SYST)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("215 UNIX Type: L8\r\n");
  return SUCCESS;
}
int connection::HandlePWD(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(PWD)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("257 \"" + this->GetCwd() + "\" is the current directory\r\n");
  return SUCCESS;
}
int connection::HandleREIN(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(REIN)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->command_in_queue = 0;
  this->userset = 0;
  this->passset = 0;
  this->mode = 1;
  this->authd = 0;
  this->Command = "";
  this->command_tokens.clear();
  if (userclass) {
    this->userclass = NULL;
  }
  this->user = "";
  this->pass = "";
  this->rnfr = "";
  this->rnfrset = 0;
  this->restvalue = 0;
  this->cwd = "";
  this->structtype = 'f';
  this->portset = 0;
  this->targetip = "";
  this->targetport = 0;
  this->Write("220 Ok\r\n");
  if (this->pasv) {
    close(this->pasvfd);
    this->pasv = 0;
  }
  islocal = 0;
  return SUCCESS;
}
int connection::HandleREST(void) {
  unsigned int rv = 0;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(REST)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() >= 2) {
    rv = std::stoi(this->command_tokens[1]);
  }
  this->Write("350 Restart position accepted (" + std::to_string(rv) +
              ").\r\n");
  this->SetRestValue(rv);
  return SUCCESS;
}
int connection::HandleRETR(void) {
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path relpath;
  std::filesystem::path plist;
  std::filesystem::path token;
  std::filesystem::path hd;
  std::string line;
  std::string requestedFile;
  std::ifstream f;
  int retrfd;
  int filesize;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(REST)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (!this->portset && !this->pasv) {
    this->Write("425 Use PORT or PASV first.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() < 2) {
    this->Write("550 Failed to open file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  requestedFile = this->command_tokens[1];
  relpath.assign(requestedFile);
  if (!relpath.has_root_path()) {
    requestedFile = this->GetCwd() + "/" + requestedFile;
  }
  fullpath = std::filesystem::relative(requestedFile, this->GetCwd());
  try {
    canon.assign(this->GetCwd() + "/" + fullpath.string());
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    this->Write("550 Failed to open file.\r\n");
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    } else {
      this->portset = 0;
    }
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = canon.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == canon.end()) {
        this->Write("550 Failed to open file.\r\n");
        if (this->pasv) {
          close(this->pasvfd);
          this->pasv = 0;
        } else {
          this->portset = 0;
        }
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        if (this->pasv) {
          close(this->pasvfd);
          this->pasv = 0;
        } else {
          this->portset = 0;
        }
        return FAIL;
      }
    }
  }
  if (!std::filesystem::exists(canon)) {
    this->Write("550 Failed to open file.\r\n");
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    } else {
      this->portset = 0;
    }
    return FAIL;
  }
  plist.assign(userlist);
  token.assign("/tmp/admin/token");
  if (std::filesystem::equivalent(plist, canon) &&
      !this->userclass->checkperm(CANRETRPLIST)) {
    this->Write("550 Permission denied.\r\n");
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    } else {
      this->portset = 0;
    }
    return FAIL;
  }
  try {
    if (std::filesystem::equivalent(token, canon) &&
        !this->userclass->checkperm(CANRETRTOKEN)) {
      this->Write("550 Permission denied.\r\n");
      if (this->pasv) {
        close(this->pasvfd);
        this->pasv = 0;
      } else {
        this->portset = 0;
      }
      return FAIL;
    }
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Need access to the file" << std::endl;
    return FAIL;
  }
  try {
    f.open(canon.string(), std::ios::in | std::ios::binary);
  } catch (const std::exception &e) {
    this->Write("550 Failed to open file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  f.seekg(0, f.end);
  filesize = f.tellg();
  if (this->GetRestValue() > filesize) {
    this->Write("550 Failed to read file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  f.seekg(this->GetRestValue(), f.beg);
  int buffer_index = 0;
  char *data = static_cast<char *>(calloc(1, filesize));
  if (*data) {
    this->Write("550 Failed to read file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  if (this->GetMode() == ASCII) {
    while (f.peek() != std::ifstream::traits_type::eof()) {
      std::getline(f, line);
      if (endswith(line, "\r")) {
        line += "\n";
      } else {
        line += "\r\n";
      }
      if (buffer_index + line.size() > filesize) {
        data = static_cast<char *>(realloc(data, filesize * 2));
        if (!data) {
          this->Write("550 Failed to read file.\r\n");
          this->portset = 0;
          if (this->pasv) {
            close(this->pasvfd);
            this->pasv = 0;
          }
          return FAIL;
        }
        filesize *= 2;
      }
      memcpy(data + buffer_index, line.c_str(), line.size());
      buffer_index += line.size();
    }
    this->Write("150 Opening ASCII mode data connection for " + canon.string() +
                " (" + std::to_string(buffer_index) + ") bytes.\r\n");
  } else if (this->GetMode() == BINARY) {
    f.read(data, filesize);
    buffer_index = f.gcount();
    this->Write("150 Opening BINARY mode data connection for " +
                canon.string() + " (" + std::to_string(buffer_index) +
                ") bytes.\r\n");
  }
  if (this->pasv) {
    srvr->AddSingleShotRETR(this->pasvfd, this->fd, data, buffer_index, "",
                            "226 Transfer complete.\r\n", 1);
    this->pasv = 0;
  } else {
    retrfd = connect_socket(this->targetip, this->targetport);
    this->portset = 0;
    if (retrfd < 0) {
      this->Write("425 Failed to establish connection.\r\n");
      return FAIL;
    }
    srvr->AddSingleShotRETR(retrfd, this->fd, data, buffer_index, "",
                            "226 Transfer complete.\r\n", 0);
  }
  this->portset = 0;
  this->pasv = 0;
  return SUCCESS;
}
int connection::HandleRMD(void) {
  std::string requestedPath;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path pp;
  std::filesystem::path hd;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(RMD)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  requestedPath = this->command_tokens[1];
  relpath.assign(requestedPath);
  if (!relpath.has_root_path()) {
    requestedPath = this->GetCwd() + "/" + requestedPath;
  }
  fullpath.assign(requestedPath);
  if (!std::filesystem::exists(fullpath)) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!std::filesystem::exists(fullpath)) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  if (!std::filesystem::is_directory(fullpath)) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = fullpath.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == fullpath.end()) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
    }
  }
  try {
    if (!std::filesystem::remove(fullpath)) {
      this->Write("550 Remove directory operation failed.\r\n");
      return FAIL;
    }
  } catch (const std::exception &e) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  this->Write("250 Remove directory operation successful.\r\n");
  return SUCCESS;
}
int connection::HandleRNFR(void) {
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path relpath;
  std::filesystem::path plist;
  std::filesystem::path token;
  std::string line;
  std::string requestedFile;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(RNFR)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() < 2) {
    this->Write("550 RNFR command failed.\r\n");
    return FAIL;
  }
  requestedFile = this->command_tokens[1];
  relpath.assign(requestedFile);
  if (!relpath.has_root_path()) {
    requestedFile = this->GetCwd() + "/" + requestedFile;
  }
  fullpath = std::filesystem::relative(requestedFile, this->GetCwd());
  try {
    canon.assign(this->GetCwd() + "/" + fullpath.string());
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    this->Write("550 RNFR command failed.\r\n");
    this->portset = 0;
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    if (!startswith(canon.string(), this->userclass->gethomedir())) {
      this->Write("550 Permission denied.\r\n");
      this->portset = 0;
      return FAIL;
    }
  }
  if (!std::filesystem::exists(canon)) {
    this->Write("550 RNFR command failed.\r\n");
    this->portset = 0;
    return FAIL;
  }
  plist.assign(userlist);
  token.assign("/tmp/admin/token");
  if (std::filesystem::equivalent(plist, canon)) {
    this->Write("550 Permission denied.\r\n");
    this->portset = 0;
    return FAIL;
  }
  try {
    if (std::filesystem::equivalent(token, canon)) {
      this->Write("550 Permission denied.\r\n");
      this->portset = 0;
      return FAIL;
    }
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Need access to the file" << std::endl;
    return FAIL;
  }
  this->rnfr = canon.string();
  this->rnfrset = 1;
  this->Write("350 Ready for RNTO.\r\n");
  return SUCCESS;
}
int connection::HandleRNTO(void) {
  std::filesystem::path fullpath;
  std::filesystem::path relpath;
  std::filesystem::path plist;
  std::filesystem::path token;
  std::filesystem::path from;
  std::filesystem::path pp;
  std::string line;
  std::string requestedFile;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(RNTO)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (!this->rnfrset) {
    this->Write("503 RNFR required first.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() < 2) {
    this->Write("559 Rename failed.\r\n");
    return FAIL;
  }
  requestedFile = this->command_tokens[1];
  relpath.assign(requestedFile);
  if (!relpath.has_root_path()) {
    requestedFile = this->GetCwd() + "/" + requestedFile;
  }
  fullpath.assign(requestedFile);
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Rename failed.\r\n");
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    if (!startswith(fullpath.string(), this->userclass->gethomedir())) {
      this->Write("550 Permission denied.\r\n");
      this->rnfrset = 0;
      return FAIL;
    }
  }
  if (std::filesystem::exists(fullpath)) {
    this->Write("557 Rename failed.\r\n");
    this->rnfrset = 0;
    return FAIL;
  }
  plist.assign(userlist);
  token.assign("/tmp/admin/token");
  if (std::filesystem::equivalent(plist, fullpath)) {
    this->Write("550 Permission denied.\r\n");
    this->rnfrset = 0;
    return FAIL;
  }
  try {
    if (std::filesystem::equivalent(token, fullpath)) {
      this->Write("550 Permission denied.\r\n");
      this->rnfrset = 0;
      return FAIL;
    }
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Need access to the file" << std::endl;
    return FAIL;
  }
  from.assign(this->rnfr);
  try {
    std::filesystem::rename(from, fullpath);
  } catch (const std::exception &e) {
    this->Write("551 Rename failed.\r\n");
    this->rnfrset = 0;
    return FAIL;
  }
  this->rnfrset = 0;
  this->Write("250 Rename successful.\r\n");
  return SUCCESS;
}
int connection::HandleSTOR(void) {
  std::string dest;
  std::string storeFile;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path hd;
  std::filesystem::path pp;
  std::filesystem::path plist;
  std::filesystem::path token;
  int storfd;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(STOR)) {
    this->Write("550 Permission denied.\r\n");
    this->portset = 0;
    return FAIL;
  }
  if (!this->portset && !this->pasv) {
    this->Write("425 Use PORT or PASV first.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() < 2) {
    this->Write("553 Could not create file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  storeFile = this->command_tokens[1];
  if (endswith(storeFile, "/")) {
    this->Write("553 Could not create file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  relpath.assign(storeFile);
  if (!relpath.has_root_path()) {
    storeFile = this->GetCwd() + "/" + storeFile;
  }
  fullpath.assign(storeFile);
  if (std::filesystem::exists(fullpath)) {
    if (!std::filesystem::is_regular_file(fullpath)) {
      this->Write("550 Failed to open file.\r\n");
      this->portset = 0;
      if (this->pasv) {
        close(this->pasvfd);
        this->pasv = 0;
      }
      return FAIL;
    }
  }
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Failed to open file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = fullpath.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == fullpath.end()) {
        this->Write("550 Failed to open file.\r\n");
        this->portset = 0;
        if (this->pasv) {
          close(this->pasvfd);
          this->pasv = 0;
        }
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        this->portset = 0;
        if (this->pasv) {
          close(this->pasvfd);
          this->pasv = 0;
        }
        return FAIL;
      }
    }
  }
  plist.assign(userlist);
  token.assign("/tmp/admin/token");
  if (std::filesystem::equivalent(plist, fullpath)) {
    this->Write("550 Permission denied.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  try {
    if (std::filesystem::equivalent(token, fullpath)) {
      this->Write("550 Permission denied.\r\n");
      this->portset = 0;
      if (this->pasv) {
        close(this->pasvfd);
        this->pasv = 0;
      }
      return FAIL;
    }
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Need access to the file" << std::endl;
    return FAIL;
  }
  this->Write("150 Ok to send data.\r\n");
  if (this->pasv) {
    srvr->AddSingleShotRETR(this->pasvfd, this->fd, NULL, 0, fullpath.string(),
                            "226 Transfer complete.\r\n", 1);
    this->pasv = 0;
  } else {
    storfd = connect_socket(this->targetip, this->targetport);
    this->portset = 0;
    if (storfd < 0) {
      this->Write("425 Failed to establish connection.\r\n");
      return FAIL;
    }
    srvr->AddSingleShotRETR(storfd, this->fd, NULL, 0, fullpath.string(),
                            "226 Transfer complete.\r\n", 0);
  }
  this->pasv = 0;
  this->portset = 0;
  return SUCCESS;
}
int connection::HandleSTOU(void) {
  std::string dest;
  std::string storeFile;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path hd;
  std::filesystem::path pp;
  std::filesystem::path plist;
  std::filesystem::path token;
  char *tfile = NULL;
  int stoufd;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(STOU)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (!this->portset & !this->pasv) {
    this->Write("425 Use PORT or PASV first.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() < 2) {
    tfile = tempnam(this->userclass->gethomedir().c_str(), "stou");
    if (!tfile) {
      this->Write("553 Could not create file.\r\n");
      this->portset = 0;
      if (this->pasv) {
        close(this->pasvfd);
        this->pasv = 0;
      }
      return FAIL;
    }
    storeFile = tfile;
  } else {
    storeFile = this->command_tokens[1];
  }
  if (endswith(storeFile, "/")) {
    this->Write("553 Could not create file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  relpath.assign(storeFile);
  if (!relpath.has_root_path()) {
    storeFile = this->GetCwd() + "/" + storeFile;
  }
  fullpath.assign(storeFile);
  if (std::filesystem::exists(fullpath)) {
    if (!std::filesystem::is_regular_file(fullpath)) {
      this->Write("550 Failed to open file.\r\n");
      this->portset = 0;
      if (this->pasv) {
        close(this->pasvfd);
        this->pasv = 0;
      }
      return FAIL;
    }
  }
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Failed to open file.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = fullpath.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == fullpath.end()) {
        this->Write("550 Failed to open file.\r\n");
        this->portset = 0;
        if (this->pasv) {
          close(this->pasvfd);
          this->pasv = 0;
        }
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        this->portset = 0;
        if (this->pasv) {
          close(this->pasvfd);
          this->pasv = 0;
        }
        return FAIL;
      }
    }
  }
  plist.assign(userlist);
  token.assign("/tmp/admin/token");
  if (std::filesystem::equivalent(plist, fullpath)) {
    this->Write("550 Permission denied.\r\n");
    this->portset = 0;
    if (this->pasv) {
      close(this->pasvfd);
      this->pasv = 0;
    }
    return FAIL;
  }
  try {
    if (std::filesystem::equivalent(token, fullpath)) {
      this->Write("550 Permission denied.\r\n");
      this->portset = 0;
      if (this->pasv) {
        close(this->pasvfd);
        this->pasv = 0;
      }
      return FAIL;
    }
  } catch (const std::exception &e) {
    std::cout << "[ERROR] Need access to the file" << std::endl;
    return FAIL;
  }
  this->Write("150 FILE: " + fullpath.string() + "\r\n");
  if (this->pasv) {
    srvr->AddSingleShotRETR(this->pasvfd, this->fd, NULL, 0, fullpath.string(),
                            "226 Transfer complete.\r\n", 1);
    this->pasv = 0;
  } else {
    stoufd = connect_socket(this->targetip, this->targetport);
    this->portset = 0;
    if (stoufd < 0) {
      this->Write("425 Failed to establish connection.\r\n");
      return FAIL;
    }
    srvr->AddSingleShotRETR(stoufd, this->fd, NULL, 0, fullpath.string(),
                            "226 Transfer complete.\r\n", 0);
  }
  this->pasv = 0;
  this->portset = 0;
  return SUCCESS;
}
int connection::HandleSTRU(void) {
  std::string structure;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    this->portset = 0;
    return FAIL;
  }
  if (!this->userclass->checkperm(STRU)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("540 Bad STRU command.\r\n");
    return FAIL;
  }
  structure = this->command_tokens[1];
  if (structure == "F" || structure == "f") {
    this->SetStructType('F');
    this->Write("200 Structure set to F.\r\n");
    return SUCCESS;
  } else {
    this->Write("540 Bad STRU command.\r\n");
  }
  return SUCCESS;
}
int connection::HandleTYPE(void) {
  std::string type;
  std::string subtype;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    this->portset = 0;
    return FAIL;
  }
  if (!this->userclass->checkperm(TYPE)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() < 2) {
    this->Write("500 Unrecognised TYPE command.\r\n");
    return FAIL;
  }
  type = this->command_tokens[1];
  if (type == "A" || type == "a") {
    if (this->command_tokens.size() == 3) {
      subtype = this->command_tokens[2];
      if (subtype != "N" && subtype != "n") {
        this->Write("500 Unrecognised TYPE command.\r\n");
        return FAIL;
      }
    } else if (this->command_tokens.size() > 3) {
      this->Write("500 Unrecognised TYPE command.\r\n");
      return FAIL;
    }
    this->SetMode(ASCII);
    this->Write("200 Switching to ASCII mode.\r\n");
    return SUCCESS;
  } else if (type == "I" || type == "i") {
    if (this->command_tokens.size() > 2) {
      this->Write("500 Unrecognised TYPE command.\r\n");
      return FAIL;
    }
    this->SetMode(BINARY);
    this->Write("200 Switching to BINARY mode.\r\n");
    return SUCCESS;
  } else if (type == "L" || type == "l") {
    if (this->command_tokens.size() == 3) {
      subtype = this->command_tokens[2];
      if (subtype != "8") {
        this->Write("500 Unrecognised TYPE command.\r\n");
        return FAIL;
      }
    } else if (this->command_tokens.size() > 3) {
      this->Write("500 Unrecognised TYPE command.\r\n");
      return FAIL;
    }
    this->SetMode(BINARY);
    this->Write("200 Switching to BINARY mode.\r\n");
    return SUCCESS;
  } else {
    this->Write("500 Unrecognised TYPE command.\r\n");
  }
  return FAIL;
}
int connection::HandleXCUP(void) {
  std::string newDir;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(XCUP)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  try {
    canon.assign(this->GetCwd());
    canon = std::filesystem::canonical(canon);
    canon = canon.parent_path();
  } catch (const std::exception &e) {
    this->Write("550 Failed to change directory.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    if (!startswith(canon.string(), this->userclass->gethomedir())) {
      this->Write("550 Permission denied.\r\n");
      return FAIL;
    }
  }
  this->Write("250 Directory successfully changed.\r\n");
  this->SetCwd(canon.string());
  return SUCCESS;
}
int connection::HandleXCWD(void) {
  std::string newDir;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(XCWD)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("550 Failed to change directory.\r\n");
    return FAIL;
  }
  newDir = this->command_tokens[1];
  relpath.assign(newDir);
  if (!relpath.has_root_path()) {
    newDir = this->GetCwd() + "/" + newDir;
  }
  fullpath = std::filesystem::relative(newDir, this->GetCwd());
  try {
    canon.assign(this->GetCwd() + "/" + fullpath.string());
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    this->Write("550 Failed to change directory.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    if (!startswith(canon.string(), this->userclass->gethomedir())) {
      this->Write("550 Permission denied.\r\n");
      return FAIL;
    }
  }
  this->Write("250 Directory successfully changed.\r\n");
  this->SetCwd(canon.string());
  return SUCCESS;
}
int connection::HandleXMKD(void) {
  std::string requestedPath;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path pp;
  std::filesystem::path hd;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(XMKD)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("550 Create directory operation failed.\r\n");
    return FAIL;
  }
  requestedPath = this->command_tokens[1];
  relpath.assign(requestedPath);
  if (!relpath.has_root_path()) {
    requestedPath = this->GetCwd() + "/" + requestedPath;
  }
  fullpath.assign(requestedPath);
  if (std::filesystem::exists(fullpath)) {
    this->Write("550 Create directory operation failed.\r\n");
    return FAIL;
  }
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Create directory operation failed.\r\n");
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = fullpath.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == fullpath.end()) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
    }
  }
  try {
    if (!std::filesystem::create_directory(fullpath)) {
      this->Write("550 Create directory operation failed.\r\n");
      return FAIL;
    }
  } catch (const std::exception &e) {
    this->Write("550 Create directory operation failed.\r\n");
    return FAIL;
  }
  this->Write("257 \"" + fullpath.string() + "\" created\r\n");
  return SUCCESS;
}
int connection::HandleXPWD(void) {
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(XPWD)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  this->Write("257 \"" + this->GetCwd() + "\" is the current directory\r\n");
  return SUCCESS;
}
int connection::HandleXRMD(void) {
  std::string requestedPath;
  std::filesystem::path relpath;
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::filesystem::path pp;
  std::filesystem::path hd;
  if (this->authd == 0) {
    this->Write("530 Please login with USER and PASS.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(XRMD)) {
    this->Write("550 Permission denied.\r\n");
    return FAIL;
  }
  if (this->command_tokens.size() != 2) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  requestedPath = this->command_tokens[1];
  relpath.assign(requestedPath);
  if (!relpath.has_root_path()) {
    requestedPath = this->GetCwd() + "/" + requestedPath;
  }
  fullpath.assign(requestedPath);
  if (!std::filesystem::exists(fullpath)) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  pp.assign(fullpath.parent_path());
  if (!std::filesystem::exists(pp)) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  } else {
    pp = std::filesystem::canonical(pp);
    fullpath.assign(pp.string() + "/" + fullpath.filename().string());
  }
  if (!std::filesystem::exists(fullpath)) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  if (!std::filesystem::is_directory(fullpath)) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  if (!this->userclass->checkperm(CANRETROUTSIDE)) {
    hd.assign(this->userclass->gethomedir());
    for (std::filesystem::path::iterator fp_it = fullpath.begin(),
                                         hd_it = hd.begin();
         hd_it != hd.end(); hd_it++, fp_it++) {
      if (fp_it == fullpath.end()) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
      if ((*fp_it).string().compare((*hd_it).string()) != 0) {
        this->Write("550 Permission denied.\r\n");
        return FAIL;
      }
    }
  }
  try {
    if (!std::filesystem::remove(fullpath)) {
      this->Write("550 Remove directory operation failed.\r\n");
      return FAIL;
    }
  } catch (const std::exception &e) {
    this->Write("550 Remove directory operation failed.\r\n");
    return FAIL;
  }
  this->Write("250 Remove directory operation successful.\r\n");
  return SUCCESS;
}
int connection::HandleCommand(void) {
  this->command_in_queue = 0;
  if (this->command_tokens[0].compare("USER") == 0) {
    if (this->command_tokens.size() >= 2) {
      this->SetUser(this->command_tokens[1]);
      this->Write("331 Please specify the password\r\n");
    }
  } else if (this->command_tokens[0].compare("PASS") == 0) {
    if (!this->userset) {
      this->Write("503 Login with USER first.\r\n");
      return SUCCESS;
    }
    if (this->command_tokens.size() >= 2) {
      std::string pass =
          std::accumulate(this->command_tokens.begin() + 1,
                          this->command_tokens.end(), std::string(""));
      this->SetPass(pass);
    }
    this->CheckAuth();
  } else if (this->command_tokens[0].compare("ABOR") == 0) {
    return this->HandleABOR();
  } else if (this->command_tokens[0].compare("ACCT") == 0) {
    return this->HandleACCT();
  } else if (this->command_tokens[0].compare("ALLO") == 0) {
    return this->HandleALLO();
  } else if (this->command_tokens[0].compare("APPE") == 0) {
    return this->HandleAPPE();
  } else if (this->command_tokens[0].compare("CDUP") == 0) {
    return this->HandleCDUP();
  } else if (this->command_tokens[0].compare("CWD") == 0) {
    return this->HandleCWD();
  } else if (this->command_tokens[0].compare("DELE") == 0) {
    return this->HandleDELE();
  } else if (this->command_tokens[0].compare("EPRT") == 0) {
    return this->HandleEPRT();
  } else if (this->command_tokens[0].compare("EPSV") == 0) {
    return this->HandleEPSV();
  } else if (this->command_tokens[0].compare("FEAT") == 0) {
    return this->HandleFEAT();
  } else if (this->command_tokens[0].compare("HELP") == 0) {
    return this->HandleHELP();
  } else if (this->command_tokens[0].compare("LIST") == 0) {
    return this->HandleLIST();
  } else if (this->command_tokens[0].compare("MDTM") == 0) {
    return this->HandleMDTM();
  } else if (this->command_tokens[0].compare("MKD") == 0) {
    return this->HandleMKD();
  } else if (this->command_tokens[0].compare("MODE") == 0) {
    return this->HandleMODE();
  } else if (this->command_tokens[0].compare("NLST") == 0) {
    return this->HandleNLST();
  } else if (this->command_tokens[0].compare("NOOP") == 0) {
    return this->HandleNOOP();
  } else if (this->command_tokens[0].compare("OPTS") == 0) {
    return this->HandleOPTS();
  } else if (this->command_tokens[0].compare("PASV") == 0) {
    return this->HandlePASV();
  } else if (this->command_tokens[0].compare("PORT") == 0) {
    return this->HandlePORT();
  } else if (this->command_tokens[0].compare("PWD") == 0) {
    return this->HandlePWD();
  } else if (this->command_tokens[0].compare("QUIT") == 0) {
    this->Write("221 Goodbye\r\n");
    this->needs_shutdown = 1;
    return SUCCESS;
  } else if (this->command_tokens[0].compare("REIN") == 0) {
    return this->HandleREIN();
  } else if (this->command_tokens[0].compare("REST") == 0) {
    return this->HandleREST();
  } else if (this->command_tokens[0].compare("RETR") == 0) {
    return this->HandleRETR();
  } else if (this->command_tokens[0].compare("RNFR") == 0) {
    return this->HandleRNFR();
  } else if (this->command_tokens[0].compare("RNTO") == 0) {
    return this->HandleRNTO();
  } else if (this->command_tokens[0].compare("RMD") == 0) {
    return this->HandleRMD();
  } else if (this->command_tokens[0].compare("SITE") == 0) {
    return this->HandleSITE();
  } else if (this->command_tokens[0].compare("SIZE") == 0) {
    return this->HandleSIZE();
  } else if (this->command_tokens[0].compare("STAT") == 0) {
    return this->HandleSTAT();
  } else if (this->command_tokens[0].compare("STOR") == 0) {
    return this->HandleSTOR();
  } else if (this->command_tokens[0].compare("STOU") == 0) {
    return this->HandleSTOU();
  } else if (this->command_tokens[0].compare("STRU") == 0) {
    return this->HandleSTRU();
  } else if (this->command_tokens[0].compare("SYST") == 0) {
    return this->HandleSYST();
  } else if (this->command_tokens[0].compare("TYPE") == 0) {
    return this->HandleTYPE();
  } else if (this->command_tokens[0].compare("XCUP") == 0) {
    return this->HandleXCUP();
  } else if (this->command_tokens[0].compare("XCWD") == 0) {
    return this->HandleXCWD();
  } else if (this->command_tokens[0].compare("XMKD") == 0) {
    return this->HandleXMKD();
  } else if (this->command_tokens[0].compare("XPWD") == 0) {
    return this->HandleXPWD();
  } else if (this->command_tokens[0].compare("XRMD") == 0) {
    return this->HandleXRMD();
  } else {
    this->Write("500 Unknown Command.\r\n");
  }
  return SUCCESS;
}
int connection::ParseCommand(void) {
  this->command_tokens = cpptok(this->Command, ' ');
  if (this->command_tokens.empty()) {
    this->command_in_queue = 0;
    return FAIL;
  }
  return SUCCESS;
}
void connection::setNeedsAccept(int flag) {
  this->needs_accept = flag;
  return;
}
void connection::setDataLength(int length) {
  this->datalength = length;
  return;
}
void connection::setDataToSend(char *data) {
  this->data_to_send = data;
  return;
}
void connection::setSingleUse(int flag) {
  this->single_use = flag;
  return;
}
int connection::CommandWaiting(void) { return this->command_in_queue; }
int connection::NeedsRemoval(void) { return this->needs_shutdown; }
connection::connection(int fd, sockaddr_in *clnt) {
  int devfd = 0;
  this->fd = fd;
  memset(&(this->clnt), 0, sizeof(sockaddr_in));
  if (clnt) {
    memcpy(&this->clnt, clnt, sizeof(sockaddr_in));
  }
  this->needs_shutdown = 0;
  this->command_in_queue = 0;
  this->userset = 0;
  this->passset = 0;
  this->mode = 0;
  this->authd = 0;
  this->restvalue = 0;
  this->islocal = 0;
  this->portset = 0;
  this->pasv = 0;
  this->needs_accept = 0;
  this->data_to_send = NULL;
  ;
  this->datalength = 0;
  this->single_use = 0;
  devfd = open("/dev/urandom", O_RDONLY);
  if (devfd < 0) {
    this->clientid = 0xdeadbeef;
  }
  read(devfd, &this->clientid, sizeof(this->clientid));
  close(devfd);
  return;
}
int connection::Write(std::string data) {
  if (data.empty()) {
    return 0;
  }
  int result = write(this->fd, data.c_str(), data.size());
  return result;
}
int connection::Read() {
  char c = 0;
  std::string cmd = "";
  int result = 0;
  while (!endswith(cmd, "\x0a")) {
    result = read(this->fd, &c, 1);
    if (result < 0) {
      this->needs_shutdown = 1;
      return result;
    } else if (result == 0) {
      this->needs_shutdown = 1;
      return result;
    }
    cmd += c;
  }
  this->Command = std::regex_replace(cmd, std::regex("[\r\n]+$"), "");
  this->command_in_queue = 1;
  return result;
}
int connection::get_fd(void) { return this->fd; }
int connection::SendBanner(void) {
  std::string banner = "220 chessFTPd (.0.0.0.0.0.0.1)\r\n";
  write(this->fd, banner.c_str(), banner.size());
  return SUCCESS;
}