// OrCAD .INF file loader

// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.


#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include "string.h"

using namespace std;

#include "lisp.h"
#include "hash.h"
#include "dlist.h"
#include "net.h"
#include "inf.h"

extern int debug;

// Lexical Tokens

enum {
	TOK_EOF,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_FIELD,

	TOK_H_HEADER,
	TOK_F_HEADER,
	TOK_TITLE,
	TOK_LINK,
	TOK_PORT,
	TOK_SIGNAL,
	TOK_EXTERNAL,
	TOK_INSTANCE,
	TOK_JOINED,
	TOK_LAYOUT,
	TOK_TRACE,
	TOK_VECTOR,
	TOK_STIMULUS,
	TOK_PIPE
};

char *tok_names[] = 
{
	"EOF",
	"LPAREN",
	"RPAREN",
	"FIELD",
	"H HEADER",
	"F HEADER",
	"TITLE",
	"LINK",
	"PORT",
	"SIGNAL",
	"EXTERNAL",
	"INSTANCE",
	"JOINED",
	"LAYOUT",
	"TRACE",
	"VECTOR",
	"STIMULUS",
	"PIPE"
};

// Tokenize .INF file

int ungot_tok = -1;

void unget_tok(int token)
  {
  ungot_tok = token;
  }

int get_tok(const char *name,int& line, istream& in, string& tok_str)
  {
  int c;
  if (ungot_tok != -1)
    {
    c = ungot_tok;
    ungot_tok = -1;
    return c;
    }
  /* Get next character, skip whitespace */
  do
    {
    c = in.get();
    if (c == '\n')
      ++line;
    } while (c == ' ' || c == '\t' || c == '\r' || c == '\n');
  if (c == '`')
    {
    c = in.get();
    if (c == 'H')
      return TOK_H_HEADER;
    else if (c == 'F')
      return TOK_F_HEADER;
    else if (c == 'B')
      return TOK_TITLE;
    else if (c == 'L')
      return TOK_LINK;
    else if (c == 'P')
      return TOK_PORT;
    else if (c == 'S')
      return TOK_SIGNAL;
    else if (c == 'E')
      return TOK_EXTERNAL;
    else if (c == 'I')
      return TOK_INSTANCE;
    else if (c == 'J')
      return TOK_JOINED;
    else if (c == 'K')
      return TOK_LAYOUT;
    else if (c == 'T')
      return TOK_TRACE;
    else if (c == 'V')
      return TOK_VECTOR;
    else if (c == 'W')
      return TOK_STIMULUS;
    else if (c == '|')
      return TOK_PIPE;
    else
      {
      cerr << name << " " << line << ": Error: Unknown statement\n";
      exit(-1);
      }
    }
  else if (c == '(')
    return TOK_LPAREN;
  else if (c == ')')
    return TOK_RPAREN;
  else if (c == -1)
    return TOK_EOF;
  else if (c == '"')
    {
    tok_str = "";
    do
      {
      while ((c = in.get()), (c != -1 && c != '"'))
        {
        tok_str += c;
        }
      if (c == '"')
        {
        c = in.get();
        if (c == '"')
          {
          tok_str += c;
          }
        else if (c != -1)
          {
          in.unget();
          c = -1;
          }
        }
      } while (c != -1);
    return TOK_FIELD;
    }
  else
    {
    tok_str = "";
    tok_str += c;
    while ((c = in.get()), (c != -1 && c != ' ' && c != '\t' && c != '\r' && c != '\n'))
      {
      tok_str += c;
      }
    if (c != -1)
      in.unget();
    return TOK_FIELD;
    }
  }

struct load_stack
  {
  struct load_stack *next;
  char *name;
  } *the_load_stack = 0;

// Parse .INF file

