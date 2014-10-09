// Netlist
// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.

struct Design;
struct Lib;
struct Cell;
struct View;
struct Port;
struct Instance;
struct Net;

// A name

struct Name
  {
  string name;				// Simple legal name
  string fullname;			// Possible string version of name
  int hasfull;				// Set if we have the string version
  int array;				// Non-zero if this is an array
  Name();
  };

// Reference to a view

struct Viewref
  {
  string viewRef;			// View name
  string cellRef;			// Cell name
  string libraryRef;			// Library name
  View *view;				// Linked target
  Cell *cell;
  Lib *lib;
  Viewref();
  };

// Reference to a port

struct Portref
  {
  Portref *next;			// Port refs are in a list
  string portRef;			// Name of port
  string instanceRef;			// Name of instance
  int member;				// -1 or 0-n for array reference
  Port *port;				// Linked target
  Instance *instance;			// Linked target
  Portref();
  };

// An entire design

struct Design
  {
  Design *next;
  Name name;
  Hash<Lib *> libraries;		// Libraries which make up design
  Design();
  };

// A design is a bunch of libraries

struct Lib
  {
  Lib *next;
  int lib_type;				// 0 = Design library, 1 = external library
  Name name;
  Design *mom;				// Parent
  Hash<Cell *> cells;			// Library is composed of cells
  Lib();
  };

// A library is a bunch of Cells

struct Cell
  {
  Cell *next;
  Name name;
  Lib *mom;				// Parent
  Hash<View *> views;			// Cell is composed of views
  Cell();
  };

// Simulation lines

// A cell is a bunch of views

struct View
  {
  View *next;
  Name name;
  Cell *mom;				// Parent
  Hash<Port *> ports;			// Interface
  Hash<Instance *> instances;		// Instances
  Hash<Net *> nets;			// Nets
  Dlist<string> sim;			// Verilog simulation copy-in text
  View();
  };

// A view has Ports (or pins)

struct Port
  {
  Name name;
  string emit_name;
  View *mom;
  int direction;	// 0=in, 1=out, 2=inout
  int supply;		// Set if this is a supply pin
  Port();
  };

// A view has Instances

struct Instance
  {
  Instance *next;
  Name name;
  string emit_name;
  View *mom;				// View we're in
  Viewref ref;				// View we reference
  Instance();
  };

// A view has nets

struct Net
  {
  Net *next;
  Name name;
  string emit_name;
  View *mom;
  Portref *pins;			// Connected pins
  Net();
  };

string find_my_wire(View *v, Instance *i, Port *p);
Hash<Net *>::ptr find_net_with_port(View *v, Instance *i, Port *p);
Portref *find_port_in_net(Net *l, Instance *i, Port *p);
void net_dump(Design *d, ostream& out);
string lower(string ss);
