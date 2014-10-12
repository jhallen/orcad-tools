// Edif parser

// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.


#include <iostream>
#include <fstream>
#include <string>
#include <ctype.h>
#include <stdlib.h>

using namespace std;

extern string legalize_string(string s);

#include "hash.h"
#include "dlist.h"
#include "lisp.h"
#include "net.h"
#include "edif.h"

// Recursive descent parser
// Convert dumb lisp parse-tree into real structured edif parse-tree

Name parsename(Node *n)
  {
  Name name;
  if(n->type==Ident_id)
    { /* Simple name */
    name.name=lower(n->ident()->s);
    name.array=0;
    name.fullname="";
    name.hasfull=0;
    }
  else if(n->type==List_id && n->list()->item->type==Ident_id &&
          n->list()->item->ident()->s=="array" &&
          n->list()->item->next->next->type==Num_id)
    { /* Array */
    name=parsename(n->list()->item->next);
    name.array=n->list()->item->next->next->num()->num;
    }
  else if(n->type==List_id && n->list()->item->type==Ident_id &&
          n->list()->item->ident()->s=="rename" &&
          n->list()->item->next->type==Ident_id && n->list()->item->next->next->type==Str_id)
    { /* Renamed name */
    name.name=lower(n->list()->item->next->ident()->s);
    name.array=0;
    name.fullname=n->list()->item->next->next->str()->s;
    name.hasfull=1;
    }
  else
    {
    cerr << n->file_name << " " << n->line << ": Error: unknown name format\n";
    exit(-1);
    }
  return name;
  }

string parserefname(Node *n)
  {
  if(n->type==Ident_id)
    { /* Simple name */
    return lower(n->ident()->s);
    }
  else
    {
    cerr << n->file_name << " " << n->line << ": Error: bad ref name\n";
    exit(-1);
    }
  }

Port *parseport(View *v,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="port")
    {
    Port *port=new Port();
    port->mom=v;
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing port name\n";
      exit(-1);
      }
    port->name=parsename(n->list()->item->next);
    if(n->list()->item->next->next && n->list()->item->next->next->type==List_id &&
       n->list()->item->next->next->list()->item->type==Ident_id &&
       n->list()->item->next->next->list()->item->ident()->s=="direction" &&
       n->list()->item->next->next->list()->item->next->type==Ident_id)
      {
      if(n->list()->item->next->next->list()->item->next->ident()->s=="INPUT")
        port->direction=0;
      else if(n->list()->item->next->next->list()->item->next->ident()->s=="OUTPUT")
        port->direction=1;
      else if(n->list()->item->next->next->list()->item->next->ident()->s=="INOUT")
        port->direction=2;
      else
        {
        cerr << n->file_name << " " << n->line << ": Error: unknown direction on port\n";
        exit(-1);
        }
      // port->misc_after=n->list()->item->next->next->next;
      return port;
      }
    else
      {
      cerr << n->file_name << " " << n->line << ": Error: bad or missing direction on port\n";
      exit(-1);
      }
    }
  else
    return 0;
  }

void parseportname(Portref *i,Node *n)
  {
  if(n->type==Ident_id)
    {
    i->member = -1;
    i->portRef=lower(n->ident()->s);
    }
  else if(n->type==List_id && n->list()->item->type==Ident_id &&
          n->list()->item->ident()->s=="member")
    {
    if(!n->list()->item->next || n->list()->item->next->type!=Ident_id)
      {
      cerr << n->file_name << " " << n->line << ": Error: bad or missing portref name\n";
      exit(-1);
      }
    i->portRef=lower(n->list()->item->next->ident()->s);
    if(!n->list()->item->next->next || n->list()->item->next->next->type!=Num_id)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing number in member\n";
      exit(-1);
      }
    i->member=n->list()->item->next->next->num()->num;
    if(n->list()->item->next->next->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: extra junk in member\n";
      exit(-1);
      }
    }
  else
    {
    cerr << n->file_name << " " << n->line << ": Error: bad portRef name\n";
    exit(-1);
    }
  }

int parseinstanceref(View *v,Portref *i,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="instanceRef")
    {
    if(!n->list()->item->next || n->list()->item->next->type!=Ident_id)
      {
      cerr << n->file_name << " " << n->line << ": Error: Missing or bad instanceRef name\n";
      exit(-1);
      }
    i->instanceRef=lower(n->list()->item->next->ident()->s);
    if(n->list()->item->next->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: Extra junk after instanceRef name\n";
      exit(-1);
      }
    // Link
    i->instance=v->instances.get(i->instanceRef);
    if(!i)
      {
      cerr << n->file_name << " " << n->line << ": Error: Couldn't find instanceRef- used before defined?\n";
      exit(-1);
      }
    return 1;
    }
  else
    return 0;
  }

