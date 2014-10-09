// Netlist functions
// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.

#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>

using namespace std;

extern string legalize_string(string s);

#include "hash.h"
#include "dlist.h"
#include "lisp.h"
#include "net.h"

string lower(string ss)
  {
  int x;
  string s = ss;
  for (x = 0; x != s.length(); ++x)
    s[x] = tolower(s[x]);
  return s;
  }

Name::Name()
  {
  array = 0;
  hasfull = 0;
  }

Portref::Portref()
  {
  port=0;
  instance=0;
  member= -1;
  next=0;
  }

Viewref::Viewref()
  {
  view = 0;
  cell = 0;
  lib = 0;
  }

Design::Design()
  {
  next = 0;
  }

Lib::Lib()
  {
  next = 0;
  mom = 0;
  }

Cell::Cell()
  {
  next = 0;
  mom = 0;
  }

View::View()
  {
  next = 0;
  mom = 0;
  }

Port::Port()
  {
  mom = 0;
  direction= -1;
  supply = 0;
  }

Instance::Instance()
  {
  next = 0;
  mom = 0;
  }

Net::Net()
  {
  next = 0;
  mom = 0;
  pins = 0;
  }

Hash<Net *>::ptr find_net_with_port(View *v, Instance *i, Port *p)
  {
  Hash<Net *>::ptr netptr;
  for(netptr=v->nets.first();netptr;netptr++)
    {
    Net *l= *netptr;
    Portref *pin;
    for (pin = l->pins; pin; pin = pin->next)
      {
      if (pin->port == p && pin->instance == i)
        return netptr;
      }
    }
  return netptr;
  }

Portref *find_port_in_net(Net *l, Instance *i, Port *p)
  {
  Portref *pin;
  for (pin = l->pins; pin; pin = pin->next)
    {
    if (pin->port == p && pin->instance == i)
      break;
    }
  return pin;
  }

string find_my_wire(View *v, Instance *i, Port *p)
  {
  Hash<Net *>::ptr netptr = find_net_with_port(v, i, p);
  if (netptr)
    return netptr->emit_name;
//    return legalize_string(netptr->name.name);
  else
    return "";
  }

void net_dump(Design *d, ostream& out)
  {
  out << "Design " << d->name.name << '\n';
  Hash<Lib *>::ptr lptr;
  for (lptr=d->libraries.first(); lptr; lptr++)
    {
    Lib *l = *lptr;
    Hash<Cell *>::ptr cptr;
    out << "  Library " << l->name.name << '\n';
    for (cptr=l->cells.first(); cptr;cptr++)
      {
      Cell *c = *cptr;
      // out << "    Cell " << c->name.name << '\n';
      Hash<View *>::ptr vptr;
      for (vptr=c->views.first(); vptr;vptr++)
        {
        View *v = *vptr;
        Hash<Port *>::ptr portptr, nportptr;
        Hash<Instance *>::ptr instanceptr;
        Hash<Net *>::ptr netptr;
        // out << "      View " << v->name.name << '\n';
        out << "    Cell " << l->name.name << '.' << c->name.name << '\n';
        for(portptr=v->ports.first(); portptr;portptr++)
          {
          Port *l= *portptr;
          out << "      Port " << l->name.name;
          switch (l->direction)
            {
            case 0:
              out << " (input)\n";
              break;
            case 1:
              out << " (output)\n";
              break;
            case 2:
              out << " (inout)\n";
              break;
            }
          }
        for(netptr=v->nets.first(); netptr;netptr++)
          {
          Net *l= *netptr;
          out << "      Net " << l->name.name << '\n';
          for (Portref *pin = l->pins; pin; pin = pin->next)
            {
            if (pin->instanceRef != "")
              {
              out << "        Port " << pin->portRef << " of " << pin->instanceRef;
              if (pin->port)
                out << " [Port=" << pin->port->name.name;
              else
                out << " [Port=<unlinked>";
              if (pin->instance)
                out << " Instance=" << pin->instance->name.name << "]";
              else
                out << " Instance=<unlinked>]";
              out << "\n";
              }
            else
              {
              out << "        Port " << pin->portRef;
              if (pin->port)
                out << " [Port=" << pin->port->name.name << "]";
              else
                out << " [Port=<unlinked>]";
              out << "\n";
              }
            }
          }
        for(instanceptr=v->instances.first(); instanceptr;instanceptr++)
          {
          Instance *l= *instanceptr;
          View *vi;
          out << "      Instance " << l->name.name << " of " << l->ref.libraryRef << "." << l->ref.cellRef;
          if (l->ref.lib)
            out << " [Lib=" << l->ref.lib->name.name;
          else
            out << " [Lib=<unlinked>";
          if (l->ref.cell)
            out << " Cell=" << l->ref.cell->name.name;
          else
            out << " Cell=<unlinked>";
          if (l->ref.view)
            out << " View=" << l->ref.view->name.name << "]";
          else
            out << " View=<unlinked>]";
          out << '\n';
          }
        }
      }
    }
  }
