#include "frame.h"

void test1(void)
{
  printw("test1\n");
}

void test2(void)
{
  printw("test2\n");
}

void test3(void)
{
  printw("test3\n");
}

int main(int argc, char *argv[])
{
  frame_init();
  frame_keybind("<C-a>",  *test1, "test1");
  frame_keybind("<C-f> <C-d>",  *test2, "test2");
  frame_set_break_key("<C-g>");
  frame_start();

  return 0;
}
