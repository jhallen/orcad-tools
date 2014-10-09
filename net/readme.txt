
'netlist' is a program which converts DOS OrCAD .INF files into Verilog.  It
allows you to simulate a digital system designed with schematics using
verilog simulators, for example using the free icarus verilog.

    Copyright (C) 2008  Joseph H. Allen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  - Joe Allen jhallen@world.std.com

Usage:

netlist -ifmt orcad_inf -ofmt verilog TOP.INF -opath PATH

  TOP.INF is the name of your top-level .INF file.

  PATH is a path to an output directory.  Each .INF file
  will be converted to a .v file in this directory.

Direct verilog inclusion:

  Sometimes you will want to simulate a model in place of a sheet instead of
  the simulating the primitives and subsheets on the sheet itself.  This can
  be done using the OrCAD pipe mechanism as follows:

    Edit the sheet with 'draft'.  Use p t to place text in this
    format:

      |sim
      |// This sheet is a flip-flop
      |reg q;
      |always @(posedge clk)
      |  q <= d;

   Note that the '|sim' is a required keyword and that the |
   symbols must all line up vertically.

   When 'netlist' finds this, it includes the verilog directly into the .v file
   and leaves out all instantiations.

Names

   'netlist' converts all names to lower case.

   'netlist' properly quotes characters which are allowed
   in OrCAD but not in verilog.  These will end up like this:
   \fr$ed 

   DOS OrCAD's bus concept does not match Verilog's.  In DOS OrCAD a bus
   like D[7..0] is exactly the same as 8 wires named D7 D6 D5 D4 D3 D2 D1
   D0.  These wires will be placed in the verilog output instead of
   something like 'wire [7:0] D;'

How to get .INF files from .SCH files using OrCAD commands:

  annotate mydesign.sch
  inet mydesign.sch

How to build:

  Install Cygwin, including gcc C++ compiler.

  In the Cygwin shell, go to the directory with the source
  files.  Type cd /cygdrive/c/net if the files are in
  C:\net

  Type Make.  This will create net.exe.

  Net.exe can be invoked from the cygwin shell or from
  cmd.com.
