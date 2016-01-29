// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
//#include <strstream>
using namespace std;

#include "lisp.h"
#include "hash.h"
#include "dlist.h"
#include "net.h"
#include "edif.h"
#include "inf.h"
#include "verilog.h"

int debug;
extern int orcad_edif_bug;

enum {
  NONE,
  VERILOG,
  VHDL,
  INF,
  EDIF,
  NET
};

int ifmt = NONE;
int ofmt = NONE;
char *in_name;

int main(int argc,char *argv[])
  {
  char *opath = 0;
  int x;
  Node *e;
  Design *d;
  string cmd;
  string s;

  if(argc < 2)
    {
    goto show_help;
    }

  for (x = 1; x != argc; ++x)
    if (!strcmp(argv[x], "-ifmt"))
      {
      ++x;
      if (!strcmp(argv[x], "orcad_inf"))
        ifmt = INF;
      else if (!strcmp(argv[x], "edif"))
        ifmt = EDIF;
      else if (!strcmp(argv[x], "orcad_edif"))
        {
        ifmt = EDIF;
        orcad_edif_bug = 1;
        }
      else if (!strcmp(argv[x], "verilog"))
        ifmt = VERILOG;
      else
        {
        cerr << "unknown input format\n";
        return -1;
        }
      }
    else if (!strcmp(argv[x], "-v"))
      {
      debug = 1;
      }
    else if (!strcmp(argv[x], "-ofmt"))
      {
      ++x;
      if (!strcmp(argv[x], "net"))
        ofmt = NET;
      else if (!strcmp(argv[x], "verilog"))
        ofmt = VERILOG;
      else
        {
        cerr << "unknown output format\n";
        return -1;
        }
      }
    else if (!strcmp(argv[x], "-opath"))
      {
      opath = argv[++x];
      }
    else if (!strcmp(argv[x], "-h"))
      {
      show_help:
      cout << "netlist -ifmt [orcad_inf|edif|orcad_edif] -ofmt [net|verilog] name [-opath path]\n";
      cout << "  Orcad_edif is for edif output from DOS OrCAD which has string escaping bug\n";
      cout << "  For simple netlist (net) output, -opath gives output file name\n";
      cout << "  For verilog output, -opath gives output directory\n";
      cout << "  Version 3 - by Joe Allen jhallen@world.std.com\n";
      return 0;
      }
    else if (argv[x][0]=='-')
      {
      cerr << "unknown option " << argv[x] << "\n";
      return -1;
      }
    else
      {
      if (in_name)
        {
        cerr << "input file name already given\n";
        return -1;
        }
      else
        {
        in_name = argv[x];
        }
      }

  if (!in_name)
    {
    cerr << "missing input file name\n";
    return -1;
    }

  switch (ifmt)
    {
    case NONE:
      {
      cerr << "no input format specified\n";
      return -1;
      }
    case INF:
      {
      d = inf_load(in_name);
      break;
      }
    case EDIF:
      {
      e = lisp_load(in_name);
      d = parse_edif(e);
      break;
      }
    default:
      {
      cerr << "input format not supported yet\n";
      return -1;
      }
    }

  switch (ofmt)
    {
    case NONE:
      {
      return 0;
      }
    case NET:
      {
      if (opath)
        {
        fstream l;
        l.open(opath,ios::out);
        if (!l)
          {
          cerr << "couldn't open " << opath << "\n";
          return -1;
          }
        net_dump(d, l);
        l.close();
        if (!l)
          {
          cerr << "close error\n";
          return -1;
          }
        return 0;
        }
      else
        net_dump(d, cout);
      return 0;
      }
    case VERILOG:
      {
      verilog_dump(d, opath, cout);
      return 0;
      }
    default:
      {
      cerr << "output format not supported yet\n";
      return -1;
      }
    }
  }
