#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#include "termkey.h"

#define MAX_KEY_LEN 20

static void finish(int sig);
typedef void (KeyCallback)(void);
struct tree_list_t;

typedef struct {
  char *string;
  KeyCallback *callback;
  char *name;
  struct tree_list_t *children;
  int nchildren;
} Tree;

typedef struct tree_list_t {
  Tree *tree;
  struct tree_list_t *next;
} TreeList;

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

void tree_set_callback(Tree *tree, KeyCallback *callback, char *name)
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

void tree_dump_impl(Tree *tree, int level)
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
    tree_dump_impl(child->tree, level + 1);
  }
}

void tree_dump(Tree *tree)
{
  tree_dump_impl(tree, 0);
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

Tree *bindings;
Tree *current_binding;

void keybind(char *string, KeyCallback *callback, char *name)
{
  if(!strlen(string))
    return;

  string = strdup(string);
  char *tok = strtok(string, " ");
  Tree *tree;
  tree = bindings;
  while(tok) {
    if(tok != " ") {
      tree = tree_insert(tree, tok);
      tok = strtok(NULL, " ");
    }
  }
  free(string);
  tree_set_callback(tree, callback, name);
}

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

TermKey *tk;

int main(int argc, char *argv[])
{
  bindings = tree_create("");
  keybind("<C-a>",  *test1, "test1");
  keybind("<C-f> <C-d>",  *test1, "test2");
  tree_dump(bindings);
  char break_key[5] = "<C-g>";
          //  return;

  TERMKEY_CHECK_VERSION;

  tk = termkey_new(0, TERMKEY_FLAG_SPACESYMBOL|TERMKEY_FLAG_CTRLC);

  initscr();
  raw();
  nonl();
  keypad(stdscr, TRUE);
  noecho();

  TermKeyFormat format = TERMKEY_FORMAT_VIM;
  char buffer[50];

  if(has_colors()) {
    start_color();
  }

  current_binding = bindings;
  Tree *node;

  while(TRUE) {
    TermKeyResult ret;
    TermKeyKey key;

    ret = termkey_waitkey(tk, &key);
    termkey_strfkey(tk, buffer, sizeof buffer, &key, format);

    node = tree_find(current_binding, buffer);

    printw("%s ", buffer);
    if(strcmp(buffer, break_key) == 0) {
      printw("... Break\n");
      current_binding = bindings;
    }
    else if(node) {
      current_binding = node;
      if(current_binding->callback) {
        printw("... Executing\n");
        current_binding->callback();
        current_binding = bindings;
      }
    }
    else {
      printw("... Not found\n");
      current_binding = bindings;
    }

    refresh();

    if(key.type == TERMKEY_TYPE_UNICODE &&
       key.modifiers & TERMKEY_KEYMOD_CTRL &&
       (key.code.codepoint == 'C' || key.code.codepoint == 'c')) {
      finish(0);
    }
  }
}

static void finish(int sig)
{
  endwin();
  termkey_destroy(tk);
  exit(0);
}
