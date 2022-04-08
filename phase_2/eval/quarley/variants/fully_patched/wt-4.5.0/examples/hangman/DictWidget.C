#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <regex>
#include <string>
#include <sstream>
#include <system_error>

#include <Wt/WStringUtil.h>

#include "DictWidget.h"

const size_t WORD_BUF_LEN = 1024;

namespace {
  const std::regex dict_name("dict-[a-z][a-z].txt$");
}

DictWidget::DictWidget(const char* path) : Wt::WComboBox() {
  srand(time(0));
  DIR* dict_dir = opendir(path);

  if (nullptr == dict_dir) {
    throw new std::system_error(errno,
      std::system_category());
  }

  struct dirent* entry = nullptr;

  std::string full_path = "";
  std::vector<char> word_buf(WORD_BUF_LEN, 0);

  while (entry = readdir(dict_dir)) {
    if (std::regex_match(entry->d_name, dict_name)) {
      full_path.clear();
      full_path.append(path);
      full_path.append("/");
      full_path.append(entry->d_name);
      int fd = open(full_path.c_str(), O_RDONLY);
      if (-1 == fd) {
        throw new std::system_error(errno,
        std::system_category());
      }
      fds.push_back(fd);

      unsigned int words = 0;

      while (true) {
        word_buf.resize(WORD_BUF_LEN);
        ssize_t got = read(fd, word_buf.data(), word_buf.size());

        if (-1 == got) {
          throw new std::system_error(errno,
          std::system_category());
        }
        if (0 == got) {
          break;
        }

        word_buf.resize(got);

        for (size_t i = 0; i < word_buf.size(); i++) {
          if ('\n' == word_buf[i]) words += 1;
        }
      }

      num_words.push_back(words);

      addItem(entry->d_name);
    }
  }

  closedir(dict_dir);
  dict_dir = nullptr;
}

DictWidget::~DictWidget() {
  for (size_t i = 0; i < fds.size(); i++) {
    close(fds[i]);
  }
}

MaybeWord DictWidget::RandomWord() {
  int i = currentIndex();
  if (i < 0) i = 0;
  int fd = fds[i];
  int word_count = num_words[i];

  int seek_got = lseek(fd, 0, SEEK_SET);
  if (-1 == seek_got) {
    std::ostringstream message;
    message << "failed to seek fd " << fd;
    return std::system_error(errno, std::system_category(),
      message.str());
  }
  if (word_count <= 0) {
    std::ostringstream message;
    message << "got nonsense word count " << word_count <<
     " from fd " << fd;
    std::cerr << message.str() << std::endl;
    return std::runtime_error(message.str());
  }
  int selection = rand() % word_count;
  std::cerr << "looking for word " << selection << 
    " of " << word_count << " in fd " << fd << std::endl;
  int cur_word = 0;

  int word_start = -1;
  int word_end = -1;

  std::vector<char> word_buf(WORD_BUF_LEN, 0);
    
  while (true) {
    std::cerr << "at word " << cur_word << std::endl;
    word_buf.resize(WORD_BUF_LEN);
    ssize_t got = read(fd, word_buf.data(), word_buf.size());
    if (-1 == got) {
          return std::system_error(errno,
          std::system_category());
    }
    if (0 == got) {
      std::ostringstream message;
      message << "couldn't find word idx " << selection <<
        " in fd " << fd;
      return std::runtime_error(message.str());
    }

    word_buf.resize(got);

    for (size_t i = 0; i < word_buf.size(); i++) {
      if ('\n' == word_buf[i]) cur_word++;
      if (cur_word == selection) {
        word_start = i + 1;
        break;
      }
    }
    if (-1 != word_start) {
      std::cerr << "word_start " << word_start << std::endl;
      for(size_t i = word_start + 1; i < word_buf.size(); i++) {
        if ('\n' == word_buf[i]) {
          word_end = i;
          break;
        }
      }
      if (-1 != word_end) break;
      
      // hit the end of the buffer, let's just try again
      return RandomWord();
    }

  }
  
  std::cerr << "start " << word_start << " end " << word_end << std::endl;

  return Wt::widen(std::string(word_buf.data() + word_start,
                                word_end - word_start));
}