#include "utils.hpp"
#include <string.h>
const char *b64value =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::vector<std::string> tokenize_line(std::string line, char c) {
  std::stringstream ss(line);
  std::vector<std::string> tokens;
  std::string imm;
  while (getline(ss, imm, c)) {
    tokens.push_back(imm);
  }
  return tokens;
}
void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}
bool cppstrncasecmp(const std::string &a, const std::string &b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) {
    return std::tolower(a) == std::tolower(b);
  });
}
int base64encode(const char *data_in, char *data_out, int in_len, int max_out) {
  int i;
  int a, b, c, d;
  int out_index = 0;
  int in_index = 0;
  int padding = in_len % 3;
  if (!data_in || !data_out || !in_len) {
    return 0;
  }
  if (max_out < 5) {
    return 0;
  }
  for (i = 0; (i + 2 < in_len) && out_index + 4 < max_out; i += 3) {
    a = (data_in[i] >> 2) & 0x3f;
    b = (((data_in[i] & 0x3) << 4) | (data_in[i + 1] >> 4)) & 0x3f;
    c = (((data_in[i + 1] & 0xf) << 2) | (data_in[i + 2] >> 6)) & 0x3f;
    d = (data_in[i + 2] & 0x3f);
    data_out[out_index++] = b64value[a];
    data_out[out_index++] = b64value[b];
    data_out[out_index++] = b64value[c];
    data_out[out_index++] = b64value[d];
  }
  in_index = (in_len / 3) * 3;
  if (padding && out_index + 4 < max_out) {
    a = (data_in[in_index] >> 2) & 0x3f;
    b = (data_in[in_index] & 0x3) << 4;
    data_out[out_index++] = b64value[a];
    if (in_len < in_index + 1) {
      data_out[out_index++] = b64value[b];
      data_out[out_index++] = '=';
      data_out[out_index++] = '=';
    } else {
      in_index++;
      b = b | (data_in[in_index] >> 4);
      data_out[out_index++] = b64value[b];
      c = (data_in[in_index] & 0xf) << 2;
      data_out[out_index++] = b64value[c];
      data_out[out_index++] = '=';
    }
  }
  return out_index;
}
int base64decode(const char *data_in, char *data_out, int in_len, int max_out) {
  int outlen = 0;
  char *t;
  int bits;
  int i = 0;
  if (!data_in || !data_out || !in_len) {
    return 0;
  }
  if (in_len % 4) {
    printf("Bad inlen\n");
    return 0;
  }
  for (i = 0; i + 4 < in_len; i += 4) {
    if (max_out < outlen + 3) {
      return 0;
    }
    t = strchr(b64value, data_in[i]);
    if (t == NULL) {
      printf("bad1\n");
      return 0;
    }
    bits = (t - b64value) << 18;
    t = strchr(b64value, data_in[i + 1]);
    if (t == NULL) {
      printf("bad2: %c\n", data_in[i + 1]);
      return 0;
    }
    bits |= ((t - b64value) & 0x3f) << 12;
    t = strchr(b64value, data_in[i + 2]);
    if (t == NULL) {
      printf("bad3\n");
      return 0;
    }
    bits |= ((t - b64value) & 0x3f) << 6;
    t = strchr(b64value, data_in[i + 3]);
    if (t == NULL) {
      printf("bad4: %c\n", data_in[i + 3]);
      return 0;
    }
    bits |= ((t - b64value) & 0x3f);
    data_out[outlen++] = (bits >> 16) & 0xff;
    data_out[outlen++] = (bits >> 8) & 0xff;
    data_out[outlen++] = (bits & 0xff);
  }
  if (max_out < outlen + 3) {
    return 0;
  }
  t = strchr(b64value, data_in[i]);
  if (t == NULL) {
    printf("bad1\n");
    return 0;
  }
  bits = (t - b64value) << 18;
  t = strchr(b64value, data_in[i + 1]);
  if (t == NULL) {
    printf("bad2: %c\n", data_in[i + 1]);
    return 0;
  }
  bits |= ((t - b64value) & 0x3f) << 12;
  if (data_in[i + 2] == '=') {
    data_out[outlen++] = (bits >> 16) & 0xff;
    data_out[outlen++] = (bits >> 8) & 0xff;
    return outlen;
  }
  t = strchr(b64value, data_in[i + 2]);
  if (t == NULL) {
    printf("bad3\n");
    return 0;
  }
  bits |= ((t - b64value) & 0x3f) << 6;
  if (data_in[i + 3] == '=') {
    data_out[outlen++] = (bits >> 16) & 0xff;
    data_out[outlen++] = (bits >> 8) & 0xff;
    data_out[outlen++] = (bits & 0xff);
    return outlen;
  }
  t = strchr(b64value, data_in[i + 3]);
  if (t == NULL) {
    printf("bad4: %c\n", data_in[i + 3]);
    return 0;
  }
  bits |= ((t - b64value) & 0x3f);
  data_out[outlen++] = (bits >> 16) & 0xff;
  data_out[outlen++] = (bits >> 8) & 0xff;
  data_out[outlen++] = (bits & 0xff);
  data_out[outlen] = '\x00';
  return outlen;
}
std::vector<std::string> get_file_list(std::string fullpath) {
  std::filesystem::path canon;
  std::vector<std::string> f;
  std::string dirline;
  f.clear();
  canon.assign(fullpath);
  try {
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    return f;
  }
  for (const std::filesystem::directory_entry &e :
       std::filesystem::directory_iterator(canon)) {
    dirline = e.path().string();
    f.push_back(dirline);
  }
  return f;
}
bool isdigit_wrapper(unsigned char c) { return !std::isdigit(c); }
bool isnumber(const std::string &s) {
  int index = 0;
  if (s[0] == '-') {
    index = 1;
  }
  if (s.empty()) {
    return false;
  }
  return std::find_if(s.begin() + index, s.end(), isdigit_wrapper) == s.end();
}