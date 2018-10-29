#include "print.h"
#include "init.h"

int main() {
  init_all();
  asm volatile("sti");
  while (1);
}
