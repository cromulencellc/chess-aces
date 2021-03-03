#include "client.hpp"
#include "email.hpp"
#include "mailbox.hpp"
#include "server.hpp"
#include <filesystem>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
int Server::HandleCommand(std::vector<std::string> tokens, Client *c) {
  std::string uid;
  std::string command;
  if (c == NULL) {
    return FAIL;
  }
  if (tokens.size() < 1) {
    c->AddLineToResponseQueue(
        "* NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  uid = tokens[0];
  if (tokens.size() < 2) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  command = tokens[1];
  if (cppstrncasecmp(command, "capability")) {
    return this->HandleCapability(tokens, c, uid);
  } else if (cppstrncasecmp(command, "noop")) {
    return this->HandleNoop(tokens, c, uid);
  } else if (cppstrncasecmp(command, "logout")) {
    return this->HandleLogout(tokens, c, uid);
  } else if (cppstrncasecmp(command, "authenticate")) {
    if (this->HandleAuthenticate(tokens, c, uid) == FAIL) {
      c->SetNeedsShutdown(1);
    }
    return FAIL;
  } else if (cppstrncasecmp(command, "login")) {
    if (this->HandleLogin(tokens, c, uid) == FAIL) {
      c->SetNeedsShutdown(1);
    }
    return FAIL;
  } else if (cppstrncasecmp(command, "list")) {
    return this->HandleList(tokens, c, uid);
  } else if (cppstrncasecmp(command, "create")) {
    return this->HandleCreate(tokens, c, uid);
  } else if (cppstrncasecmp(command, "delete")) {
    return this->HandleDelete(tokens, c, uid);
  } else if (cppstrncasecmp(command, "rename")) {
    return this->HandleRename(tokens, c, uid);
  } else if (cppstrncasecmp(command, "select")) {
    return this->HandleSelect(tokens, c, uid);
  } else if (cppstrncasecmp(command, "close")) {
    return this->HandleClose(tokens, c, uid);
  } else if (cppstrncasecmp(command, "fetch")) {
    return this->HandleFetch(tokens, c, uid);
  } else if (cppstrncasecmp(command, "expunge")) {
    return this->HandleExpunge(tokens, c, uid);
  } else if (cppstrncasecmp(command, "status")) {
    return this->HandleStatus(tokens, c, uid);
  } else if (cppstrncasecmp(command, "copy")) {
    return this->HandleCopy(tokens, c, uid);
  } else if (cppstrncasecmp(command, "store")) {
    return this->HandleStore(tokens, c, uid);
  } else {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  return SUCCESS;
}
int Server::InitLoggedInUser(Client *c) {
  struct stat st;
  std::string md;
  struct passwd *pwd = NULL;
  struct group *grp = NULL;
  if (!c) {
    return FAIL;
  }
  pwd = getpwnam(c->GetUsername().c_str());
  if (!pwd) {
    c->AddLineToResponseQueue("* BYE [ALERT] Fatal error: Invalid user");
    return FAIL;
  }
  grp = getgrnam(c->GetUsername().c_str());
  if (!grp) {
    c->AddLineToResponseQueue("* BYE [ALERT] Fatal error: Invalid user");
    return FAIL;
  }
  md = "/home/" + c->GetUsername() + "/Maildir";
  if (stat(md.c_str(), &st) != 0) {
    c->AddLineToResponseQueue("* BYE [ALERT] Fatal error: No such file or "
                              "directory: No such file or directory");
    return FAIL;
  }
  c->SetUid(pwd->pw_uid);
  c->SetGid(grp->gr_gid);
  c->SetMailDir(md);
  return SUCCESS;
}
int Server::HandleCapability(std::vector<std::string> tokens, Client *c,
                             std::string uid) {
  if (c == NULL) {
    return FAIL;
  }
  c->AddLineToResponseQueue("* CAPABILITY IMAP4rev1 AUTH=PLAIN\r\n");
  c->AddLineToResponseQueue(uid + " OK CAPABILITY completed\r\n");
  return SUCCESS;
}
int Server::HandleNoop(std::vector<std::string> tokens, Client *c,
                       std::string uid) {
  if (c == NULL) {
    return FAIL;
  }
  c->AddLineToResponseQueue(uid + " OK NOOP completed\r\n");
  return SUCCESS;
}
int Server::HandleLogout(std::vector<std::string> tokens, Client *c,
                         std::string uid) {
  if (c == NULL) {
    return FAIL;
  }
  c->Write("* BYE CHESS-IMAP server shutting down\r\n");
  c->Write(uid + " OK LOGOUT completed\r\n");
  c->SetNeedsShutdown(1);
  return SUCCESS;
}
int Server::HandleAuthenticate(std::vector<std::string> tokens, Client *c,
                               std::string uid) {
  std::string authtype = "";
  std::string authline;
  int result;
  char decoded[512];
  char *user = NULL;
  char *pass = NULL;
  memset(decoded, 0, 512);
  if (!c) {
    return FAIL;
  }
  if (tokens.size() < 3) {
    c->AddLineToResponseQueue(uid + " BAD no authentication type\r\n");
    return FAIL;
  }
  authtype = tokens[2];
  if (cppstrncasecmp(authtype, "plain")) {
    c->Write("+\r\n");
    if (c->Read() <= 0) {
      return FAIL;
    }
    c->SetNeedsResolving(0);
    authline = c->GetQueue();
    c->ClearQueue();
    result = base64decode(authline.c_str(), decoded, authline.size(), 511);
    if (result == 0) {
      c->AddLineToResponseQueue(uid + " NO Login failed.\r\n");
      return FAIL;
    }
    if (decoded[0] != '\x00') {
      c->AddLineToResponseQueue(uid + " NO Login failed.\r\n");
      return FAIL;
    }
    user = decoded + 1;
    pass = user + strlen(user) + 1;
    if (decoded + result <= pass) {
      c->AddLineToResponseQueue(uid + " NO Login failed.\r\n");
      return FAIL;
    }
    c->SetUsername(user);
    if (this->InitLoggedInUser(c) == SUCCESS) {
      c->AddLineToResponseQueue(uid + " OK LOGIN Ok.\r\n");
      c->SetLoggedIn(1);
      return SUCCESS;
    } else {
      return FAIL;
    }
  } else {
    c->AddLineToResponseQueue(uid + " BAD unknown authentication type: " +
                              authtype + "\r\n");
    return FAIL;
  }
  return SUCCESS;
}
int Server::HandleLogin(std::vector<std::string> tokens, Client *c,
                        std::string uid) {
  std::string user;
  std::string pass;
  if (tokens.size() < 4) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  user = tokens[2];
  pass = tokens[3];
  c->SetUsername(user);
  if (this->InitLoggedInUser(c) == SUCCESS) {
    c->AddLineToResponseQueue(uid + " OK LOGIN Ok.\r\n");
    c->SetLoggedIn(1);
    return SUCCESS;
  } else {
    return FAIL;
  }
  return SUCCESS;
}
int Server::HandleList(std::vector<std::string> tokens, Client *c,
                       std::string uid) {
  mailbox *root = NULL;
  mailbox *parent = NULL;
  std::smatch m;
  std::string line;
  std::regex root_reg;
  std::string rreg;
  std::regex children_reg;
  std::string rchild;
  if (!c) {
    return FAIL;
  }
  if (c->GetLoggedIn() == 0) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  root = parse_mailbox(c->GetMailDir(), 0, "");
  if (root == NULL) {
    c->AddLineToResponseQueue(uid + " OK LIST completed\r\n");
    return FAIL;
  }
  if (tokens.size() != 4) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    free_mailbox(root);
    return FAIL;
  }
  rreg = tokens[2];
  rchild = tokens[3];
  if (rreg[0] == '"' && rreg[rreg.size() - 1] == '"' && rreg.size() >= 2) {
    rreg.erase(0, 1);
    rreg.erase(rreg.size() - 1, 1);
    try {
      root_reg = std::regex(rreg, std::regex_constants::icase);
    } catch (const std::exception &e) {
      free_mailbox(root);
      c->AddLineToResponseQueue(uid + " OK LIST completed\r\n");
      return FAIL;
    }
    parent = find_root_from_regex(root, root_reg);
    if (!parent) {
      free_mailbox(root);
      c->AddLineToResponseQueue(uid + " OK LIST completed\r\n");
      return FAIL;
    }
  } else {
    parent = find_root_from_string(root, rreg);
    if (!parent) {
      c->AddLineToResponseQueue(uid + " OK LIST completed\r\n");
      free_mailbox(root);
      return FAIL;
    }
  }
  if ((rchild[0] == '"' && rchild[rchild.size() - 1] == '"' &&
       rchild.size() >= 2) ||
      rchild == "*") {
    if (rchild != "*") {
      rchild.erase(0, 1);
      rchild.erase(rchild.size() - 1, 1);
    } else {
      rchild = "";
    }
    try {
      children_reg = std::regex(rchild, std::regex_constants::icase);
    } catch (const std::exception &e) {
      c->AddLineToResponseQueue(uid + " OK LIST completed\r\n");
      free_mailbox(root);
      return FAIL;
    }
    this->ListMaildirs(c, parent, children_reg);
    c->AddLineToResponseQueue(uid + " OK LIST completed\r\n");
  } else {
    this->ListMaildirs(c, parent, rchild);
    c->AddLineToResponseQueue(uid + " OK LIST completed\r\n");
  }
  free_mailbox(root);
  return SUCCESS;
}
int Server::ListMaildirs(Client *c, mailbox *parent, std::regex r) {
  std::smatch m;
  std::string line;
  if (!c) {
    return FAIL;
  }
  if (!parent) {
    return FAIL;
  }
  line = "* LIST (";
  if (parent->total_children) {
    line += "\\HasChildren) ";
  } else {
    line += "\\HasNoChildren) ";
  }
  line += "\".\" \"" + parent->fullname + "\"\r\n";
  c->AddLineToResponseQueue(line);
  for (int i = 0; i < parent->total_children; i++) {
    if (std::regex_search(parent->children[i]->fullname, m, r)) {
      this->ListMaildirs(c, parent->children[i], r);
    }
  }
  return SUCCESS;
}
int Server::ListMaildirs(Client *c, mailbox *parent, std::string s) {
  std::smatch m;
  std::string line;
  if (!c) {
    return FAIL;
  }
  if (!parent) {
    return FAIL;
  }
  line = "* LIST (";
  if (parent->total_children) {
    line += "\\HasChildren) ";
  } else {
    line += "\\HasNoChildren) ";
  }
  line += "\".\" \"" + parent->fullname + "\"\r\n";
  c->AddLineToResponseQueue(line);
  for (int i = 0; i < parent->total_children; i++) {
    if (parent->children[i]->fullname == s) {
      this->ListMaildirs(c, parent->children[i], s);
    }
  }
  return SUCCESS;
}
int Server::HandleCreate(std::vector<std::string> tokens, Client *c,
                         std::string uid) {
  std::string parent = "";
  std::string child = "";
  std::string newmb = "";
  size_t index = 0;
  mailbox *mb = NULL;
  mailbox *ch = NULL;
  mailbox *pnt = NULL;
  if (!c) {
    return FAIL;
  }
  if (c->GetLoggedIn() == 0) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() < 3) {
    c->AddLineToResponseQueue(
        "* NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  for (auto it = tokens.begin() + 2; it != tokens.end(); ++it) {
    newmb += *it + " ";
  }
  rtrim(newmb);
  index = newmb.find_first_of('.');
  if (index == std::string::npos) {
    c->AddLineToResponseQueue(uid + " NO Invalid mailbox name.\r\n");
    return FAIL;
  }
  parent = newmb.substr(0, index);
  child = newmb.substr(index);
  if (cppstrncasecmp(parent, "inbox") == false) {
    c->AddLineToResponseQueue(uid + " NO Invalid mailbox name.\r\n");
    return FAIL;
  }
  mb = parse_mailbox(c->GetMailDir(), 0, "");
  if (mb == NULL) {
    c->AddLineToResponseQueue(uid + "NO Error in IMAP user maildir.\r\n");
    return FAIL;
  }
  pnt = mb;
  ch = get_child_by_name(mb, child);
  while (ch != NULL) {
    index = child.find_first_of('.', 1);
    if (index == std::string::npos) {
      c->AddLineToResponseQueue(uid + " NO Cannot create this folder.\r\n");
      free_mailbox(mb);
      return FAIL;
    } else {
      child = child.substr(index);
    }
    pnt = ch;
    ch = get_child_by_name(pnt, child);
  }
  if (pnt->max_children <= pnt->total_children) {
    c->AddLineToResponseQueue(uid + " NO Cannot create this folder.\r\n");
    free_mailbox(mb);
    return FAIL;
  }
  if (std::filesystem::create_directory(
          std::filesystem::path(pnt->fullpath + "/" + child)) == false) {
    c->AddLineToResponseQueue(uid + " NO Cannot create this folder.\r\n");
    free_mailbox(mb);
    return FAIL;
  }
  if (std::filesystem::create_directory(std::filesystem::path(
          pnt->fullpath + "/" + child + "/cur")) == false) {
    c->AddLineToResponseQueue(uid + " NO Cannot create this folder.\r\n");
    std::filesystem::remove_all(
        std::filesystem::path(pnt->fullpath + "/" + child));
    free_mailbox(mb);
    return FAIL;
  }
  if (std::filesystem::create_directory(std::filesystem::path(
          pnt->fullpath + "/" + child + "/tmp")) == false) {
    c->AddLineToResponseQueue(uid + " NO Cannot create this folder.\r\n");
    std::filesystem::remove_all(
        std::filesystem::path(pnt->fullpath + "/" + child));
    free_mailbox(mb);
    return FAIL;
  }
  if (std::filesystem::create_directory(std::filesystem::path(
          pnt->fullpath + "/" + child + "/new")) == false) {
    c->AddLineToResponseQueue(uid + " NO Cannot create this folder.\r\n");
    std::filesystem::remove_all(
        std::filesystem::path(pnt->fullpath + "/" + child));
    free_mailbox(mb);
    return FAIL;
  }
  std::string z = pnt->fullpath + "/" + child;
  chown(z.c_str(), c->GetUid(), c->GetGid());
  z = pnt->fullpath + "/" + child + "/cur";
  chown(z.c_str(), c->GetUid(), c->GetGid());
  z = pnt->fullpath + "/" + child + "/tmp";
  chown(z.c_str(), c->GetUid(), c->GetGid());
  z = pnt->fullpath + "/" + child + "/new";
  chown(z.c_str(), c->GetUid(), c->GetGid());
  c->AddLineToResponseQueue(uid + " OK \"" + newmb + "\" created.\r\n");
  free_mailbox(mb);
  return SUCCESS;
}
int Server::HandleStatus(std::vector<std::string> tokens, Client *c,
                         std::string uid) {
  std::string mb_name;
  std::string data_req;
  std::vector<std::string> cur;
  std::vector<std::string> newb;
  std::vector<std::string> attribs;
  std::string line;
  int counter;
  mailbox *mb;
  mailbox *root;
  if (!c) {
    return FAIL;
  }
  if (c->GetLoggedIn() == 0) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() < 4) {
    c->AddLineToResponseQueue(
        "* NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  mb_name = tokens[2];
  for (auto it = tokens.begin() + 3; it != tokens.end(); ++it) {
    data_req += *it + " ";
  }
  rtrim(data_req);
  if (data_req[0] == '(') {
    if (data_req[data_req.size() - 1] == ')') {
      data_req = data_req.substr(1, data_req.size() - 2);
    } else {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
  }
  attribs = tokenize_line(data_req, ' ');
  root = parse_mailbox(c->GetMailDir(), 0, "");
  if (!root) {
    c->AddLineToResponseQueue(uid + " NO Error in IMAP server.\r\n");
    return FAIL;
  }
  mb = get_mailbox_by_fullname(root, mb_name);
  if (!mb) {
    free_mailbox(root);
    c->AddLineToResponseQueue(
        uid + " NO Mailbox does not exist, or must be subscribed to.\r\n");
    return FAIL;
  }
  cur = get_file_list(mb->fullpath + "/cur");
  newb = get_file_list(mb->fullpath + "/new");
  line = "* STATUS " + mb_name + " (";
  for (auto it = attribs.begin(); it != attribs.end(); ++it) {
    if (cppstrncasecmp(*it, "messages") == true) {
      line += "MESSAGES " + std::to_string(cur.size() + newb.size()) + " ";
    } else if (cppstrncasecmp(*it, "recent") == true) {
      line += "RECENT " + std::to_string(cur.size()) + " ";
    } else if (cppstrncasecmp(*it, "unseen") == true) {
      counter = 0;
      for (auto m_it = cur.begin(); m_it != cur.end(); ++m_it) {
        if (hasflag(*m_it, 'S') == false) {
          counter++;
        }
      }
      for (auto m_it = newb.begin(); m_it != newb.end(); ++m_it) {
        if (hasflag(*m_it, 'S') == false) {
          counter++;
        }
      }
      line += "UNSEEN " + std::to_string(counter) + " ";
    } else if (cppstrncasecmp(*it, "passed") == true) {
      counter = 0;
      for (auto m_it = cur.begin(); m_it != cur.end(); ++m_it) {
        if (hasflag(*m_it, 'P') == true) {
          counter++;
        }
      }
      for (auto m_it = newb.begin(); m_it != newb.end(); ++m_it) {
        if (hasflag(*m_it, 'P') == true) {
          counter++;
        }
      }
      line += "PASSED " + std::to_string(counter) + " ";
    } else if (cppstrncasecmp(*it, "replied") == true) {
      counter = 0;
      for (auto m_it = cur.begin(); m_it != cur.end(); ++m_it) {
        if (hasflag(*m_it, 'R') == true) {
          counter++;
        }
      }
      for (auto m_it = newb.begin(); m_it != newb.end(); ++m_it) {
        if (hasflag(*m_it, 'R') == true) {
          counter++;
        }
      }
      line += "REPLIED " + std::to_string(counter) + " ";
    } else if (cppstrncasecmp(*it, "seen") == true) {
      counter = 0;
      for (auto m_it = cur.begin(); m_it != cur.end(); ++m_it) {
        if (hasflag(*m_it, 'S') == true) {
          counter++;
        }
      }
      for (auto m_it = newb.begin(); m_it != newb.end(); ++m_it) {
        if (hasflag(*m_it, 'S') == true) {
          counter++;
        }
      }
      line += "SEEN " + std::to_string(counter) + " ";
    } else if (cppstrncasecmp(*it, "trashed") == true) {
      counter = 0;
      for (auto m_it = cur.begin(); m_it != cur.end(); ++m_it) {
        if (hasflag(*m_it, 'T') == true) {
          counter++;
        }
      }
      for (auto m_it = newb.begin(); m_it != newb.end(); ++m_it) {
        if (hasflag(*m_it, 'T') == true) {
          counter++;
        }
      }
      line += "TRASHED " + std::to_string(counter) + " ";
    } else if (cppstrncasecmp(*it, "draft") == true) {
      counter = 0;
      for (auto m_it = cur.begin(); m_it != cur.end(); ++m_it) {
        if (hasflag(*m_it, 'D') == true) {
          counter++;
        }
      }
      for (auto m_it = newb.begin(); m_it != newb.end(); ++m_it) {
        if (hasflag(*m_it, 'D') == true) {
          counter++;
        }
      }
      line += "DRAFT " + std::to_string(counter) + " ";
    } else if (cppstrncasecmp(*it, "flagged") == true) {
      counter = 0;
      for (auto m_it = cur.begin(); m_it != cur.end(); ++m_it) {
        if (hasflag(*m_it, 'F') == true) {
          counter++;
        }
      }
      for (auto m_it = newb.begin(); m_it != newb.end(); ++m_it) {
        if (hasflag(*m_it, 'F') == true) {
          counter++;
        }
      }
      line += "FLAGGED " + std::to_string(counter) + " ";
    } else {
      free_mailbox(root);
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
  }
  free_mailbox(root);
  rtrim(line);
  line += ")";
  c->AddLineToResponseQueue("* STATUS " + mb_name + " " + line + "\r\n");
  c->AddLineToResponseQueue(uid + " OK STATUS completed\r\n");
  return SUCCESS;
}
int Server::HandleDelete(std::vector<std::string> tokens, Client *c,
                         std::string uid) {
  std::string parent;
  std::string child;
  std::string delmb;
  size_t index;
  mailbox *mb = NULL;
  mailbox *ch = NULL;
  mailbox *pnt = NULL;
  mailbox *temp = NULL;
  if (!c) {
    return FAIL;
  }
  if (c->GetLoggedIn() == 0) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() != 3) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  delmb = tokens[2];
  if (cppstrncasecmp(delmb, "inbox") == true) {
    c->AddLineToResponseQueue(uid + " NO Cannot delete this folder.\r\n");
    return FAIL;
  }
  index = delmb.find_first_of('.');
  if (index == std::string::npos) {
    c->AddLineToResponseQueue(uid + " NO Invalid mailbox name.\r\n");
    return FAIL;
  }
  parent = delmb.substr(0, index);
  child = delmb.substr(index);
  mb = parse_mailbox(c->GetMailDir(), 0, "");
  if (mb == NULL) {
    c->AddLineToResponseQueue(uid + "NO Error in IMAP user maildir.\r\n");
    return FAIL;
  }
  pnt = mb;
  ch = get_child_by_name(mb, child);
  while (ch != NULL) {
    index = child.find_first_of('.', 1);
    if (index == std::string::npos) {
      break;
    } else {
      child = child.substr(index);
    }
    pnt = ch;
    ch = get_child_by_name(pnt, child);
  }
  if (ch == NULL) {
    c->AddLineToResponseQueue(
        uid + " NO Mailbox does not exist, or must be subscribed to.\r\n");
    free_mailbox(mb);
    return FAIL;
  }
  if (ch->total_children) {
    c->AddLineToResponseQueue(uid + " NO Name \"" + ch->fullname +
                              "\" has inferior hierarchical names\r\n");
    free_mailbox(mb);
    return FAIL;
  }
  temp = c->GetSelected();
  if (temp) {
    if (cppstrncasecmp(ch->name, temp->name) == true) {
      c->AddLineToResponseQueue(uid + " NO Cannot delete selected\r\n");
      free_mailbox(mb);
      return FAIL;
    }
  }
  std::filesystem::remove_all(std::filesystem::path(ch->fullpath));
  remove_child_by_name(pnt, ch->name);
  c->AddLineToResponseQueue(uid + " OK Folder deleted.\r\n");
  free_mailbox(mb);
  return SUCCESS;
}
int Server::HandleRename(std::vector<std::string> tokens, Client *c,
                         std::string uid) {
  std::string initial;
  std::string final;
  std::string dest_folder;
  mailbox *mb = NULL;
  mailbox *root = NULL;
  mailbox *t = NULL;
  std::filesystem::path base;
  std::filesystem::path dest;
  if (!c) {
    return FAIL;
  }
  if (c->GetLoggedIn() == 0) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() != 4) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  initial = tokens[2];
  final = tokens[3];
  if (cppstrncasecmp(initial, "inbox") == true) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (shared_parent(initial, final) == false) {
    c->AddLineToResponseQueue(uid + " NO Invalid hierarchy change.\r\n");
    return FAIL;
  }
  root = parse_mailbox(c->GetMailDir(), 0, "");
  if (!root) {
    c->AddLineToResponseQueue(uid + " NO Error in IMAP server.\r\n");
    return FAIL;
  }
  mb = get_mailbox_by_fullname(root, initial);
  if (mb == NULL) {
    c->AddLineToResponseQueue(uid + " NO Access denied for RENAME on " +
                              initial + "\r\n");
    free_mailbox(root);
    return FAIL;
  }
  t = get_mailbox_by_fullname(root, final);
  if (t) {
    c->AddLineToResponseQueue(uid + " NO RENAME failed: File exists\r\n");
    free_mailbox(root);
    return FAIL;
  }
  dest_folder = final.substr((final.find_last_of('.') != std::string::npos)
                                 ? final.find_last_of('.')
                                 : 0);
  base = std::filesystem::path(mb->fullpath);
  dest = std::filesystem::path(base.parent_path().string() + "/" + dest_folder);
  std::filesystem::rename(base, dest);
  free_mailbox(root);
  c->AddLineToResponseQueue(uid + " OK Folder renamed.\r\n");
  return SUCCESS;
}
int Server::HandleSelect(std::vector<std::string> tokens, Client *c,
                         std::string uid) {
  mailbox *mb = NULL;
  mailbox *root = NULL;
  std::vector<std::string> cur;
  std::vector<std::string> newb;
  if (!c) {
    return FAIL;
  }
  if (!c->GetLoggedIn()) {
    return FAIL;
  }
  if (tokens.size() != 3) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  root = parse_mailbox(c->GetMailDir(), 0, "");
  if (!root) {
    c->AddLineToResponseQueue(uid + " NO Error in IMAP server.\r\n");
    return FAIL;
  }
  mb = find_root_from_string(root, tokens[2]);
  if (!mb) {
    free_mailbox(root);
    c->AddLineToResponseQueue(
        uid + " NO Mailbox does not exist, or must be subscribed to.\r\n");
    return FAIL;
  }
  cur = get_file_list(mb->fullpath + "/cur");
  newb = get_file_list(mb->fullpath + "/new");
  c->AddLineToResponseQueue(
      "* FLAGS (\\Draft \\Answered \\Flagged \\Deleted \\Seen \\Recent)\r\n");
  c->AddLineToResponseQueue("* OK [PERMANENTFLAGS (\\* \\Draft \\Answered "
                            "\\Flagged \\Deleted \\Seen)] Limited\r\n");
  c->AddLineToResponseQueue("* " + std::to_string(cur.size() + newb.size()) +
                            " EXISTS\r\n");
  c->AddLineToResponseQueue("* " + std::to_string(newb.size()) + " RECENT\r\n");
  c->AddLineToResponseQueue(uid + " OK [READ-WRITE] Ok\r\n");
  if (c->GetSelected() != NULL) {
    free_mailbox(c->GetSelected());
  }
  c->SetSelected(deep_copy_mb(mb));
  free_mailbox(root);
  std::string b;
  std::string a;
  int index = 0;
  for (auto it = newb.begin(); it != newb.end(); ++it) {
    b = *it;
    a = *it;
    index = b.find("/new");
    if (index == std::string::npos) {
      continue;
    }
    b.replace(index, 4, "/cur");
    b += ":2,S";
    std::filesystem::rename(a, b);
  }
  return SUCCESS;
}
int Server::HandleExpunge(std::vector<std::string> tokens, Client *c,
                          std::string uid) {
  std::vector<std::string> cur;
  std::vector<std::string> newb;
  std::string fn;
  std::string flags;
  std::filesystem::path p;
  mailbox *mb = NULL;
  int index;
  if (!c->GetLoggedIn()) {
    return FAIL;
  }
  mb = c->GetSelected();
  if (mb == NULL) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() != 2) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  cur = get_file_list(mb->fullpath + "/cur");
  newb = get_file_list(mb->fullpath + "/new");
  for (auto it = cur.begin(); it != cur.end(); ++it) {
    fn = *it;
    index = fn.find(":2,");
    if (index == std::string::npos) {
      continue;
    }
    flags = fn.substr(index + 3);
    for (auto c_it = flags.begin(); c_it != flags.end(); ++c_it) {
      if (*c_it == 'T' || *c_it == 't') {
        p = std::filesystem::path(fn);
        std::filesystem::remove(p);
        c->AddLineToResponseQueue(
            "* " + std::to_string((it - cur.begin()) + 1) + " EXPUNGE\r\n");
        break;
      }
    }
  }
  for (auto it = newb.begin(); it != newb.end(); ++it) {
    fn = *it;
    index = fn.find(":2,");
    if (index == std::string::npos) {
      continue;
    }
    flags = fn.substr(index + 3);
    for (auto c_it = flags.begin(); c_it != flags.end(); ++c_it) {
      if (*c_it == 'T' || *c_it == 't') {
        p = std::filesystem::path(fn);
        std::filesystem::remove(p);
        c->AddLineToResponseQueue(
            "* " + std::to_string((it - newb.begin()) + 1 + cur.size()) +
            " EXPUNGE\r\n");
        break;
      }
    }
  }
  c->AddLineToResponseQueue(uid + " OK EXPUNGE completed\r\n");
  return SUCCESS;
}
int Server::HandleClose(std::vector<std::string> tokens, Client *c,
                        std::string uid) {
  std::vector<std::string> cur;
  std::vector<std::string> newb;
  std::string fn;
  int index;
  std::string flags;
  std::filesystem::path p;
  mailbox *mb;
  if (!c->GetLoggedIn()) {
    return FAIL;
  }
  mb = c->GetSelected();
  if (mb == NULL) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() != 2) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  cur = get_file_list(mb->fullpath + "/cur");
  newb = get_file_list(mb->fullpath + "/new");
  for (auto it = cur.begin(); it != cur.end(); ++it) {
    fn = *it;
    index = fn.find(":2,");
    if (index == std::string::npos) {
      continue;
    }
    flags = fn.substr(index + 3);
    for (auto c_it = flags.begin(); c_it != flags.end(); ++c_it) {
      if (*c_it == 'T' || *c_it == 't') {
        p = std::filesystem::path(fn);
        std::filesystem::remove(p);
        break;
      }
    }
  }
  for (auto it = newb.begin(); it != newb.end(); ++it) {
    fn = *it;
    index = fn.find(":2,");
    if (index == std::string::npos) {
      continue;
    }
    flags = fn.substr(index + 3);
    for (auto c_it = flags.begin(); c_it != flags.end(); ++c_it) {
      if (*c_it == 'T' || *c_it == 't') {
        p = std::filesystem::path(fn);
        std::filesystem::remove(p);
        break;
      }
    }
  }
  free_mailbox(mb);
  c->SetSelected(NULL);
  c->AddLineToResponseQueue(uid + " OK CLOSE completed\r\n");
  return SUCCESS;
}
int Server::HandleFetch(std::vector<std::string> tokens, Client *c,
                        std::string uid) {
  std::vector<std::string> cur;
  std::vector<std::string> newb;
  mailbox *mb = NULL;
  std::string sequence;
  std::vector<std::string> t;
  std::string a;
  std::string line;
  std::vector<std::string> attribs;
  std::vector<std::string> body_tokens;
  int attrib_start_index = 0;
  int attrib_end_index = 0;
  int bracket_depth = 0;
  int paren_depth = 0;
  int start;
  int end;
  if (!c) {
    return FAIL;
  }
  if (!c->GetLoggedIn()) {
    return FAIL;
  }
  mb = c->GetSelected();
  if (mb == NULL) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() < 4) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  sequence = tokens[2];
  cur = get_file_list(mb->fullpath + "/cur");
  newb = get_file_list(mb->fullpath + "/new");
  if (sequence.find(':') != std::string::npos) {
    t = tokenize_line(sequence, ':');
    if (t.size() != 2) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (!isnumber(t[0])) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (!isnumber(t[1]) && t[1] != "*") {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    start = std::stoi(t[0]);
    if (t[1] == "*") {
      end = cur.size() + newb.size();
    } else {
      end = std::stoi(t[1]);
    }
    if (start < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (end < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (end < start) {
      c->AddLineToResponseQueue(uid + " OK FETCH completed.\r\n");
      return FAIL;
    }
    if (start == 0 || end == 0) {
      c->AddLineToResponseQueue("* NO Invalid message sequence number: 0\r\n");
      c->AddLineToResponseQueue(uid + " OK FETCH completed.\r\n");
      return FAIL;
    }
  } else {
    if (!isnumber(sequence)) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    start = end = std::stoi(sequence);
    if (start < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (start == 0) {
      c->AddLineToResponseQueue("* NO Invalid message sequence number: 0\r\n");
      c->AddLineToResponseQueue(uid + " OK FETCH completed.\r\n");
      return FAIL;
    }
  }
  if (end > cur.size() + newb.size()) {
    c->AddLineToResponseQueue("* NO Invalid message sequence number: " +
                              std::to_string(end) + "\r\n");
    c->AddLineToResponseQueue(uid + " OK FETCH completed.\r\n");
    return FAIL;
  }
  for (auto it = newb.begin(); it != newb.end(); ++it) {
    cur.push_back(*it);
  }
  for (auto it = tokens.begin() + 3; it != tokens.end(); ++it) {
    a += *it + " ";
  }
  rtrim(a);
  if (a[0] == '(') {
    if (a[a.size() - 1] == ')') {
      a.erase(0, 1);
      a.erase(a.size() - 1, 1);
    } else {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
  }
  attrib_start_index = 0;
  std::string new_attrib = "";
  a += ' ';
  for (auto it = a.begin(); it != a.end(); ++it) {
    if (*it == '[') {
      bracket_depth++;
      continue;
    } else if (*it == ']') {
      bracket_depth--;
      continue;
    } else if (*it == ' ') {
      if (bracket_depth == 0 && paren_depth == 0) {
        attrib_end_index = it - a.begin();
        new_attrib =
            a.substr(attrib_start_index, attrib_end_index - attrib_start_index);
        attrib_start_index = attrib_end_index + 1;
        if (attrib_start_index == attrib_end_index) {
          continue;
        }
        attribs.push_back(new_attrib);
      }
    } else if (isalnum(*it) || *it == '.') {
      continue;
    } else if (*it == '(') {
      paren_depth++;
      continue;
    } else if (*it == ')') {
      paren_depth--;
      continue;
    } else {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
  }
  if (bracket_depth || paren_depth) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  for (auto it = cur.begin() + (start - 1); it != cur.begin() + (end); ++it) {
    line = "* " + std::to_string((it - cur.begin()) + 1) + " (";
    for (auto attrib_it = attribs.begin(); attrib_it != attribs.end();
         ++attrib_it) {
      if (cppstrncasecmp(*attrib_it, "full") == true) {
        line += get_flag_string(*it) + " ";
        line += get_internal_date_string(*it) + " ";
        line += rfc822(*it) + " ";
        line += envelope(*it) + " ";
      } else if (cppstrncasecmp(*attrib_it, "flags") == true) {
        line += get_flag_string(*it) + " ";
      } else if (cppstrncasecmp(*attrib_it, "internaldate") == true) {
        line += get_internal_date_string(*it) + " ";
      } else if (cppstrncasecmp(*attrib_it, "rfc822") == true) {
        line += rfc822(*it) + " ";
      } else if (cppstrncasecmp(*attrib_it, "rfc822.size") == true) {
        line += rfc822_size(*it) + " ";
      } else if (cppstrncasecmp(*attrib_it, "rfc822.header") == true) {
        line += rfc822_header(*it) + " ";
      } else if (cppstrncasecmp(*attrib_it, "rfc822.text") == true) {
        line += rfc822_text(*it) + " ";
      } else if (cppstrncasecmp(*attrib_it, "envelope") == true) {
        line += envelope(*it) + " ";
      } else if (cppstrncasecmp((*attrib_it).substr(0, 4), "body") == true) {
        if ((*attrib_it).size() == 4) {
          line += "BODY (\"text\" \"plain\" NIL NIL NIL \"8bit\" 13 1)";
        } else {
          a = (*attrib_it).substr(4);
          if (a[0] != '[' || a[a.size() - 1] != ']') {
            c->AddLineToResponseQueue(
                uid + " NO Error in IMAP command received by server.\r\n");
            return FAIL;
          }
          a.erase(0, 1);
          a.erase(a.size() - 1, 1);
          body_tokens = tokenize_line(a, ' ');
          line += body_headers(*it, body_tokens) + " ";
        }
      } else {
        c->AddLineToResponseQueue(
            uid + " NO Error in IMAP command received by server.\r\n");
        return FAIL;
      }
    }
    rtrim(line);
    line += "\n)\r\n";
    c->AddLineToResponseQueue(line);
  }
  c->AddLineToResponseQueue(uid + " OK FETCH completed.\r\n");
  return SUCCESS;
}
int Server::HandleCopy(std::vector<std::string> tokens, Client *c,
                       std::string uid) {
  mailbox *mb = NULL;
  mailbox *root = NULL;
  mailbox *destmb = NULL;
  std::string sequence;
  std::vector<std::string> t;
  std::vector<std::string> cur;
  std::vector<std::string> newb;
  std::string from;
  std::string to;
  int start;
  int end;
  int loopend;
  std::filesystem::path pf;
  std::filesystem::path pt;
  if (!c) {
    return FAIL;
  }
  if (!c->GetLoggedIn()) {
    return FAIL;
  }
  mb = c->GetSelected();
  if (mb == NULL) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() != 4) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  cur = get_file_list(mb->fullpath + "/cur");
  newb = get_file_list(mb->fullpath + "/new");
  sequence = tokens[2];
  if (sequence.find(':') != std::string::npos) {
    t = tokenize_line(sequence, ':');
    if (t.size() != 2) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (!isnumber(t[0])) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (!isnumber(t[1]) && t[1] != "*") {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    start = std::stoi(t[0]);
    if (t[1] == "*") {
      end = cur.size() + newb.size();
    } else {
      end = std::stoi(t[1]);
    }
    if (start < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (end < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (end < start) {
      c->AddLineToResponseQueue(uid + " OK COPY completed.\r\n");
      return FAIL;
    }
    if (start == 0 || end == 0) {
      c->AddLineToResponseQueue("* NO Invalid message sequence number: 0\r\n");
      c->AddLineToResponseQueue(uid + " OK COPY completed.\r\n");
      return FAIL;
    }
  } else {
    if (!isnumber(sequence)) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    start = end = std::stoi(sequence);
    if (start < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (start == 0) {
      c->AddLineToResponseQueue("* NO Invalid message sequence number: 0\r\n");
      c->AddLineToResponseQueue(uid + " OK COPY completed.\r\n");
      return FAIL;
    }
  }
  if (end > cur.size() + newb.size()) {
    c->AddLineToResponseQueue("* NO Invalid message sequence number: " +
                              std::to_string(end) + "\r\n");
    c->AddLineToResponseQueue(uid + " OK COPY completed.\r\n");
    return FAIL;
  }
  root = parse_mailbox(c->GetMailDir(), 0, "");
  if (!root) {
    c->AddLineToResponseQueue(uid + " NO Error in IMAP server.\r\n");
    return FAIL;
  }
  destmb = get_mailbox_by_fullname(root, tokens[3]);
  if (!destmb) {
    free_mailbox(root);
    c->AddLineToResponseQueue(uid +
                              " NO [TRYCREATE] Mailbox does not exist.\r\n");
    return FAIL;
  }
  loopend = (end > cur.size()) ? cur.size() : end;
  if (start < cur.size()) {
    for (int i = start - 1; i < loopend; i++) {
      from = *(cur.begin() + i);
      pf = std::filesystem::path(from);
      pt = std::filesystem::path(destmb->fullpath + "/cur/" +
                                 std::to_string(rand()) + "." +
                                 pf.filename().string());
      try {
        std::filesystem::copy(pf, pt);
      } catch (const std::exception &e) {
        free_mailbox(root);
        c->AddLineToResponseQueue(uid + " NO COPY error. sequence no: " +
                                  std::to_string(i + 1) + "\r\n");
        return FAIL;
      }
    }
  }
  if (end > cur.size()) {
    if (start < cur.size()) {
      start = 0;
    } else {
      start -= cur.size();
    }
    end -= cur.size();
    for (int i = start; i < end; i++) {
      from = *(newb.begin() + i);
      pf = std::filesystem::path(from);
      pt = std::filesystem::path(destmb->fullpath + "/new/" +
                                 std::to_string(rand()) + "." +
                                 pf.filename().string());
      try {
        std::filesystem::copy(pf, pt);
      } catch (const std::exception &e) {
        free_mailbox(root);
        c->AddLineToResponseQueue(uid + " NO COPY error. sequence no: " +
                                  std::to_string(cur.size() + i) + "\r\n");
        return FAIL;
      }
    }
  }
  free_mailbox(root);
  c->AddLineToResponseQueue(uid + " OK COPY completed.\r\n");
  return SUCCESS;
}
int Server::HandleStore(std::vector<std::string> tokens, Client *c,
                        std::string uid) {
  mailbox *mb;
  std::string sequence;
  std::vector<std::string> t;
  std::vector<std::string> cur;
  std::vector<std::string> newb;
  std::string base;
  std::vector<std::string> flag_sets;
  std::vector<char> flags;
  std::string req;
  std::string fr;
  std::string to;
  int start;
  int end;
  int set_type = 0;
  if (!c) {
    return FAIL;
  }
  if (!c->GetLoggedIn()) {
    return FAIL;
  }
  mb = c->GetSelected();
  if (mb == NULL) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  if (tokens.size() < 5) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  cur = get_file_list(mb->fullpath + "/cur");
  newb = get_file_list(mb->fullpath + "/new");
  for (auto it = newb.begin(); it != newb.end(); ++it) {
    cur.push_back(*it);
  }
  sequence = tokens[2];
  if (sequence.find(':') != std::string::npos) {
    t = tokenize_line(sequence, ':');
    if (t.size() != 2) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (!isnumber(t[0])) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (!isnumber(t[1]) && t[1] != "*") {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    start = std::stoi(t[0]);
    if (t[1] == "*") {
      end = cur.size();
    } else {
      end = std::stoi(t[1]);
    }
    if (start < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (end < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (end < start) {
      c->AddLineToResponseQueue(uid + " OK STORE completed.\r\n");
      return FAIL;
    }
    if (start == 0 || end == 0) {
      c->AddLineToResponseQueue("* NO Invalid message sequence number: 0\r\n");
      c->AddLineToResponseQueue(uid + " OK STORE completed.\r\n");
      return FAIL;
    }
  } else {
    if (!isnumber(sequence)) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    start = end = std::stoi(sequence);
    if (start < 0) {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
    if (start == 0) {
      c->AddLineToResponseQueue("* NO Invalid message sequence number: 0\r\n");
      c->AddLineToResponseQueue(uid + " OK STORE completed.\r\n");
      return FAIL;
    }
  }
  if (end > cur.size()) {
    c->AddLineToResponseQueue("* NO Invalid message sequence number: " +
                              std::to_string(end) + "\r\n");
    c->AddLineToResponseQueue(uid + " OK STORE completed.\r\n");
    return FAIL;
  }
  req = tokens[3];
  if (req[0] == '-') {
    set_type = 1;
    req.erase(0, 1);
  } else if (req[0] == '+') {
    set_type = 2;
    req.erase(0, 1);
  }
  if (cppstrncasecmp(req, "flags") == false) {
    c->AddLineToResponseQueue(
        uid + " NO Error in IMAP command received by server.\r\n");
    return FAIL;
  }
  fr = "";
  for (auto it = tokens.begin() + 4; it != tokens.end(); ++it) {
    fr += *it + " ";
  }
  rtrim(fr);
  if (fr[0] == '(') {
    if (fr[fr.size() - 1] == ')') {
      fr.erase(0, 1);
      fr.erase(fr.size() - 1, 1);
    } else {
      c->AddLineToResponseQueue(
          uid + " NO Error in IMAP command received by server.\r\n");
      return FAIL;
    }
  }
  flag_sets = tokenize_line(fr, ' ');
  to = "";
  for (auto it = cur.begin() + start; it != cur.begin() + (end + 1); ++it) {
    if (set_type == 1 || set_type == 2) {
      base = *it;
    } else {
      base = get_base_name(*it);
    }
    for (auto flag_it = flag_sets.begin(); flag_it != flag_sets.end();
         ++flag_it) {
      std::string (*set_flag)(std::string, char);
      if (set_type == 2 || set_type == 0) {
        set_flag = add_flag_to_name;
      } else {
        set_flag = remove_flag_from_name;
      }
      if (cppstrncasecmp(*flag_it, "\\Seen") == true) {
        base = set_flag(base, 'S');
      } else if (cppstrncasecmp(*flag_it, "\\Answered") == true) {
        base = set_flag(base, 'R');
      } else if (cppstrncasecmp(*flag_it, "\\Flagged") == true) {
        base = set_flag(base, 'F');
      } else if (cppstrncasecmp(*flag_it, "\\Deleted") == true) {
        base = set_flag(base, 'T');
      } else if (cppstrncasecmp(*flag_it, "\\Draft") == true) {
        base = set_flag(base, 'D');
      } else {
        c->AddLineToResponseQueue(
            uid + " NO Error in IMAP command received by server.\r\n");
        return FAIL;
      }
    }
    try {
      std::filesystem::rename(std::filesystem::path(*it),
                              std::filesystem::path(base));
    } catch (const std::exception &e) {
      c->AddLineToResponseQueue(uid + " NO Error in IMAP server.\r\n");
      return FAIL;
    }
  }
  c->AddLineToResponseQueue(uid + " OK STORE completed\r\n");
  return SUCCESS;
}