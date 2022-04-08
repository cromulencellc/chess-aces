#ifndef DICTWIDGET_H_
#define DICTWIDGET_H_

#include <vector>
#include <string>

#include <Wt/WComboBox.h>

#include "MaybeWord.h"

class DictWidget : public Wt::WComboBox {
  public:
  DictWidget(const char* path);
  ~DictWidget();

  MaybeWord RandomWord();

  private:
  std::vector<int> fds;
  std::vector<unsigned int> num_words;
};

#endif