InfDesign *inf_load_node(const char* name, int& line, istream& in)
  {
  InfDesign *inf = 0;
  string tok_str;

  for (;;)
    {
    int t = get_tok(name, line, in, tok_str);
    switch(t)
      {
      case TOK_H_HEADER: case TOK_F_HEADER:
        {
        if (!inf)
          inf = new InfDesign();
        else
          cerr << name << " " << line << ": Error: Two Headers\n";
        if (t == TOK_H_HEADER)
          inf->hier = 1;
        else
          inf->hier = 0;

        t = get_tok(name, line, in, tok_str); // Format_version
          inf->ver = tok_str;

        t = get_tok(name, line, in, tok_str); // File name
          inf->name = tok_str;

        /* All set: now ready for rest of design */
        break;
        }
      case TOK_TITLE:
        {
        t = get_tok(name, line, in, tok_str); // sheet number
          inf->sheet_number = tok_str;
        t = get_tok(name, line, in, tok_str); // total sheet number
          inf->total_sheet_number = tok_str;
        t = get_tok(name, line, in, tok_str); // sheet size
          inf->sheet_size = tok_str;
        t = get_tok(name, line, in, tok_str); // date
          inf->date = tok_str;
        t = get_tok(name, line, in, tok_str); // document number
          inf->document_number = tok_str;
        t = get_tok(name, line, in, tok_str); // revision code
          inf->revision_code = tok_str;
        t = get_tok(name, line, in, tok_str); // title
          inf->title = tok_str;
        t = get_tok(name, line, in, tok_str); // organization name
          inf->organization_name = tok_str;
        t = get_tok(name, line, in, tok_str); // address 1
          inf->address_line_1 = tok_str;
        t = get_tok(name, line, in, tok_str); // address 2
          inf->address_line_2 = tok_str;
        t = get_tok(name, line, in, tok_str); // address 3
          inf->address_line_3 = tok_str;
        t = get_tok(name, line, in, tok_str); // address 4
          inf->address_line_4 = tok_str;
        break;
        }
      case TOK_EOF:
        {
        return inf;
        break;
        }
      case TOK_LPAREN:
        {
        cerr << name << " " << line << ": Error: not expecting ( here\n";
        exit(-1);
        break;
        }
      case TOK_RPAREN:
        {
        cerr << name << " " << line << ": Error: not expecting ) here\n";
        exit(-1);
        break;
        }
      case TOK_FIELD:
        {
        cerr << name << " " << line << ": Error: not expecting field here\n";
        exit(-1);
        break;
        }
      case TOK_LINK:
        {
        InfLink *l;
        t = get_tok(name, line, in, tok_str); // filename
        l = new InfLink();
        l->name = tok_str;
        inf->links.add(l->name, l);
        break;
        }
      case TOK_PORT:
        {
        InfPort *p = new InfPort();
        t = get_tok(name, line, in, tok_str); // Port type
        if (tok_str == "I")
          p->type = 'I';
        else if (tok_str == "O")
          p->type = 'O';
        else if (tok_str == "B")
          p->type = 'B';
        else if (tok_str == "U")
          p->type = 'U';
        else if (tok_str == "S")
          p->type = 'S';
        else
          {
          // Huh?
          p->type = 'I';
          }
        t = get_tok(name, line, in, tok_str); // Port name
        p->name = tok_str;
        inf->ports.add(p->name, p);
        break;
        }
      case TOK_SIGNAL:
        {
        InfSignal *s = new InfSignal();
        t = get_tok(name, line, in, tok_str);
        s->name = tok_str;
        t = get_tok(name, line, in, tok_str);
        s->sheet_number = atoi(tok_str.c_str());
        inf->signals.add(s->name, s);
        break;
        }
      case TOK_EXTERNAL:
        {
        InfExtern *e = new InfExtern();
        t = get_tok(name, line, in, tok_str); // External library
        e->name = tok_str;
        inf->externs.add(e->name, e);
        break;
        }
      case TOK_INSTANCE:
        {
        InfInstance *i = new InfInstance();
        t = get_tok(name, line, in, tok_str); // R or C
        if (tok_str == "R")
          { /* Part instance */
          i->type = 'R';
          t = get_tok(name, line, in, tok_str); // Part value
          i->part_value = tok_str;
          t = get_tok(name, line, in, tok_str); // Library
          i->library = tok_str;
          t = get_tok(name, line, in, tok_str); // Library part name
          i->library_part_name = tok_str;
          t = get_tok(name, line, in, tok_str); // Absolute identifier
          i->absolute_identifier = tok_str;
          t = get_tok(name, line, in, tok_str); // Part reference
          i->name = tok_str;
          t = get_tok(name, line, in, tok_str); // Sub part code
          i->sub_part_code = tok_str;
          t = get_tok(name, line, in, tok_str); // Part field 1
          i->part_field[0] = tok_str;
          t = get_tok(name, line, in, tok_str); // Part field 2
          i->part_field[1] = tok_str;
          t = get_tok(name, line, in, tok_str); // Part field 3
          i->part_field[2] = tok_str;
          t = get_tok(name, line, in, tok_str); // Part field 4
          i->part_field[3] = tok_str;
          t = get_tok(name, line, in, tok_str); // Part field 5
          i->part_field[4] = tok_str;
          t = get_tok(name, line, in, tok_str); // Part field 6
          i->part_field[5] = tok_str;
          t = get_tok(name, line, in, tok_str); // Part field 7
          i->part_field[6] = tok_str;
          t = get_tok(name, line, in, tok_str); // Part field 8
          i->part_field[7] = tok_str;
          t = get_tok(name, line, in, tok_str); // Module field
          i->module_field = tok_str;
          /* Get pins */
          for (;;)
            {
            t = get_tok(name, line, in, tok_str);
            if (t == TOK_LPAREN)
              {
              InfPin *p = new InfPin();
              t = get_tok(name, line, in, tok_str); /* Pin name */
              p->name = tok_str;
              t = get_tok(name, line, in, tok_str); /* Pin number */
              p->pin_number = tok_str;
              t = get_tok(name, line, in, tok_str); /* Direction */
              if (tok_str == "I")
                p->type = 'I';
              else if (tok_str == "O")
                p->type = 'O';
              else if (tok_str == "B")
                p->type = 'B';
              else if (tok_str == "S")
                p->type = 'S';
              else if (tok_str == "P")
                p->type = 'P';
              else if (tok_str == "T")
                p->type = 'T';
              else if (tok_str == "C")
                p->type = 'C';
              else if (tok_str == "E")
                p->type = 'E';
              else // Huh?
                p->type = 'I';
              t = get_tok(name, line, in, tok_str); /* Right parenthesis */
              i->pins.add(p->name, p);
              }
            else
              {
              unget_tok(t);
              break;
              }
            }
          inf->instances.add(i->name, i);
          }
        else if (tok_str == "C")
          { /* Sheet instance */
          InfInstance *i = new InfInstance();
          i->type = 'C';
          t = get_tok(name, line, in, tok_str); // Sheet file name
          i->sheet_file_name = tok_str;
          t = get_tok(name, line, in, tok_str); // Absolute identifier
          i->absolute_identifier = tok_str;
          t = get_tok(name, line, in, tok_str); // Sheet name
          i->name = tok_str;

          /* Get pins */
          for (;;)
            {
            t = get_tok(name, line, in, tok_str);
            if (t == TOK_LPAREN)
              {
              InfPin *p = new InfPin();
              t = get_tok(name, line, in, tok_str); /* Port name */
              p->name = tok_str;
              t = get_tok(name, line, in, tok_str); /* Direction */
              if (tok_str == "I")
                p->type = 'I';
              else if (tok_str == "O")
                p->type = 'O';
              else if (tok_str == "B")
                p->type = 'B';
              else if (tok_str == "S")
                p->type = 'S';
              else if (tok_str == "P")
                p->type = 'P';
              else if (tok_str == "T")
                p->type = 'T';
              else if (tok_str == "C")
                p->type = 'C';
              else if (tok_str == "E")
                p->type = 'E';
              else // Huh?
                p->type = 'I';
              t = get_tok(name, line, in, tok_str); /* Right parenthesis */
              i->pins.add(p->name, p);
              }
            else
              {
              unget_tok(t);
              break;
              }
            }
          inf->instances.add(i->name, i);
          }
        else
          {
          cerr << name << " " << line << ": Error: Unknown instance type (expecting R or C)\n";
          exit(-1);
          }
        break;
        }
      case TOK_JOINED: /* A net */
        {
        InfJoin *ji = new InfJoin();
        for (;;)
          {
          t = get_tok(name, line, in, tok_str);
          if (t == TOK_LPAREN)
            {
            t = get_tok(name, line, in, tok_str); /* PortRef type */
            /* Net name: Pick a port.  If no port, pick a signal. */
            /* Add all other locations as pin references */
            if (tok_str == "S")
              { /* Signal */
              InfJoinItem *j = new InfJoinItem();
              j->type = 'S';
              t = get_tok(name, line, in, tok_str); /* signal name */
              j->name = tok_str;
              t = get_tok(name, line, in, tok_str); /* sheet number */
              j->sheet_number = atoi(tok_str.c_str());
              ji->items.add(j);
              }
            else if (tok_str == "P")
              { /* Port */
              InfJoinItem *j = new InfJoinItem();
              j->type = 'P';
              t = get_tok(name, line, in, tok_str); /* port type */
              if (tok_str == "I")
                j->pin_type = 'I';
              else if (tok_str == "O")
                j->pin_type = 'O';
              else if (tok_str == "B")
                j->pin_type = 'B';
              else if (tok_str == "U")
                j->pin_type = 'U';
              else if (tok_str == "S")
                j->pin_type = 'S';
              else
                j->pin_type = 'I';
              t = get_tok(name, line, in, tok_str); /* port name */
              j->name = tok_str;
              ji->items.add(j);
              }
            else if (tok_str == "R")
              { /* Part pin */
              InfJoinItem *j = new InfJoinItem();
              j->type = 'R';
              t = get_tok(name, line, in, tok_str); /* part reference */
              j->instance_name = tok_str;
              t = get_tok(name, line, in, tok_str); /* pin number */
              j->name = tok_str;
              t = get_tok(name, line, in, tok_str); /* pin type */
              if (tok_str == "I")
                j->pin_type = 'I';
              else if (tok_str == "O")
                j->pin_type = 'O';
              else if (tok_str == "B")
                j->pin_type = 'B';
              else if (tok_str == "S")
                j->pin_type = 'S';
              else if (tok_str == "P")
                j->pin_type = 'P';
              else if (tok_str == "T")
                j->pin_type = 'T';
              else if (tok_str == "C")
                j->pin_type = 'C';
              else if (tok_str == "E")
                j->pin_type = 'E';
              else // Huh?
                j->pin_type = 'I';
              ji->items.add(j);
              }
            else if (tok_str == "C")
              { /* Sheet pin */
              InfJoinItem *j = new InfJoinItem();
              j->type = 'C';
              t = get_tok(name, line, in, tok_str); /* sheet name */
              j->instance_name = tok_str;
              t = get_tok(name, line, in, tok_str); /* sheet net name */
              j->name = tok_str;
              t = get_tok(name, line, in, tok_str); /* sheet net type */
              if (tok_str == "I")
                j->pin_type = 'I';
              else if (tok_str == "O")
                j->pin_type = 'O';
              else if (tok_str == "B")
                j->pin_type = 'B';
              else if (tok_str == "S")
                j->pin_type = 'S';
              else if (tok_str == "P")
                j->pin_type = 'P';
              else if (tok_str == "T")
                j->pin_type = 'T';
              else if (tok_str == "C")
                j->pin_type = 'C';
              else if (tok_str == "E")
                j->pin_type = 'E';
              else // Huh?
                j->pin_type = 'I';
              ji->items.add(j);
              }
            t = get_tok(name, line, in, tok_str); /* ) */
            }
          else
            {
            unget_tok(t);
            break;
            }
          }
        inf->joins.add(ji);
        break;
        }
      case TOK_LAYOUT:
        {
        cerr << name << " " << line << ": Error: don't know how to deal with layout\n";
        exit(-1);
        break;
        }
      case TOK_TRACE:
        {
        cerr << name << " " << line << ": Error: don't know how to deal with trace\n";
        exit(-1);
        break;
        }
      case TOK_VECTOR:
        {
        cerr << name << " " << line << ": Error: don't know how to deal with vector\n";
        exit(-1);
        break;
        }
      case TOK_STIMULUS:
        {
        cerr << name << " " << line << ": Error: don't know how to deal with stimulus\n";
        exit(-1);
        break;
        }
      case TOK_PIPE:
        {
        InfPipe *l;
        InfPipeItem *i;
        t = get_tok(name, line, in, tok_str); // filename
        l = new InfPipe();
        l->name = tok_str;
        inf->pipes.add(l->name, l);
        while ((t = get_tok(name, line, in, tok_str)) == TOK_FIELD)
          {
          i = new InfPipeItem();
          i->s = tok_str;
          l->items.add(i);
          }
        unget_tok(t);
        break;
        }
      }
    }
  return inf;
  }

