#ifndef FRAME_H
#define FRAME_H

#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <math.h>

#include "termkey.h"

typedef void (Callback)(void);

void frame_init(void);
void frame_start(void);
void frame_destroy(void);
void frame_keybind(const char *string, Callback *callback, const char *name);
void frame_set_break_key(const char *string);
void frame_set_redraw_callback(Callback *callback);
void frame_draw_point(int x, int y);
void frame_draw_line(int x1, int y1, int x2, int y2);
void frame_dump_bindings(void);

#endif
