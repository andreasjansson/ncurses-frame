#include "frame.h"

static int x, y;

void draw_piano(void)
{
  int ys[] = {0, 2, 4, 5, 7, 9, 11};
  int i;
  for(i = 0; i < 7; i ++) {
    frame_draw_line(0, 12 - ys[i], 80, 12 -ys[i]);
  }
}

void redraw(void)
{
  frame_draw_point(x, y);
  draw_piano();
}

void up(void)
{
  y --;
}

void down(void)
{
  y ++;
}

void left(void)
{
  x --;
}

void right(void)
{
  x ++;
}

int main(int argc, char *argv[])
{
  frame_init();
  frame_keybind("<C-p>", *up, "up");
  frame_keybind("<C-n>", *down, "down");
  frame_keybind("<C-b>", *left, "left");
  frame_keybind("<C-f>", *right, "right");
  frame_set_redraw_callback(*redraw);
  frame_start();

  return 0;
}
