#include "target_x86.h"
#include <assert.h>
int main() {
  assert(target_x86_test() == 0);
  return 0;
}