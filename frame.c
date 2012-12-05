#include "frame.h"

#define MAX_KEY_LEN 20

struct tree_list_t;

typedef struct {
  char *string;
  Callback *callback;
  char *name;
  struct tree_list_t *children;
  int nchildren;
} Tree;

typedef struct tree_list_t {
  Tree *tree;
  struct tree_list_t *next;
} TreeList;

static TermKey *tk;
static Tree *bindings;
static Tree *current_binding;
static char *break_key = NULL;
static Callback *redraw_callback = NULL;

void tree_destroy(Tree *tree);

Tree *tree_create(char *string)
{
  Tree *tree;
  int string_len = strlen(string);
  tree = malloc(sizeof(Tree));
  tree->string = calloc(string_len + 1, sizeof(char));
  strcpy(tree->string, string);
  tree->callback = NULL;
  tree->name = NULL;
  tree->children = NULL;
  tree->nchildren = 0;

  return tree;
}

void tree_destroy_children(Tree *tree)
{
  TreeList *child;
  for(child = tree->children; child != NULL; child = child->next) {
    tree_destroy(child->tree);
    free(child);
  }
}

void tree_destroy(Tree *tree)
{
  tree_destroy_children(tree);
  free(tree);
}

void tree_set_callback(Tree *tree, Callback *callback, const char *name)
{
  tree_destroy_children(tree);
  tree->callback = callback;
  tree->name = calloc(strlen(name) + 1, sizeof(char));
  strcpy(tree->name, name);
}

Tree *tree_insert(Tree *tree, char *string)
{
  TreeList *new_child, *child, *previous_child;
  previous_child = NULL;
  int cmp;

  if(tree->children) {
    for(child = tree->children; child != NULL; child = child->next) {
      cmp = strcmp(string, child->tree->string);
      if(cmp == 0) {
        return child->tree;
      }
      else if(cmp < 0) {
        new_child = malloc(sizeof(TreeList));
        new_child->tree = tree_create(string);
        if(previous_child) {
          previous_child->next = new_child;
          new_child->next = child;
        }
        else {
          tree->children = new_child;
          new_child->next = child;
        }
        return new_child->tree;
      }
      previous_child = child;
    }
    new_child = malloc(sizeof(TreeList));
    new_child->tree = tree_create(string);
    new_child->next = NULL;
    previous_child->next = new_child;
  }
  else {
    new_child = malloc(sizeof(TreeList));
    new_child->tree = tree_create(string);
    new_child->next = NULL;
    tree->children = new_child;
  }

  return new_child->tree;
}

void tree_dump(Tree *tree, int level)
{
  TreeList *child;
  int i;
  for(child = tree->children; child; child = child->next) {
    for(i = 0; i < level * 2; i ++) {
      printf(" ");
    }
    printf("%s", child->tree->string);
    if(child->tree->name) {
      printf(" --> %s", child->tree->name);
    }
    printf("\n");
    tree_dump(child->tree, level + 1);
  }
}

void frame_dump_bindings(void)
{
  tree_dump(bindings, 0);
}

Tree *tree_find(Tree *tree, char *string)
{
  TreeList *child;
  int cmp;
  for(child = tree->children; child; child = child->next) {
    cmp = strcmp(child->tree->string, string);
    if(cmp == 0)
      return child->tree;
    else if(cmp > 0)
      return NULL;
  }
  return NULL;
}

void init_colours(void)
{
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
}

void frame_keybind(const char *string, Callback *callback, const char *name)
{
  if(!strlen(string))
    return;

  char *str = strdup(string);
  char *tok = strtok(str, " ");
  Tree *tree;
  tree = bindings;
  while(tok) {
    if(strcmp(tok, " ") != 0) {
      tree = tree_insert(tree, tok);
      tok = strtok(NULL, " ");
    }
  }
  free(str);
  tree_set_callback(tree, callback, name);
}

void frame_set_break_key(const char *string)
{
  break_key = strdup(string);
}

void frame_set_redraw_callback(Callback *callback)
{
  redraw_callback = callback;
}

void frame_init(void)
{
  TERMKEY_CHECK_VERSION;
  bindings = tree_create("");
  tk = termkey_new(0, TERMKEY_FLAG_SPACESYMBOL|TERMKEY_FLAG_CTRLC);
}

void frame_destroy(void)
{
  endwin();
  termkey_destroy(tk);
}

void frame_start(void)
{
  initscr();
  raw();
  nonl();
  keypad(stdscr, TRUE);
  noecho();

  TermKeyFormat format = TERMKEY_FORMAT_VIM;
  char buffer[50];

  if(has_colors()) {
    start_color();
    init_colours();
  }

  curs_set(0);

  current_binding = bindings;
  Tree *node;

  TermKeyKey key;

  // defaults
  if(break_key == NULL)
    frame_set_break_key("<C-g>");

  while(TRUE) {

    if(redraw_callback)
      redraw_callback();
    refresh();

    termkey_waitkey(tk, &key);
    termkey_strfkey(tk, buffer, sizeof buffer, &key, format);

    node = tree_find(current_binding, buffer);

    if(strcmp(buffer, break_key) == 0) {
      current_binding = bindings;
    }
    else if(node) {
      current_binding = node;
      if(current_binding->callback) {
        current_binding->callback();
        current_binding = bindings;
      }
    }
    else {
      current_binding = bindings;
    }

    if(key.type == TERMKEY_TYPE_UNICODE &&
       key.modifiers & TERMKEY_KEYMOD_CTRL &&
       (key.code.codepoint == 'C' || key.code.codepoint == 'c')) {
      frame_destroy();
      break;
    }
  }

  return;
}

void frame_draw_point(int x, int y)
{
  attron(COLOR_PAIR(1));
  mvaddch(y, x, ' ');
  attroff(COLOR_PAIR(1));
}

void frame_draw_line(int x1, int y1, int x2, int y2)
{
  double len = ceil(sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)));
  double dx = (x2 - x1) / len;
  double dy = (y2 - y1) / len;
  int x;
  int y;
  int i;
  //printf("%1.f, %.1f", x + dx * i, y + dy * i);
  for(i = 0; i < len; i ++) {
    x = round(x1 + dx * i);
    y = round(y1 + dy * i);
    frame_draw_point(x, y);
  }
}
