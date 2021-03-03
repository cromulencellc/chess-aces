#ifndef __EMAIL_HPP__
#define __EMAIL_HPP__
#include <iostream>
#include <vector>
typedef struct header {
  std::string field;
  std::string field_data;
} header;
typedef struct message {
  std::vector<std::string> flags;
  std::vector<header *> h;
  int body_length = 0;
  std::string body = "";
  struct message *message = NULL;
} message;
int open_message(std::string fn, char **data);
message *parse_message(char *data);
void free_message(message *m);
std::vector<header *> get_all_headers(message *m);
std::string get_header_by_name(message *m, std::string n);
std::vector<header *> get_headers_by_vector(message *m,
                                            std::vector<std::string> vs);
std::string add_flag_to_name(std::string name, char f);
std::string remove_flag_from_name(std::string name, char f);
std::string get_base_name(std::string name);
bool hasflag(std::string fn, char flag);
std::string get_flag_string(std::string fn);
std::string get_internal_date_string(std::string fn);
std::string rfc822(std::string fn);
std::string rfc822_size(std::string fn);
std::string rfc822_header(std::string fn);
std::string rfc822_text(std::string fn);
std::string envelope(std::string fn);
std::string body_headers(std::string fn, std::vector<std::string> body_tokens);
#endif