Portref *parseportref(View *v,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="portRef")
    {
    Portref *i=new Portref();
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: Missing port name\n";
      exit(-1);
      }
    parseportname(i,n->list()->item->next);
    if(n->list()->item->next->next)
      {
      if(!parseinstanceref(v,i,n->list()->item->next->next) || n->list()->item->next->next->next)
        {
        cerr << n->file_name << " " << n->line << ": Error: Extra junk in portRef\n";
        exit(-1);
        }
      }
    // Link
    if(i->instance)
      { // Find port in instance
      if(!i->instance->ref.view)
        {
        cerr << n->file_name << " " << n->line << ": Error: no viewref in instance?\n";
        exit(-1);
        }
      i->port=i->instance->ref.view->ports.get(i->portRef);
      }
    else
      { // Find port in v
      i->port=v->ports.get(i->portRef);
      }
    if(!i->port)
      {
      cerr << n->file_name << " " << n->line << ": Error: couldn't link port " << i->instanceRef << "." << i->portRef <<"\n";
      exit(-1);
      }
    return i;
    }
  else
    return 0;  
  }

int parsejoin(View *v,Net *i,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="joined")
    {
    for(n=n->list()->item->next;n;n=n->next)
      {
      Portref *r=parseportref(v,n);
      if(!r)
        {
        cerr << n->file_name << " " << n->line << ": Error: Unknown item in joined\n";
        exit(-1);
        }
      r->next=i->pins;
      i->pins=r;
      }
    return 1;
    }
  else
    return 0;
  }

Net *parsenet(View *v,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="net")
    {
    Net *i=new Net();
    i->mom=v;
    i->pins=0;
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: Missing net name\n";
      exit(-1);
      }
    i->name=parsename(n->list()->item->next);
    if(!n->list()->item->next->next || !parsejoin(v,i,n->list()->item->next->next))
      {
      cerr << n->file_name << " " << n->line << ": Error: Empty net\n";
      exit(-1);
      }
    // i->misc_after=n->list()->item->next->next->next;
/*    if(n->list()->item->next->next->next)
      {
      cerr << "Extra junk in net\n";
      lisp_dump(n, cerr);
      cerr << '\n';
      } */
    return i;
    }
  else
    return 0;
  }

int parselibraryref(View *v,Viewref *r,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="libraryRef")
    {
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing libraryRef name\n";
      exit(-1);
      }
    r->libraryRef=parserefname(n->list()->item->next);
    if(n->list()->item->next->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: extra junk at end of libraryRef list\n";
      exit(-1);
      }
    r->lib=v->mom->mom->mom->libraries.get(r->libraryRef);
    if(!r->lib)
      {
      cerr << n->file_name << " " << n->line << ": Error: forward ref or unknown library\n";
      exit(-1);
      }
    return 1;
    }
  else
    return 0;
  }

int parsecellref(View *v,Viewref *r,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="cellRef")
    {
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: Missing cellRef name\n";
      exit(-1);
      }
    r->cellRef=parserefname(n->list()->item->next);
    if(!n->list()->item->next->next || !parselibraryref(v,r,n->list()->item->next->next))
      {
      /*
      cerr << "Missing libraryRef\n";
      n->show(cerr,0,0);
      cerr << '\n';
      exit(-1); - this is ok- it means cell is in current library */
      r->libraryRef="";
      r->lib=v->mom->mom;
      if(n->list()->item->next->next)
        {
        cerr << n->file_name << " " << n->line << ": Error: Extra junk at end of cellRef list\n";
        exit(-1);
        }
      r->cell=r->lib->cells.get(r->cellRef);
      if(!r)
        {
        cerr << n->file_name << " " << n->line << ": Error: forward ref to or missing cell\n";
        exit(-1);
        }
      return 1;
      }
    if(n->list()->item->next->next->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: Extra junk at end of cellRef list\n";
      exit(-1);
      }
    r->cell=r->lib->cells.get(r->cellRef);
    if(!r)
      {
      cerr << n->file_name << " " << n->line << ": Error: forward ref to or missing cell\n";
      exit(-1);
      }
    return 1;
    }
  else
    return 0;
  }

int parseviewref(View *v,Viewref *r,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="viewRef")
    {
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing viewRef name\n";
      exit(-1);
      }
    r->viewRef=parserefname(n->list()->item->next);
    if(!n->list()->item->next->next || !parsecellref(v,r,n->list()->item->next->next))
      {
      cerr << n->file_name << " " << n->line << ": Error: missing cellRef in viewRef\n";
      exit(-1);
      }
    if(n->list()->item->next->next->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: extra junk at end of viewRef list\n";
      exit(-1);
      }
    r->view=r->cell->views.get(r->viewRef);
    if(!r->view)
      {
      cerr << n->file_name << " " << n->line << ": Error: forward ref to or missing view in cell\n";
      exit(-1);
      }
    return 1;
    }
  else
    return 0;
  }

Instance *parseinstance(View *v,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="instance")
    {
    Instance *i=new Instance();
    i->mom=v;
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing instance name\n";
      exit(-1);
      }
    i->name=parsename(n->list()->item->next);
    if(!n->list()->item->next->next || !parseviewref(v,&i->ref,n->list()->item->next->next))
      {
      cerr << n->file_name << " " << n->line << ": Error: missing view reference in instance\n";
      exit(-1);
      }
    // i->misc_after=n->list()->item->next->next->next;
    return i;
    }
  else
    return 0;
  }

