// Lisp loader

// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.


#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>

using namespace std;

int orcad_edif_bug = 0;

#include "lisp.h"

Node::Node()
  {
  next = 0;
  }

List::List(Node *new_item, char *new_file_name, int line_no)
  {
  type = List_id;
  item = new_item;
  line = line_no;
  file_name = new_file_name;
  }

Str::Str(string new_string, char *new_file_name, int line_no)
  {
  type = Str_id;
  s = new_string;
  line = line_no;
  file_name = new_file_name;
  }

Ident::Ident(string new_ident, char *new_file_name, int line_no)
  {
  type = Ident_id;
  s = new_ident;
  line = line_no;
  file_name = new_file_name;
  }

Num::Num(int new_num, char *new_file_name, int line_no)
  {
  type = Num_id;
  num = new_num;
  line = line_no;
  file_name = new_file_name;
  }

Node *load_node(char *filename, istream& in, int& line)
  {
  for(;;)
    {
    int c = in.get();
    switch(c)
      {
      default:
        {
        in.unget();
        return 0;
        }

      case -1:
        /* End of file */
        return 0;

      case ' ': case '\t': case '\r':
        /* Skip whitespace */
        break;

      case '\n':
        /* Skip whitespace, inc line number */
        ++line;
        break;

      case '(':
        { /* Parse a list */
        Node *v;
        Node *first = 0, *last = 0;
        while(v = load_node(filename, in, line))
          if(last) last->next = v, last = v;
          else first = last = v;
        c = in.get();
        if(c != ')')
          {
          cerr << filename << ' ' << line << ": Error: parenthesis mismatch\n";
          exit(-1);
          }
        return new List(first, filename, line);
        }

      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        { /* Parse a number */
        int x = c - '0';
        while(c = in.get(), c >= '0' && c <= '9')
          x = x * 10 + c - '0';
        if(c != -1) in.unget();
        return new Num(x, filename, line);
        }

      case '"':
        { /* Parse a string */
        string s;
        int start_line = line;
        more:
        while(c = in.get(), (c != '"' && c != -1)) s += c;
        if (c == -1)
          {
          cerr << filename << " " << start_line << ": Error: unterminated string\n";
          }
        else if (orcad_edif_bug)
          {
          /* OrCAD hack */
          c = in.get();
          if (c != ')')
            {
            s += '"';
            goto more;
            }
          else
            {
            in.unget();
            }
          }
        return new Str(s, filename, line);
        }

      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
      case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
      case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
      case 'v': case 'w': case 'x': case 'y': case 'z':
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
      case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
      case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
      case 'V': case 'W': case 'X': case 'Y': case 'Z':
      case '_': case '&':
        { /* Parse an identifier */
        string s;
        if (c != '&')
          s += c;
        while(c = in.get(), 
              c >= 'a' && c <= 'z' ||
              c >= 'A' && c <= 'Z' ||
              c >= '0' && c <= '9' ||
              c == '_')
          s += c;
        if(c != -1) in.unget();
        return new Ident(s, filename, line);
        }
      }
    }
  }

Node *lisp_load(char *name)
  {
  Node *v;
  int c;
  int line = 1;
  ifstream f;
  f.open(name, ios::in);
  if(!f)
    {
    cerr << "couldn't open " << name << "\n";
    exit(-1);
    }
  v = load_node(name, f, line);
  while(c = f.get(), c!=-1)
    if(c == '\n') ++line;
    else if(c != ' ' && c != '\t' && c != '\r') break;
  if(c != -1)
    {
    cerr << name << ' ' << line << ": extra junk in input - goodbye\n";
    exit(-1);
    }
  return v;
  }

void lisp_dump(Node *n, ostream& out)
  {
  switch(n->type)
    {
    case List_id:
      {
      out << ' ';
      out << '(';
      for (n = n->list()->item;n;n = n->next)
        {
        lisp_dump(n, out);
        }
      out << ' ';
      out << ')';
      break;
      }
    case Str_id:
      {
      out << ' ';
      out << '"';
      out << n->str()->s;
      out << '"';
      break;
      }
    case Ident_id:
      {
      out << ' ';
      out << n->ident()->s;
      break;
      }
    case Num_id:
      {
      out << ' ';
      out << n->num()->num;
      break;
      }
    }
  }
