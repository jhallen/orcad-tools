// OrCAD .INF file
// Copyright (C) 2008 Joseph H. Allen
// See file COPYING for license.

// Port

// `P module_port_type "module_port_name"

struct InfPort
  {
  string name; // From quoted string
  int type; // 'I'nput, 'O'utput, 'B'idirectional, 'U'nspecified, 'S'upply,
  };

// External library

// `E library_name

struct InfExtern
  {
  string name; // External library name, like TTL.LIB
  };

// Part pin or child pin

struct InfPin
  {
  string name; // child sheet-net name or part pin-name
  string pin_number; // part pin-number, unused for sheet-net
  int type; // 'I'nput, 'O'utput, 'B'idirectional, 'S'upply, 'P'assive, 'T'hree-state, open-'C'ollector, open-'E'mitter
  };

// An instance

// `I R "part_value" library "library_part_name" absolute_identifier
//       part_reference [sub_part_code] "part_field_1" ... "part_field_8"
//       ( "pin_name_1" ?pin_number_1? pin_type_1 ) ...
// `I C "sheet_file_name" absolute_identifier "sheet_name" ( "sheet_net_name_1
//       sheet_net_type_1 ) ...

struct InfInstance
  {
  string name; // Instance name: part reference designator or child sheet name
  int type; // 'R' for part or sheet-path part, 'C' for child instance
  string absolute_identifier; // Unique hex date/time code
  Hash<InfPin *> pins; // Pins

  // For part
  string part_value; // Part value, like 47K
  string library; // Library name, like DEVICE.LIB
  string library_part_name; // Part name, like "R"
  string sub_part_code; // Stuff in []
  string part_field[8]; // User defined part fields
  string module_field; // Package type, like 14PDIP

  // For child sheet (wow- no parameters on child sheets!)
  string sheet_file_name; // File name of child, like ALU.SCH
  };

// A join item (unnamed)

struct InfJoinItem
  {
  int type; // 'S' Signal, 'P' port, 'R' part pin, 'C' child pin

  string instance_name; // Refdes or child sheet net name.  Not used for ports or signals.
  string name; // Part pin name, signal name, child sheet net name or module port name
  int sheet_number; // For signals
  int pin_type;
    // For part or child sheet: 'I'nput, 'O'utput, 'B'idirectional, 'S'upply, 'P'assive, 'T'hree-state, open-'C'ollector, open-'E'mitter
    // For module ports: 'I'nput, 'O'utput, 'B'idirectional, 'U'nspecified, 'S'upply
    // Not used for signals.
  };

// A join: all contained join items are electrically connected

// `J ( S "signal_name" sheet_number ) ( P module_port_type "module_port_name" )
//    ( R part_reference ?pin_number? pin_type ) ( C "sheet_name" "sheet_net_name" sheet_net_type )
//    . . .

struct InfJoin
  {
  Dlist<InfJoinItem *> items;
  };

// Other sheets of this schematic

// `L netlist_filename

struct InfLink
  {
  string name; // File name without extension
  };

// A signal

// `S "signal_name" sheet_number

struct InfSignal
  {
  string name;
  int sheet_number;
  };

// A layout directive (unnamed)

// `K S "signal_name" sheet_number "directive"
// `K R part_reference ?pin_number? "directive"
// `K R sheet_name "sheet_net_name" "directive"
// `K B "bus_name[range]" sheet_number "directive"

struct InfLayout
  {
  int type; // 'S' signal, 'R' pin, 'B' bus

  string instance_name; // Child name or reference designator
  string name; // Signal name, bus name, pin number or child sheet-net name
  int sheet_number; // Sheet number for signal names and bus names
  string directive; // Layout directive
  };

// A trace marker (for simulation tool) (unnamed)

// `T S "signal_name" sheet_number "signa_display_name"
// `T R part_reference ?pin_number? "part_display_name"
// `T B "bus_name[range]" sheet_number "bus_display_name"

struct InfTrace
  {
  int type; // 'S' signal, 'R' pin, 'B' bus

  string instance_name; // refdes or child sheet name
  string name; // Signal name, bus name or pin name
  int sheet_number;
  int bus_display_type; // 'B' binary, 'D' decimal, 'H' hexidecimal, 'O' octal
  string signal_display_name;
  };

// Vector information (for simulation tool)

// `V S "signal_name" sheet_number column_number
// `V R part_reference ?pin_number? column_number
// `V B "bus_name[range]" sheet_number first_column_number

struct InfVector
  {
  int type; // 'S' signal, 'R' pin, 'B' bus

  string instance_name;
  string name;
  int sheet_number;
  int column_number;
  };

// Stimulus list item

struct InfStimList
  {
  string stimuli;
    // time:function.  functions is: 0, 1, U, Z, T
    // time1:G:time2   (goto)
  };

// Stimulus (for simulation tool)

// `W S "signal_name" sheet_number ( 0:0 50:T 100:G:50 )
// `W R part_reference ?pin_number? ( stimuli )
// `W B "bus_name[range]" sheet_number ( stimuli )

struct InfStimulus
  {
  int type; // 'S' signal, 'R' pin, 'B' bus
  string instance_name;
  string name;
  int sheet_number;

  Hash<InfStimList *> items;
  };

// A pipe item (like a sheet property)

struct InfPipeItem
  {
  string s;
  };

// A named set of pipe items

// `|
// `|keyword
// `|line1
// `|line2

struct InfPipe
  {
  string name;
  Dlist<InfPipeItem *> items;
  };

// `H format_version file_name
// `F format_version file_name
// `B "sheet_number" "total_sheet_number" etc.

struct InfDesign
  {
  string name; // File name from within .INF file: uppercase, no extension.
  string ver;
  int hier; // Set if hierarchical .INF file (`H instead of `F).

  // Links
  Hash<InfLink *> links;

  // External libraries
  Hash<InfExtern *> externs;

  // Module ports
  Hash<InfPort *> ports;

  // Signals
  Hash<InfSignal *> signals;

  // Instances
  Hash<InfInstance *> instances;

  // Joins
  Dlist<InfJoin *> joins;

  // Pipe items
  Hash<InfPipe *> pipes;

  // Layout directives
  Hash<InfLayout *> layouts;

  // Trace items
  Hash<InfTrace *> traces;

  // Vectors
  Hash<InfVector *> vectors;

  // Stimuli
  Hash<InfStimulus *> stims;

  // Header block items
  string sheet_number;
  string total_sheet_number;
  string sheet_size;
  string date;
  string document_number;
  string revision_code;
  string title;
  string organization_name;
  string address_line_1;
  string address_line_2;
  string address_line_3;
  string address_line_4;
  };

// Load a .INF file
Design *inf_load(const char *name);