// Load a .INF file

InfDesign *inf_load_1(const char *name)
  {
  InfDesign *v;
  int c;
  ifstream f;
  int line = 1;
  f.open(name, ios::in);
  if(!f)
    {
    cerr << "couldn't open " << name << "\n";
    exit(-1);
    }
  cout << "Loading " << name << "\n";
  v = inf_load_node(name, line, f);
  while(c = f.get(), c!=-1)
    if(c == '\n') ++line;
    else if(c != ' ' && c != '\t' && c != '\r') break;
  if(c != -1)
    {
    cerr << name << ' ' << line << ": Error: extra junk in input - goodbye\n";
    exit(-1);
    }
  f.close();
  return v;
  }

// Dump a loaded .INF file
// (this is not done)

void InfDump(InfDesign *inf, ostream& out)
  {
  if (inf->hier)
    cout << "`H ";
  else
    cout << "`F ";
  cout << inf->ver << " " << inf->name << "\n";

  cout << "`B \"" << inf->sheet_number << "\" \"" << inf->total_sheet_number << "\" \"" <<
    inf->sheet_size << "\" \"" << inf->date << "\" \"" << inf->document_number << "\" \"" <<
    inf->revision_code << "\"\n\"" << inf->title << "\" \"" << inf->organization_name << "\"\n\"" <<
    inf->address_line_1 << "\"\n\"" << inf->address_line_2 << "\"\n\"" <<
    inf->address_line_3 << "\"\n\"" << inf->address_line_4 << "\"\n";

  // Ports
  Hash<InfPort *>::ptr pi;
  for (pi=inf->ports.first();pi ;pi++)
    {
    InfPort *p = *pi;
    cout << "`P " << (char)p->type << " \"" << p->name << "\"\n";
    }

  // Externs
  Hash<InfExtern *>::ptr ei;
  for (ei=inf->externs.first();ei ;ei++)
    {
    InfExtern *e = *ei;
    cout << "`E " << e->name << "\n";
    }

  // Instances
  Hash<InfInstance *>::ptr ii;
  for (ii=inf->instances.first();ii;ii++)
    {
    InfInstance *i = *ii;
    if (i->type == 'R')
      {
      int x;
      cout << "`I R \"" << i->part_value << "\" " << i->library << " \"" << i->library_part_name << "\" " << i->absolute_identifier <<
        "  " << i->name << "  " << i->sub_part_code << "\n";
      for (x = 0; x != 8; ++x)
        {
        cout << "\"" << i->part_field[x] << "\"";
        if (x != 7)
          cout << " ";
        }
      cout << "\n";
      Hash<InfPin *>::ptr pi;
      for (pi=i->pins.first();pi;pi++)
        {
        InfPin *p = *pi;
        cout << "( \"" << p->name << "\" \"" << p->pin_number << "\" " << (char)p->type << " )\n";
        }
      }
    else
      {
      }
    }

  // Joins
  Dlist<InfJoin *>::ptr ji;
  for (ji=inf->joins.first();ji;ji++)
    {
    InfJoin *j = *ji;
    cout << "`J ";
    Dlist<InfJoinItem *>::ptr ii;
    for (ii=j->items.first(); ii; ii++)
      {
      InfJoinItem *i = *ii;
      if (i->type == 'S')
        {
        cout << "( S \"" << i->name << "\" " << i->sheet_number << " )\n";
        }
      else if (i->type == 'P')
        {
        cout << "( P " << (char)i->pin_type << " \"" << i->name << "\" )\n";
        }
      else if (i->type == 'R')
        {
        cout << "( R " << i->instance_name << " \"" << i->name << "\" " << (char)i->pin_type << " )\n";
        }
      else if (i->type == 'C')
        {
        cout << "( C \"" << i->instance_name << "\" \"" << i->name << "\" " << (char)i->pin_type << " )\n";
        }
      }
    }
  }

