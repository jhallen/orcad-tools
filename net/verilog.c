// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.

#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include <stdlib.h>

using namespace std;

#include "hash.h"
#include "dlist.h"
#include "lisp.h"
#include "net.h"
#include "verilog.h"

string lowerize_string(string s)
  {
  int x;
  for (x = 0; x != s.length(); ++x)
    if (s[x] >= 'A' && s[x] <= 'Z')
      s[x] += 'a' - 'A';
    else if (s[x] == ' ' || s[x] == '/' || s[x] == '.' || s[x] == '\\')
      s[x] = '_';
  return s;
  }

string legalize_string(string s)
  {
  int x;
  for (x = 0; x != s.length(); ++x)
    {
    if (s[x] == ' ')
      s[x] = '_';
    else if (s[x] >= 'A' && s[x] <= 'Z')
      s[x] += 'a' - 'A';
    }
  for (x = 0; x != s.length(); ++x)
    if (!(s[x] >= 'A' && s[x] <= 'Z' ||
          s[x] >= 'a' && s[x] <= 'z' ||
          s[x] >= '0' && s[x] <= '9' ||
          s[x] == '_'))
      break;
  if (x != s.length())
    {
    return "\\" + s + " ";
    }
  return s;
  }

void do_module(ostream& out, Cell *c, View *v)
  {
  Hash<Port *>::ptr pp;
  Hash<Instance *>::ptr ip;
  Hash<Net *>::ptr np;


  // Determine port names
  for (pp = v->ports.first(); pp; pp++)
    {
    pp->emit_name = legalize_string(pp->name.name);
    }

  // Determine net names
  for (np = v->nets.first(); np; np++)
    {
    pp = v->ports.find(np->name.name);
    if (pp && !find_port_in_net(*np, NULL, *pp))
      // Rename net if there is a port with same name which is not part of it
      np->emit_name = legalize_string("n_" + np->name.name);
    else
      np->emit_name = legalize_string(np->name.name);
    }

  // Emit module
  out << "// " << c->name.name << '\n';
  out << "\nmodule " << legalize_string(c->name.name) << '\n';
  out << "  (\n";

  for (pp = v->ports.first(); pp; pp++)
    {
    if (pp.next())
      out << "  " << pp->emit_name << ",\n";
    else
      out << "  " << pp->emit_name << "\n";
    }
  out << "  );\n\n";

  // Declare ports...
  out << "// Declare ports\n";
  for (pp = v->ports.first(); pp; pp++)
    {
    switch(pp->direction)
      {
      case 0:
        {
        out << "input " << pp->emit_name << ";\n";
        break;
        }
      case 1:
        {
        out << "output " << pp->emit_name << ";\n";
        break;
        }
      case 2:
        {
        out << "inout " << pp->emit_name << ";\n";
        break;
        }
      }
    }
  out << "\n";

  // Declare nets...
  out << "// Declare nets\n";
  for (np = v->nets.first(); np; np++)
    {
    out << "wire " << np->emit_name << ";\n";
    }
  out << "\n";

  // Connect ports to nets
  out << "// Connect ports to nets\n";
  for (pp = v->ports.first(); pp; pp++)
    {
    np = find_net_with_port(v, NULL, *pp);
    if (np)
      {
      if (np->emit_name == pp->emit_name)
        out << "// port name == net name == " << np->emit_name << "\n";
      else
        switch (pp->direction)
          {
          case 0: // Input
            {
            out << "assign " << np->emit_name << " = " << pp->emit_name << ";\n";
            break;
            }
          case 1: // Output
            {
            out << "assign " << pp->emit_name << " = " << np->emit_name << ";\n";
            break;
            }
          case 2: // InOut
            {
            out << "// ERROR inout port and net with different names: " << pp->emit_name << " " << np->emit_name << "\n";
            break;
            }
          }
      }
    }
  out << "\n";

  // Emit direct copy lines
  if (v->sim.len())
    {
    out << "// Code copied from schematic |sim lines\n";
    for (Dlist<string>::ptr sp = v->sim.first(); sp; ++sp)
      out << *sp << "\n";
    out << "\n";
    }

  // Emit instances
  out << "// Instances\n";
  for(ip=v->instances.first();ip;ip++)
    {
    Instance *l=*ip;
    View *vi;
    // out << "    Instance " << l->name.name << " of " << l->ref.libraryRef << "." << l->ref.cellRef << '\n';
    out << legalize_string(l->ref.cellRef) << " " << legalize_string(l->name.name) << "\n";
    out << "  (\n";
    vi = l->ref.view;
    if (vi)
      {
      Hash<Port *>::ptr portptr, nportptr;
      for(portptr=vi->ports.first();portptr;portptr = nportptr)
        {
        nportptr = portptr.next();
        Port *p=*portptr;
        if (nportptr)
          {
          out << "  ." << legalize_string(p->name.name) << " (" << find_my_wire(v, l, p) << "),\n";
          }
        else
          {
          out << "  ." << legalize_string(p->name.name) << " (" << find_my_wire(v, l, p) << ")\n";
          }
        }
      }
    else
      {
      out << "  // Couldn't not find this part - broken reference.\n";
      }
    out << "  );\n\n";
    }
  out << "\nendmodule\n";
  }

void verilog_dump(Design *d, char *path, ostream& out)
  {
  if (!path) out << "// Design " << d->name.name << '\n';
  Hash<Lib *>::ptr lptr;
  for (lptr=d->libraries.first(); lptr; lptr++)
    {
    Lib *l = *lptr;
    Hash<Cell *>::ptr cptr;
    // out << "  Library " << l->name.name << '\n';
    if (!l->lib_type) for (cptr=l->cells.first();cptr;cptr++)
      {
      Cell *c = *cptr;
      // out << "    Cell " << c->name.name << '\n';
      Hash<View *>::ptr vptr;
      for (vptr=c->views.first();vptr;vptr++)
        {
        View *v = *vptr;
        fstream fout;
        if (path)
          {
          string p = path;
          string name = p + "/" + lowerize_string(c->name.name) + ".v";
          fout.open(name.c_str(),ios::out);
          if (!fout)
            {
            cerr << "couldn't open " << name << "\n";
            exit(-1);
            }
          do_module(fout, c, v);
          fout.close();
          if (!fout)
            {
            cerr << "close error " << name << "\n";
            }
          }
        else
          {
          do_module(out, c, v);
          }
        }
      }
    }
  }