int parseinterface(View *v,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="interface")
    {
    Node *first, *last;
    Node *next;
    Port *port;
    first=last=0;
    for(n=n->list()->item->next;n;n=next)
      {
      next=n->next;
      if(port=parseport(v,n))
        {
        port->mom=v;
        v->ports.add(port->name.name,port);
        }
      else
        { // Add to misc
        n->next=last, last=n;
        if(!first) first=last;
        }
      }
    if(first)
      {
      cerr << n->file_name << " " << n->line << ": Error: unexpected junk in interface\n";
      exit(-1);
      }
    return 1;
    }
  else
    return 0;
  }

int parsecontents(View *v,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="contents")
    {
    Node *first, *last;
    Node *next;
    Net *net;
    Instance *instance;
    first=last=0;
    for(n=n->list()->item->next;n;n=next)
      {
      next=n->next;
      if(net=parsenet(v,n))
        {
        net->mom=v;
        v->nets.add(net->name.name,net);
        }
      else if(instance=parseinstance(v,n))
        {
        instance->mom=v;
        v->instances.add(instance->name.name,instance);
        }
      else
        { // Add to misc
        n->next=last, last=n;
        if(!first) first=last;
        }
      }
    if(first)
      {
      cerr << n->file_name << " " << n->line << ": Error: unexpected junk in contents\n";
      exit(-1);
      }
    return 1;
    }
  else
    return 0;
  }

View *parseview(Cell *cell,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="view")
    {
    int flg=0;
    Node *first, *last;
    Node *next;
    View *view;
    view=new View();
    view->mom=cell;
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing view name\n";
      exit(-1);
      }
    view->name=parsename(n->list()->item->next);
    first=last=0;
    for(n=n->list()->item->next->next;n;n=next)
      {
      next=n->next;
      if(parseinterface(view,n) || parsecontents(view,n))
        {
        if(!flg)
          {
          // view->misc_before=first;
          first=last=0;
          flg=1;
          }
        }
      else
        { // Add to misc
        n->next=last, last=n;
        if(!first) first=last;
        }
      }
    // view->misc_after=first;
    return view;
    }
  return 0;
  }

Cell *parsecell(Lib *lib,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     n->list()->item->ident()->s=="cell")
    {
    int flg=0;
    Node *first, *last;
    Node *next;
    Cell *cell;
    View *v;
    cell=new Cell();
    cell->mom=lib;
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing cell name\n";
      exit(-1);
      }
    cell->name=parsename(n->list()->item->next);
//    cout << "Parsing " << cell->name.name << "\n";
    first=last=0;
    for(n=n->list()->item->next->next;n;n=next)
      {
      next=n->next;
      if(v=parseview(cell,n))
        {
        if(!flg)
          {
          // cell->misc_before=first;
          first=last=0;
          flg=1;
          }
        // Add to hash table
        cell->views.add(v->name.name,v);
        }
      else
        { // Add to misc
        n->next=last, last=n;
        if(!first) first=last;
        }
      }
    // cell->misc_after=first;
    return cell;
    }
  return 0;
  }

Lib *parselibrary(Design *e,Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id &&
     (n->list()->item->ident()->s=="library" ||
      n->list()->item->ident()->s=="external"))
    {
    int flg=0;
    Node *first, *last;
    Node *next;
    Lib *lib;
    Cell *l;
    lib = new Lib();
    if(n->list()->item->ident()->s=="library") lib->lib_type = 0;
    else lib->lib_type = 1;
    lib->mom=e;
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing library name\n";
      exit(-1);
      }
    lib->name=parsename(n->list()->item->next);
    first=last=0;
    for(n=n->list()->item->next->next;n;n=next)
      {
      next=n->next;
      if(l=parsecell(lib,n))
        {
        if(!flg)
          {
          // lib->misc_before=first;
          first=last=0;
          flg=1;
          }
        // Add to hash table
        lib->cells.add(l->name.name,l);
        }
      else
        { // Add to misc
        n->next=last, last=n;
        if(!first) first=last;
        }
      }
    // lib->misc_after=first;
    return lib;
    }
  return 0;
  }

Design *parse_edif(Node *n)
  {
  if(n->type==List_id && n->list()->item->type==Ident_id && n->list()->item->ident()->s=="edif")
    {
    int flg=0;
    Node *first, *last;
    Node *next;
    Design *edif=new Design();
    Lib *l;
    if(!n->list()->item->next)
      {
      cerr << n->file_name << " " << n->line << ": Error: missing edif name\n";
      exit(-1);
      }
    edif->name=parsename(n->list()->item->next);
    first=last=0;
    for(n=n->list()->item->next->next;n;n=next)
      {
      next=n->next;
      if(l=parselibrary(edif,n))
        {
        if(!flg)
          {
          // edif->misc_before=first;
          first=last=0;
          flg=1;
          }
        // Add to hash table
        edif->libraries.add(l->name.name,l);
        }
      else
        { // Add to misc
        n->next=last, last=n;
        if(!first) first=last;
        }
      }
    // edif->misc_after=first;
    return edif;
    }
  return 0;
  }