View *create_library_part(Design *design, string library, string library_part_name)
  {
  // Create part in library
  Lib *li;
  Cell *ce;
  View *vi = 0;
  li = design->libraries.get(library);
  if (!li)
    {
    li = new Lib();
    li->lib_type = 1;
    li->mom = design;
    li->name.name = library;
    design->libraries.add(li->name.name, li);
    }
  ce = li->cells.get(library_part_name);
  if (!ce)
    {
    ce = new Cell();
    ce->name.name = library_part_name;
    ce->mom = li;
    li->cells.add(ce->name.name, ce);
    }
  vi = ce->views.get("netlist");
  if (!vi)
    {
    vi = new View();
    vi->name.name = "netlist";
    vi->mom = ce;
    ce->views.add("netlist", vi);
    }
  else
    {
    /* Should check if view matches */
    vi = 0;
    }
  return vi;
  }

// Convert .INF into netlist

Design *inf_to_net(Design *design, InfDesign *inf)
  {
  int model = 0;
  Lib *lib;
  Cell *cell;
  View *view;

  // Create design if there is none
  if (!design)
    {
    design = new Design();
    design->name.name = inf->name;
    }

  // Create library, add it to design
  lib = design->libraries.get("main");
  if (!lib)
    {
    lib = new Lib();
    lib->lib_type = 0;
    lib->name.name = "main";
    lib->mom = design;
    design->libraries.add("main", lib);
    }

  // Create cell, add it to library
  cell = lib->cells.get(inf->name);
  /* It already exists */
  if (cell)
    {
    cerr << "Cell already exists?\n";
    exit(-1);
    }
  cell = new Cell();
  cell->name.name = inf->name;
  cell->mom = lib;
  lib->cells.add(inf->name, cell);

  // Create view, add it to cell
  view = new View();
  view->name.name = "netlist";
  view->mom = cell;
  cell->views.add("netlist", view);

  // Check for simulation model substitution
  
  InfPipe *pipe = inf->pipes.get("|sim");
  if (pipe)
    {
    Dlist<InfPipeItem *>::ptr item;
    // Copy lines to view
    for (item = pipe->items.first(); item; item++)
      {
      string s;
      int x;
      for (x = 1; x < item->s.length(); ++x)
        s += item->s[x];
      view->sim.add(s);
      }
    // Keep ports, make nets for those ports and forget about instances.
    model = 1;
    }

  // Add Ports
  Hash<InfPort *>::ptr pi;
  for (pi=inf->ports.first(); pi; pi++)
    {
    InfPort *inf_p = *pi;
    Port *p = new Port();
    switch(inf_p->type)
      {
      case 'I':
        p->direction = 0;
        break;
      case 'O':
        p->direction = 1;
        break;
      case 'B':
        p->direction = 2;
        break;
      case 'S':
        p->supply = 1;
        p->direction = 0;
        break;
      default:
        cerr << "Bad direction\n";
        p->direction = 0;
        break;
      }
    p->name.name = inf_p->name;
    p->mom = view;
    view->ports.add(p->name.name, p);
    if (model)
      {
      // Add a net for the port
      Net *net = new Net(); // Create net
      net->name.name = p->name.name; // Same name as port
        Portref *ref = new Portref();
        ref->portRef = p->name.name;
        ref->port = p;
        ref->next = net->pins; net->pins = ref;
      view->nets.add(net->name.name, net); // Add net
      }
    }

  // Add instances
  if (!model)
    {
    Hash<InfInstance *>::ptr ii;
    for (ii=inf->instances.first(); ii; ii++)
      {
      InfInstance *inf_i = *ii;
      Instance *i = new Instance();
      i->name.name = inf_i->name;
      i->mom = view;
      if (inf_i->type == 'R')
        {
        // Create new library part (returns 0 if part already exists).
        // FIXME: it should check if the part matches.
        View *vi = create_library_part(design, inf_i->library, inf_i->library_part_name);

        // Add pins to new library part
        Hash<InfPin *>::ptr pini;
        for (pini=inf_i->pins.first(); pini; pini++)
          {
          InfPin *pin = *pini;
          if (vi)
            {
            Port *p = new Port();
            p->mom = vi;
            p->name.name = pin->name;
            switch (pin->type)
              {
              case 'I':
                {
                p->direction = 0;
                break;
                }
              case 'O':
                {
                p->direction = 1;
                break;
                }
              case 'B':
                {
                p->direction = 2;
                break;
                }
              default:
                {
                p->direction = 0;
                break;
                }
              }
            vi->ports.add(p->name.name, p);
            }
          }

        i->ref.libraryRef = inf_i->library;
        i->ref.cellRef = inf_i->library_part_name;
        i->ref.viewRef = "netlist";

        view->instances.add(i->name.name, i);
        }
      else if (inf_i->type == 'C')
        {
        // Sub-sheet
        int x;
        i->ref.libraryRef = "main";
        i->ref.viewRef = "netlist";
        x = inf_i->sheet_file_name.find('.');
        i->ref.cellRef = inf_i->sheet_file_name.substr(0, x);
        view->instances.add(i->name.name, i);
        // Remember to load sub-sheet
        if (debug) cout << "Remembering to load subsheet " << inf_i->sheet_file_name << "\n";
        char *s = strdup(lower(inf_i->sheet_file_name).c_str());
        char *u = strstr(s, ".sch");
        if (u) strcpy(u, ".inf");
        struct load_stack *k = new load_stack();
        k->name = s;
        k->next = the_load_stack;
        the_load_stack = k;
        }
      }

    // Add nets
    Dlist<InfJoin *>::ptr ji;
    int netno = 1;
    for (ji=inf->joins.first(); ji; ji++)
      {
      InfJoin *j = *ji;
      Dlist<InfJoinItem *>::ptr i;
      if (debug) cout << "Creating net\n";
      Net *net = new Net(); // Create net
      int prio = 5;
      // Add references
      for (i = j->items.first(); i; i++)
        {
        if (debug) cout << "  pin of type " << (char)i->type << "\n";
        switch (i->type)
          {
          case 'S':
            { // There is no such thing as a signal in edif
            if (debug) cout << "  Signal " << i->name << "\n";
            if (prio > 1)
              {
              prio = 1;
              net->name.name = i->name;
              }
            break;
            }
          case 'P':
            { // Module port
            Portref *ref = new Portref();
            if (debug) cout << "  Port " << i->name << "\n";
            ref->portRef = i->name;
            ref->port = view->ports.get(ref->portRef);
            if (ref->port)
              {
              if (debug) cout << "    linked to " << ref->port->name.name << "\n";
              }
            switch (i->pin_type)
              {
              case 'I':
                if (prio > 3)
                  {
                  prio = 3;
                  net->name.name = i->name;
                  }
                break;
              case 'O':
                if (prio > 4)
                  {
                  prio = 4;
                  net->name.name = i->name;
                  }
                break;
              default: // B for bidirectional...
                if (prio > 2)
                  {
                  prio = 2;
                  net->name.name = i->name;
                  }
                break;
              }
            ref->next = net->pins; net->pins = ref;
            break;
            }
          case 'R':
            { // Primitive port
            Portref *ref = new Portref();
            if (debug) cout << "  PrimPin " << i->name << " of " << i->instance_name << "\n";
            ref->portRef = i->name;
            ref->instanceRef = i->instance_name;
            ref->instance = view->instances.get(ref->instanceRef); // Check for failing reference
            ref->next = net->pins; net->pins = ref;
            break;
            }
          case 'C':
            { // Sub-sheet port
            Portref *ref = new Portref();
            if (debug) cout << "  SheetPin " << i->name << " of " << i->instance_name << "\n";
            ref->portRef = i->name;
            ref->instanceRef = i->instance_name;
            ref->instance = view->instances.get(ref->instanceRef); // Check for failing reference
            ref->next = net->pins; net->pins = ref;
            break;
            }
          }
        }
      // No better net names?  Make one up.
      if (prio == 5)
        {
        char buf[20];
        sprintf(buf, "n_%d", netno++);
        net->name.name = buf;
        }
      if (debug) cout << "  it was named " << net->name.name << "\n";
      view->nets.add(net->name.name, net); // Add net
      }
    }
  return design;
  }

