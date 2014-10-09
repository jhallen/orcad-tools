/* Lisp loader */
// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.

/* Lisp node types */

enum
  {
  List_id,
  Str_id,
  Ident_id,
  Num_id
  };

/* Some kind of LISP node */

struct List;
struct Str;
struct Ident;
struct Num;

struct Node
  {
  Node *next;
  int type;
  int line; /* Source file line number */
  char *file_name;

  /* Convert this generic type into a concrete type, probably based
   * on type code */
  inline List *list() { return (List *)this; }
  inline Str *str() { return (Str *)this; }
  inline Ident *ident() { return (Ident *)this; }
  inline Num *num() { return (Num *)this; }

  Node();
  };

/* A list: item points to first element in list */

struct List : Node
  {
  Node *item;

  List(Node *new_item, char *new_file_name, int line_no);
  };

/* A "string" */

struct Str : Node
  {
  string s; /* The string */

  Str(string new_string, char *new_file_name, int line_no);
  };

/* An identifier */

struct Ident : Node
  {
  string s; /* The string */

  Ident(string new_ident, char *new_file_name, int line_no);
  };

/* A number */

struct Num : Node
  {
  int num;

  Num(int new_num, char *new_file_name, int line_no);
  };

Node *lisp_load(char *name);
void lisp_dump(Node *n, ostream& out);