void edif_link(Design *d)
  {
  cout << "Linking...\n";
  Lib *lib = d->libraries.get("main");
  if (debug) cout << "got lib\n";
  Hash<Cell *>::ptr cellptr;
  if (!lib)
    {
    cout << "Link error, no \"main\" library\n";
    return;
    }
  else
    {
    if (debug) cout << "library main...\n";
    }
  for (cellptr = lib->cells.first();cellptr;cellptr++)
    {
    Cell *cell = *cellptr;
    Hash<View *>::ptr viewptr;
    if (debug) cout << "Cell " << cell->name.name << "\n";
    for (viewptr = cell->views.first();viewptr;viewptr++)
      {
      View *v = *viewptr;
      Hash<Instance *>::ptr instptr;
      if (debug) cout << "  View " << v->name.name << "\n";
      if (debug) cout << "    Linking instances\n";
      for (instptr = v->instances.first();instptr;instptr++)
        {
        Instance *i = *instptr;
        if (debug) cout << "    Instance " << i->name.name << "\n";
        Lib *li = d->libraries.get(i->ref.libraryRef);
        if (li)
          {
          Cell *ce = li->cells.get(i->ref.cellRef);
          if (ce)
            {
            View *vi = ce->views.get("netlist");
            if (!vi)
              {
              cerr << "Link error: Couldn't find view netlist of cell " << ce->name.name << " ???\n";
              exit(-1);
              }
            i->ref.lib = li;
            i->ref.cell = ce;
            i->ref.view = vi;
            }
          else
            {
            cerr << "Link error: Couldn't find cell " << i->ref.libraryRef << "." << i->name.name << "\n";
            }
          }
        else
          {
          cerr << "Link error: Couldn't find library " << i->ref.libraryRef << "\n";
          }
        }
      if (debug) cout << "    Linking nets\n";
      Hash<Net *>::ptr netptr;
      for (netptr = v->nets.first();netptr;netptr++)
        {
        Net *n = *netptr;
        Portref *pin;
        if (debug) cout << "      Net " << netptr->name.name << "\n";
        for (pin = n->pins; pin; pin = pin->next)
          {
          if (debug) cout << "        Pin " << pin->portRef << " on instance " << pin->instanceRef << "\n";
          Instance *i = pin->instance;
          if (i && i->ref.view)
            {
            if (debug) cout << "      portRef " << pin->portRef << "\n";
            pin->port = i->ref.view->ports.get(pin->portRef);
            if (!pin->port)
              {
              cerr << "Link error: Couldn't find port " << pin->instanceRef << "." << pin->portRef << "\n";
              }
            }
          else
            {
            }
          }
        }
      }
    }
  }

void hookup_supplies(Design *d)
  {
  int flag;
  do
    {
    flag = 0;
    cout << "Supply hookup...\n";
    Lib *lib = d->libraries.get("main");
    Hash<Cell *>::ptr cellptr;
    if (!lib)
      {
      cout << "Supply hookup error, no \"main\" library\n";
      return;
      }
    else
      {
      if (debug) cout << "library main...\n";
      }
    for (cellptr = lib->cells.first();cellptr;cellptr++)
      {
      Cell *cell = *cellptr;
      Hash<View *>::ptr viewptr;
      if (debug) cout << "Cell " << cell->name.name << "\n";
      for (viewptr = cell->views.first();viewptr;viewptr++)
        {
        View *v = *viewptr;
        Hash<Instance *>::ptr instptr;
        if (debug) cout << "  View " << v->name.name << "\n";
        if (debug) cout << "    Linking instances\n";
        for (instptr = v->instances.first();instptr;instptr++)
          {
          Instance *i = *instptr;
          if (debug) cout << "    Instance " << i->name.name << "\n";
          // Look for supply pins on instance
          View *iv = i->ref.view;
          Hash<Port *>::ptr pp;
          for (pp = iv->ports.first(); pp; pp++)
            if (pp->supply)
              {
              Port *p = *pp;
              // We have a supply pin in an instance.  Make sure it's hooked up to something.
              if (!find_net_with_port(v, i, p))
                {
                // Supply pin is not hooked up.
                // Does supply pin exist in this view?
                Port *fp = v->ports.get(p->name.name);
                if (!fp)
                  { // It does not exist: add it
                  fp = new Port();
                  fp->name.name = p->name.name;
                  fp->mom = v;
                  fp->supply = 1;
                  fp->direction = 0;
                  v->ports.add(fp->name.name, fp);
                  }
                // Find net with this supply pin
                Hash<Net *>::ptr np = find_net_with_port(v, NULL, fp);
                Net *net;
                if (!np)
                  { // It does not exist: add it
                  net = new Net();
                  net->name.name = fp->name.name; // Give it same name as port (should check if it already exists!)
                  net->mom = v;
                  // Add supply pin to net
                  Portref *pin = new Portref();
                    pin->portRef = fp->name.name;
                    pin->port = fp;
                  net->pins = pin;
                  v->nets.add(net->name.name, net);
                  }
                else
                  net = *np;
                // Add supply pin to net
                Portref *pin = new Portref();
                pin->portRef = p->name.name;
                pin->instanceRef = i->name.name;
                pin->port = p;
                pin->instance = i;
                pin->next = net->pins;
                net->pins = pin;
                // Set flag for multi-pass
                flag = 1;
                }
              }
          }
        }
      }
    } while(flag);
  }

Design *inf_load(const char *name)
  {
  Design *d = 0;
  InfDesign *v;
  loop:
  cout << "Loading " << name << "\n";
  v = inf_load_1(name);
  if (debug) cout << "Convert inf to net " << name << "\n";
  if (v)
    d = inf_to_net(d, v);
  if (the_load_stack)
    {
    load_stack *k = the_load_stack;
    the_load_stack = k->next;
    name = k->name;
    goto loop;
    }
  if (d)
    {
    edif_link(d);
    hookup_supplies(d);
    }
  return d;
  }


/*
--- How to name nets:
  If there is a signal name (an explicit net name), use it.
  If there is an inout port, use it.
  If there is an input port, use it.
  If there is an output port, use it.
  Otherwise,
    generate a name.
*/

// Test .INF loader
/*
int main(int argc, char *argv[])
  {
  InfDesign *v = inf_load_1(argv[1]);
  InfDump(v, cout);
  }
*/
