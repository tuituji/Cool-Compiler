
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"

#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <sstream>
#include <string>
#include <stack>

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;

//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol 
       arg,
       arg2,
       Bool,
       concat,
       cool_abort,
       copy,
       Int,
       in_int,
       in_string,
       IO,
       length,
       Main,
       main_meth,
       No_class,
       No_type,
       Object,
       out_int,
       out_string,
       prim_slot,
       self,
       SELF_TYPE,
       Str,
       str_field,
       substr,
       type_name,
       val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
  arg         = idtable.add_string("arg");
  arg2        = idtable.add_string("arg2");
  Bool        = idtable.add_string("Bool");
  concat      = idtable.add_string("concat");
  cool_abort  = idtable.add_string("abort");
  copy        = idtable.add_string("copy");
  Int         = idtable.add_string("Int");
  in_int      = idtable.add_string("in_int");
  in_string   = idtable.add_string("in_string");
  IO          = idtable.add_string("IO");
  length      = idtable.add_string("length");
  Main        = idtable.add_string("Main");
  main_meth   = idtable.add_string("main");
//   _no_class is a symbol that can't be the name of any 
//   user-defined class.
  No_class    = idtable.add_string("_no_class");
  No_type     = idtable.add_string("_no_type");
  Object      = idtable.add_string("Object");
  out_int     = idtable.add_string("out_int");
  out_string  = idtable.add_string("out_string");
  prim_slot   = idtable.add_string("_prim_slot");
  self        = idtable.add_string("self");
  SELF_TYPE   = idtable.add_string("SELF_TYPE");
  Str         = idtable.add_string("String");
  str_field   = idtable.add_string("_str_field");
  substr      = idtable.add_string("substr");
  type_name   = idtable.add_string("type_name");
  val         = idtable.add_string("_val");
}

static char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };


//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);
int label_index = 0;

Symbol cur_class;

struct Class_Offset {
	Symbol cla;
	int offset;
	Class_Offset(): cla(No_type), offset(0) {};
	Class_Offset(Symbol c, int o): cla(c), offset(o) { };
};

std::map<Symbol, std::vector<Symbol> >attr_list;
std::map<Symbol, std::map<Symbol, int> > attr_table;
std::vector<CgenNodeP> nodes;
std::map<Symbol, CgenNodeP>cgenNode_table;
std::map<Symbol, std::vector<Symbol> >dispatch_list;
std::map<Symbol, std::map<Symbol, Class_Offset> > dispatch_table;

std::vector<Symbol> let_case_scope;
std::vector<Symbol> method_args;
//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  CgenClassTable *codegen_classtable = new CgenClassTable(classes,os);

  os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")" 
    << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << endl; }

static void emit_load_address(char *dest_reg, char *address, ostream& s)
{ s << LA << dest_reg << " " << address << endl; }

static void emit_partial_load_address(char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream& s)
{ s << MOVE << dest_reg << " " << source_reg << endl; }

static void emit_neg(char *dest, char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << endl; }

static void emit_add(char *dest, char *src1, char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << endl; }

static void emit_addu(char *dest, char *src1, char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << endl; }

static void emit_addiu(char *dest, char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << endl; }

static void emit_div(char *dest, char *src1, char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << endl; }

static void emit_mul(char *dest, char *src1, char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << endl; }

static void emit_sub(char *dest, char *src1, char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << endl; }

static void emit_sll(char *dest, char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << endl; }

static void emit_jalr(char *dest, ostream& s)
{ s << JALR << "\t" << dest << endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << endl; }

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_label_ref(int l, ostream &s)
{ s << "label" << l; }

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }


static void emit_test_collector(ostream &s)
{
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP,SP,4,s);
  emit_load(ACC,0,SP,s);
}

static void emit_gc_check(char *source, ostream &s)
{
  if (source != (char*)A1) emit_move(A1, source, s);
  s << JAL << "_gc_check" << endl;
}


///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s  << LABEL                                             // label
      << WORD << stringclasstag << endl                                 // tag
      << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << endl // size
      << WORD;


 /***** Add dispatch information for class String ******/
      s << "String" << DISPTAB_SUFFIX;    
      s << endl;                                              // dispatch table
      s << WORD;  lensym->code_ref(s);  s << endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,stringclasstag);
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                // label
      << WORD << intclasstag << endl                      // class tag
      << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl  // object size
      << WORD; 

 /***** Add dispatch information for class Int ******/
      s << "Int" << DISPTAB_SUFFIX;    
      s << endl;                                          // dispatch table
      s << WORD << str << endl;                           // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
  for (List<IntEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,intclasstag);
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}
  
//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                  // label
      << WORD << boolclasstag << endl                       // class tag
      << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl   // object size
      << WORD;

 /***** Add dispatch information for class Bool ******/
      s << "Bool" << DISPTAB_SUFFIX;    
      s << endl;                                            // dispatch table
      s << WORD << val << endl;                             // value (0 or 1)
}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << endl;
  str << GLOBAL; falsebool.code_ref(str);  str << endl;
  str << GLOBAL; truebool.code_ref(str);   str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //
  str << INTTAG << LABEL
      << WORD << intclasstag << endl;
  str << BOOLTAG << LABEL 
      << WORD << boolclasstag << endl;
  str << STRINGTAG << LABEL 
      << WORD << stringclasstag << endl;    
}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL 
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void CgenClassTable::code_bools(int boolclasstag)
{
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

void CgenClassTable::code_select_gc()
{
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}


//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  stringtable.add_string("");
  inttable.add_string("0");

  stringtable.code_string_table(str,stringclasstag);
  inttable.code_string_table(str,intclasstag);
  code_bools(boolclasstag);
}


// code the name table
void CgenClassTable::code_class_nameTab()
{
	str << CLASSNAMETAB << LABEL;
	for (List<CgenNode> *l = nds; l; l = l->tl()){
		nodes.push_back(l->hd());
	}
	for (int i = nodes.size() - 1; i >= 0; --i) {
		char* s = nodes[i]->get_name()->get_string();
		StringEntry *entry = stringtable.lookup_string(s);
		str << WORD; 
		entry->code_ref(str); 
		str<< endl;
	}
#if 0
	// do it like this ,, ok ??
	for (List<CgenNode> *l = nds; l; l = l->tl()){
		char* s = l->hd()->get_name()->get_string();
		StringEntry *entry = stringtable.lookup_string(s);
		str << WORD; 
		entry->code_ref(str); 
		str<< endl;
	}
#endif
}

void CgenClassTable::code_class_objTab()
{
	str << CLASSOBJTAB << LABEL;
	for (int i = nodes.size()-1; i >=0; --i) {
		str << WORD << nodes[i]->get_name() << PROTOBJ_SUFFIX << endl;
		str << WORD << nodes[i]->get_name() << CLASSINIT_SUFFIX << endl;
	}
	return ;

#if 0
	for (List<CgenNode> *l = nds; l; l = l->tl()){
		CgenNodeP node= l->hd();
		Symbol name = node->get_name();
		str << WORD << name << PROTOBJ_SUFFIX << endl;
		str << WORD << name << CLASSINIT_SUFFIX << endl;
	}
	return;
#endif
}

// At the most parent method and then self's method
void add_method(CgenNodeP node, Symbol cla)
{
	Symbol name = node->get_name();
	if (name != Object) {
		add_method(node->get_parentnd(), cla);
	}
	Features fs = node->features; // now it is public 
	for(int i = fs->first(); fs->more(i); i = fs->next(i)) {
		method_class *method = dynamic_cast<method_class*>(fs->nth(i));
		if (method == NULL) continue;
		std::vector<Symbol> *vec = &(dispatch_list[cla]);
		std::map<Symbol, Class_Offset> *map_node = &(dispatch_table[cla]);
		//std::map<Symbol, std::map<Symbol, int> > *map_node = &(dispatch_table[cla]);
		std::vector<Symbol>::iterator itr;
		int offset  = (*map_node).size();
		itr = find((*vec).begin(), (*vec).end(), method->name);
		if (itr == (*vec).end()) {  // no override  then add the method
			(*vec).push_back(method->name); // push all the method
			(*map_node)[method->name] = Class_Offset(name, offset);
		}
		else { 
		// if override then 
			(*map_node)[method->name].cla = name;
		}
	}
}

void CgenClassTable::code_dispatchTab()
{
	for (List<CgenNode> *l = nds; l; l = l->tl())	{
		CgenNodeP node = l->hd();
		dispatch_list[node->get_name()] = std::vector<Symbol>();
		dispatch_table[node->get_name()] 
					= std::map<Symbol, Class_Offset>();
					//= std::map<Symbol, std::map<Symbol, int> >();
		add_method(node, node->get_name());
	}
	for (List<CgenNode> *l = nds; l; l = l->tl())	{
		CgenNodeP node = l->hd();
		Symbol name = node->get_name();
		str << name << DISPTAB_SUFFIX << LABEL;
		std::vector<Symbol> *vec = &(dispatch_list[name]);
		std::vector<Symbol>::iterator itr;
#if 0
		for (size_t index = 0; index != (*vec).size(); ++index) {
			Symbol m_name = (*vec)[index]; // the method name
			std::map <Symbol, int> *sym_off = &dispatch_table[name][m_name];
			for (std::map<Symbol, int>::iterator it = (*sym_off).begin();
				it != (*sym_off).end(); ++it) {
				if (it->second == (int)index) {
		
					str << WORD << (it->first) << METHOD_SEP << m_name <<endl; 
				}
			}
		}
#endif
		for (itr = (*vec).begin(); itr != (*vec).end(); ++itr) {	
			Symbol cla = dispatch_table[name][*itr].cla;
			str << WORD << cla << METHOD_SEP << (*itr) <<endl; 
		}

	}
}


int get_num_of_attrs(CgenNodeP node)
{
	int num = 0;
	while(node->get_name() != No_class) {
		Features fs = node->features;
		for(int i = fs->first(); fs->more(i); i = fs->next(i)){
			attr_class *attr = dynamic_cast<attr_class*>(fs->nth(i));
			if (attr != NULL) ++num;
		}
		node = node->get_parentnd();
	}
	return num;
}


void CgenClassTable::code_prototype_oneAttr(CgenNodeP node, Symbol cla)
{
	if (node->get_name() != Object) {
		code_prototype_oneAttr(node->get_parentnd(), cla);
	}
	Features fs = node->features;
	for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
		attr_class *attr = dynamic_cast<attr_class*>(fs->nth(i));
		if (attr == NULL) continue;
		if (attr->type_decl == Int) {
			IntEntry *entry = inttable.lookup_string("0");
			str << WORD;
			entry->code_ref(str);
			str  << endl; 
		}
		else if (attr->type_decl == Str) {
			StringEntry *entry = stringtable.lookup_string("");
			str << WORD;
			entry->code_ref(str);
			str << endl;
		}
		else if (attr->type_decl == Bool) {
			str << WORD;
			falsebool.code_ref(str);
			str << endl;
		}
		else {
			str << WORD << 0 << endl;
		}
		// need to change the attr_table declaration
		// and also store the  type??
		std::vector<Symbol> *attrs = &attr_list[cla];
		int index = (*attrs).size();
		(*attrs).push_back(attr->name);
		std::map<Symbol, int> *map_offset = &attr_table[cla];
		(*map_offset)[attr->name] = index;
	}
}
void CgenClassTable::code_prototype()
{
#if 0
	int tag = 0;
	for (List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP node = l->hd();
		Symbol name = node->get_name();
		int num_attr  = get_num_of_attrs(node);
		str << WORD << "-1" <<endl;  // the gabage collector
		str << name << PROTOBJ_SUFFIX << LABEL;
		str << WORD << tag << endl;
		str << WORD << 3 + num_attr << endl; // 3 : tag, dispatch_table, size 
		str << WORD << name << DISPTAB_SUFFIX << endl;
		attr_list[name]= std::vector<Symbol>();
		code_prototype_oneAttr(node, name);
		++tag;
	}
#endif
	for (size_t i = 0; i < nodes.size(); i++) {
		int class_tag = nodes.size() - 1 - i;
		Symbol name = nodes[i]->get_name();
		int num_attr  = get_num_of_attrs(nodes[i]);

		str << WORD << "-1" << endl;
		str << name << PROTOBJ_SUFFIX << LABEL;
		str << WORD << class_tag << endl;
		str << WORD << 3 + num_attr << endl; // obj size
		str << WORD << name << DISPTAB_SUFFIX << endl;
		attr_list[name]= std::vector<Symbol>();
		code_prototype_oneAttr(nodes[i], name);
	}
}

void CgenClassTable::code_class_initializer()
{
// emit initialize code for each class
	for (List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP node = l->hd();
		Symbol name = node->get_name();
		cgenNode_table[name] = node;
		str << node->get_name() << CLASSINIT_SUFFIX << LABEL;

		emit_addiu(SP, SP, -12, str);
		emit_store(FP, 3, SP, str);
		// the runtime will set  SELF  and ACC to Main.main 
		// see trap.handler    la  $a0 Main_protObj 
		// sw  $a0 4($sp)
		// move    $s0 $a0
		emit_store(SELF, 2, SP, str);
		emit_store(RA, 1, SP, str); // where ra is set the  before main called
		emit_addiu(FP, SP, 4, str); // the new fp
		emit_move(SELF, ACC, str);
		
		// call the parent init code 
		if (name != Object) {  // the Object class do not have parent
			char pa_label[128];
			char* pa_name = node->get_parentnd()->get_name()->get_string();
			sprintf(pa_label, "%s%s", pa_name, CLASSINIT_SUFFIX);
			// do not need to set stack 
			emit_jal(pa_label, str);
		}
		Features fs = node->features;
		for(int i = fs->first(); fs->more(i); i = fs->next(i)) {
			attr_class *attr = dynamic_cast<attr_class*>(fs->nth(i));
			if (attr == NULL) continue;
			attr->init->code(str, name);  // the return value is in ACC
			int offset = attr_table[name][attr->name] + 3; 
			if (attr->init->get_type() == NULL) {
				if (attr->type_decl == Int) {
					emit_load_int(ACC, inttable.lookup_string("0"), str);
				}
				else if(attr->type_decl == Bool) {
					emit_load_bool(ACC, falsebool, str);
				}
				else if(attr->type_decl == Str){
					emit_load_string(ACC, stringtable.lookup_string(""), str);
				}
				else {
					emit_load_imm(ACC, 0, str);
				}
			}
			emit_store(ACC, offset, SELF, str);
		}
		
		// the callee will garantuee the s0 is not changed after return	
		// so SELF here is still the class used before
		emit_move(ACC, SELF, str);
		emit_load(FP, 3, SP, str);  // pop the stack 
		emit_load(SELF, 2, SP, str);
		emit_load(RA, 1, SP, str);
		emit_addiu(SP, SP, 12, str);
		emit_return(str); 
	}
}

void CgenClassTable::code_class_method()
{
	for (List<CgenNode> *l = nds; l; l = l->tl()) {
		CgenNodeP node = l->hd();
		Symbol name = node->get_name();
		Features fs = node->features;
		// if it is the basic type, then omit
		// those basic types have already defined
		// like dynamic linking library

		if (name == Object || name == IO || name == Str) continue;
		cur_class = name;
		for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
			method_class *method = dynamic_cast<method_class*>(fs->nth(i));
			if (method == NULL) continue;
			// the  all the parameters and store them in the stack
			Formals fs = method->formals;
			method_args.clear();
			for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
				method_args.push_back(fs->nth(i)->get_name());
			}
	 
			str << node->get_name() << METHOD_SEP << method->name << LABEL;
			// same as class init
			emit_addiu(SP, SP, -12, str);
			emit_store(FP, 3, SP, str);
			emit_store(SELF, 2, SP, str);
			emit_store(RA, 1, SP, str); 
			emit_addiu(FP, SP, 4, str); 
			// notice, this is where self is set
			// when dispatch a method , ACC must set to the object address
			emit_move(SELF, ACC, str);  
			// before dispatch other code , s0 is set to self
			method->expr->code(str, cur_class);
			
			// after the method call 
			// pop the stack	
			emit_load(FP, 3, SP, str);  // pop the stack 
			emit_load(SELF, 2, SP, str);
			emit_load(RA, 1, SP, str);
			emit_addiu(SP, SP, 12 + method->formals->len() * 4, str);
			emit_return(str);
		}
	}
	
}


CgenClassTable::CgenClassTable(Classes classes, ostream& s) : nds(NULL) , str(s)
{
 	// need change here
	// see the code produced by coolc
   stringclasstag = 4 /* Change to your String class tag here */;
   intclasstag =    2 /* Change to your Int class tag here */;
   boolclasstag =   3 /* Change to your Bool class tag here */;

   enterscope();
   if (cgen_debug) cout << "Building CgenClassTable" << endl;
   install_basic_classes();
   install_classes(classes);
   build_inheritance_tree();

   code();  // all goes into code
   exitscope();
}

void CgenClassTable::install_basic_classes()
{

// The tree package uses these globals to annotate the classes built below.
  //curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
	new CgenNode(class_(No_class,No_class,nil_Features(),filename),
			    Basic,this));
  addid(SELF_TYPE,
	new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
			    Basic,this));
  addid(prim_slot,
	new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
			    Basic,this));

// 
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
   new CgenNode(
    class_(Object, 
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename),
    Basic,this));

// 
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
     class_(IO, 
            Object,
            append_Features(
            append_Features(
            append_Features(
            single_Features(method(out_string, single_Formals(formal(arg, Str)),
                        SELF_TYPE, no_expr())),
            single_Features(method(out_int, single_Formals(formal(arg, Int)),
                        SELF_TYPE, no_expr()))),
            single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
            single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	   filename),	    
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer. 
//
   install_class(
    new CgenNode(
     class_(Int, 
	    Object,
            single_Features(attr(val, prim_slot, no_expr())),
	    filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//       
   install_class(
    new CgenNode(
      class_(Str, 
	     Object,
             append_Features(
             append_Features(
             append_Features(
             append_Features(
             single_Features(attr(val, Int, no_expr())),
            single_Features(attr(str_field, prim_slot, no_expr()))),
            single_Features(method(length, nil_Formals(), Int, no_expr()))),
            single_Features(method(concat, 
				   single_Formals(formal(arg, Str)),
				   Str, 
				   no_expr()))),
	    single_Features(method(substr, 
				   append_Formals(single_Formals(formal(arg, Int)), 
						  single_Formals(formal(arg2, Int))),
				   Str, 
				   no_expr()))),
	     filename),
        Basic,this));

}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
  Symbol name = nd->get_name();

  if (probe(name))
    {
      return;
    }

  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  nds = new List<CgenNode>(nd,nds);
  addid(name,nd);
}

void CgenClassTable::install_classes(Classes cs)
{
  for(int i = cs->first(); cs->more(i); i = cs->next(i))
    install_class(new CgenNode(cs->nth(i),NotBasic,this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
  for(List<CgenNode> *l = nds; l; l = l->tl())
      set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n)
{
  children = new List<CgenNode>(n,children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}



void CgenClassTable::code()
{
  if (cgen_debug) cout << "coding global data" << endl;
  code_global_data();

  if (cgen_debug) cout << "choosing gc" << endl;
  code_select_gc();

  if (cgen_debug) cout << "coding constants" << endl;
  code_constants();

//                 Add your code to emit
//                   - prototype objects
//                   - class_nameTab
//                   - dispatch tables
//
  if (cgen_debug) cout << "coding class_nameTab" << endl;
  code_class_nameTab();

  if (cgen_debug) cout << "coding clas_objTab" << endl;
  code_class_objTab();

  if (cgen_debug) cout << "coding class_dispatchTab" << endl;
  code_dispatchTab();

  if (cgen_debug) cout << "coding prototype objects" << endl;
  code_prototype();
////////

  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();

//                 Add your code to emit
//                   - object initializer
//                   - the class methods
//                   - etc...
  if (cgen_debug) cout << "coding class initializer" << endl;
  code_class_initializer();

  if (cgen_debug) cout << "coding class method" << endl;
  code_class_method();
/////////
  
}


CgenNodeP CgenClassTable::root()
{
   return probe(Object);
}


///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   children(NULL),
   basic_status(bstatus)
{ 
   stringtable.add_string(name->get_string());          // Add class name to string table
}


//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************
int  reverse_find_symbol_in_let(Symbol name)
{
	int i; 
	for (i = let_case_scope.size() -1; i >=0; --i) {
		if (let_case_scope[i] == name) return i;
	}
	return i;
}

void assign_class::code(ostream &s, Symbol cla) {

	expr->code(s, cla); // the value will be in ACC reg
	// first we check the let scope
	int offset;

	int idx = reverse_find_symbol_in_let(name);
	if (idx != -1) {
		//offset = let_scope.size() - idx + let_scope_offset;
		offset = let_case_scope.size() - idx ;
		emit_store(ACC, offset, SP, s);
		return;
	}
	std::vector<Symbol>::iterator itr;
#if 0
	if ((itr = find(let_case_scope.begin(), let_case_scope.end(), name)) 
			!= let_scope.end())	{
		offset = let_case_scope.end() - itr;// + let_scope_offset;
		emit_store(ACC, offset, SP, s);
		return;
	}
#endif
	// else if the object are the parameters
	if((itr = find(method_args.begin(), method_args.end(), name))
			!= method_args.end()) {
		offset = method_args.end() - itr + 2;
		emit_load(ACC, offset, FP, s);
		return;
	}
	
	offset = attr_table[cla][name] + 3;
	s << "# " << cla <<  "." << name <<" # offset is " << offset <<endl;
	emit_store(ACC, offset, SELF, s);

}

void static_dispatch_class::code(ostream &s, Symbol cla)
{
	for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
		actual->nth(i)->code(s, cla);
		emit_push(ACC, s);
		let_case_scope.push_back(No_type);
		//++let_scope_offset;
	}
	Symbol type = expr->get_type();
	if (type == SELF_TYPE) {
		emit_move(ACC, SELF, s);  // mov $a0 $s0
	}
	else {
		s << "#come to dispatch " << type << endl;
		expr->code(s, cla);  // ?? right ??
	}
	char buf[64];
	int index = label_index++;
	emit_bne(ACC, ZERO, index, s);
	sprintf(buf, "%s0", STRCONST_PREFIX);  // str_const0 is the file name
	emit_load_address(ACC, buf, s);	
	emit_load_imm(T1, 1, s);
	emit_jal("_dispatch_abort", s);
	emit_label_def(index, s);
	//ACCa0 is the self, 8 offset is the method_table
	// label_index here is ok since non change of label_index 
	// between emit_bne  and  emit_label_ref
	// get the offset
	if (type == SELF_TYPE) type = cla;
	
	int offset  = dispatch_table[type_name][name].offset;  // maybe from parent class	
	sprintf(buf, "%s%s", type_name->get_string(), DISPTAB_SUFFIX);
	emit_load_address(T1, buf, s);
	
#if 0
	Symbol tmp = type_name;
	std::map<Symbol, Class_Offset>* sy_offset = &dispatch_table[tmp];
	std::map<Symbol, Class_Offset>::iterator itr = (*sy_offset).find(name);
	while(itr == (*sy_offset).end()) {
		CgenNodeP node = cgenNode_table[tmp];
		node = node->get_parentnd();
		tmp = node->get_name();
		sy_offset = &dispatch_table[tmp];
		itr = (*sy_offset).find(name);
	}
	int offset = (itr->second).offset;
#endif
#if 0
	Symbol tmp = type_name;
	std::map<Symbol, int> *sy_offset = &dispatch_table[type][name];
	std::map<Symbol, int>::iterator itr = (*sy_offset).find(tmp);
	while(itr == (*sy_offset).end()) {
		CgenNodeP node = cgenNode_table[tmp];
		node = node->get_parentnd();
		tmp = node->get_name();
		itr = (*sy_offset).find(tmp);
	}
	int offset = itr->second;
#endif
	emit_load(T1, offset, T1, s);  // load the method
	emit_jalr(T1, s);
	for (int i = 0; i < actual->len(); ++i) {
		let_case_scope.pop_back();
	}

}

void dispatch_class::code(ostream &s, Symbol cla) {

	// first push all the parameters
	// what is the seq ??
	for (int i = actual->first(); actual->more(i); i = actual->next(i)) {
		actual->nth(i)->code(s, cla);
		emit_push(ACC, s);
		let_case_scope.push_back(No_type);
		//++let_scope_offset;
	}
	// notice before dispatch a method , must set ACC to the Object address
	// see code_class_method()
	Symbol type = expr->get_type();

	expr->code(s, cla);  //  notice 
	// check if the objcet is null, if null then call _dispatch_abort
	// actually, we can check if the object is null or not
	char buf[64];
	int index = label_index++;
	emit_bne(ACC, ZERO, index, s);
	sprintf(buf, "%s0", STRCONST_PREFIX);  // str_const0 is the file name
	emit_load_address(ACC, buf, s);	
	emit_load_imm(T1, 1, s);
	emit_jal("_dispatch_abort", s);
	emit_label_def(index, s);
	emit_load(T1, 2, ACC, s);
	//ACCa0 is the self, 8 offset is the method_table
	// label_index here is ok since non change of label_index 
	// between emit_bne  and  emit_label_ref
	// get the offset
	if (type == SELF_TYPE) type = cla;
	int offset  = dispatch_table[type][name].offset;  // maybe from parent class
#if 0
	Symbol tmp = type;
	std::map<Symbol, Class_Offset>* sy_offset = &dispatch_table[tmp];
	std::map<Symbol, Class_Offset>::iterator itr = (*sy_offset).find(name);
	while(itr == (*sy_offset).end()) {
		CgenNodeP node = cgenNode_table[tmp];
		node = node->get_parentnd();
		tmp = node->get_name();
		sy_offset = &dispatch_table[tmp];
		itr = (*sy_offset).find(name);
	}
	int offset = (itr->second).offset;
#endif	
#if 0
	int offset = 0 ;// = dispatch_table[type][name][type];
	std::map<Symbol, int> *cla_off = &dispatch_table[type][name];
	for (std::map<Symbol, int>::iterator itr = (*cla_off).begin(); itr != (*cla_off).end(); ++itr) {
		if (itr->second > offset) offset = itr->second;
	}
#endif
	s << "# offset is "<<   offset << " meth " << type << endl;
	emit_load(T1, offset, T1, s);  // load the method
	emit_jalr(T1, s);
	for (int i = 0; i < actual->len(); ++i) {
		let_case_scope.pop_back();
	}
	//let_scope_offset = 0;
}


void cond_class::code(ostream &s, Symbol cla)
{
	pred->code(s, cla); // the pred result is in ACC
	// for Bool class, offset 3(12) is the attr value
	int then_label_ = label_index++;
	int else_label_ = label_index++;
	// ACC now stores a Bool Object while its offset 3(12) is the value
	emit_load(T1, 3, ACC, s);
	emit_beqz(T1, then_label_, s);
	// here may change label_index, so we store it in index first
	then_exp->code(s, cla);
	emit_branch(else_label_, s); 
	emit_label_def(then_label_, s);
	else_exp->code(s, cla);
	emit_label_def(else_label_, s);
}

void loop_class::code(ostream &s, Symbol cla)
{
	int while_index = label_index++;
	int end_index = label_index++;
	emit_label_def(while_index, s);
	pred->code(s, cla);	// the result is Bool object in ACC
	emit_load(T1, 3, ACC, s);
	emit_load_imm(ACC, 0, s);
	emit_beqz(T1, end_index, s);
	body->code(s, cla);
	emit_branch(while_index, s);	
	emit_label_def(end_index, s);
	// do not need return value
}

void emit_cmp_type (ostream &s, Symbol cla, Symbol a0, Symbol a1)
{

	// by compare the class id
	char name[64];
	sprintf(name, "%s%s", a0->get_string(), PROTOBJ_SUFFIX);
	emit_load_address(T1, name, s);  // get theclass proto
	emit_load(T1, 0, T1, s);         // get the class id

	sprintf(name, "%s%s", a1->get_string(), PROTOBJ_SUFFIX);
	emit_load_address(T2, name, s);  // get theclass proto
	emit_load(T2, 0, T2, s);         // get the class id

	emit_load_address(ACC,"bool_const1", s); 
	emit_load_address(A1, "bool_const0", s);
	emit_jal("equality_test",s);  // finally acc store the value of the result
}

void typcase_class::code(ostream &s, Symbol cla)
{
	s << "# code case expr" << endl;
	expr->code(s, cla);  // generate code for init part value store in ACC
	// check case , abort
	int case_label = label_index++;
	emit_bne(ACC, ZERO, case_label, s);
	emit_load_address(ACC, "str_const0", s);
	emit_load_imm(T1, 1, s);
	emit_jal("_case_abort2", s);
	emit_label_def(case_label, s);
	
    emit_push(ACC, s);    // to store the value
	s  << "#come to case " << endl;

	int label_end = label_index++;
	for(int i = cases->first(); cases->more(i); i = cases->next(i)){
		branch_class* branch = static_cast<branch_class*>(cases->nth(i));
		// do not need check here, already checked in semant phase
//		branch_class* branch = dynamic_cast<branch_class*>(cases->nth(i));
//		if (branch == NULL) cerr << "error case is not a branch_class type" << endl;
//		branch->expr(s, cla);
		int next_case = label_index++;
		char name[64];
	
		emit_load(T1, 1, SP, s); // the stack now is the Object of expr
		emit_load(T1, 0, T1, s); //  get the object id

		sprintf(name, "%s%s", branch->type_decl->get_string(), PROTOBJ_SUFFIX);
		emit_load_address(T2, name, s);  // get theclass proto
		emit_load(T2, 0, T2, s);         // get the class id

		emit_bne(T1 , T2, next_case, s);
		// push the name to let_case_scope but do not need to push to stack
		// because branch->name is bind to  cases->expr, which have been already pushed 
		let_case_scope.push_back(branch->name);  
		branch->expr->code(s, cla);// the result will be in ACC
		// do all the things
		emit_branch(label_end, s);  // if match then to the end
		emit_label_def(next_case, s);
		let_case_scope.pop_back();
	}
	// if none of the case match, then
	// return NULL pointer 

	emit_jal("_case_abort", s);
	emit_load_imm(ACC, 0, s); 
	emit_label_def(label_end, s);
	emit_addiu(SP, SP, 4, s);
	return;
}

void block_class::code(ostream &s, Symbol cla)
{
	for (int i = body->first(); body->more(i); i = body->next(i)) {
		body->nth(i)->code(s, cla);
	}
}


void let_class::code(ostream &s, Symbol cla)
{
	// if init is empty, the get_type call will result null
	if (init->get_type() == NULL){
		if (type_decl == Int) {
			emit_load_int(ACC, inttable.lookup_string("0"), s);
		}
		else if(type_decl == Bool) {
			emit_load_bool(ACC, falsebool, s);
		}
		else if (type_decl == Str){
			emit_load_string(ACC, stringtable.lookup_string(""), s);
		}
		else {
			emit_load_imm(ACC, 0, s);
		}
	}
	else {
		init->code(s, cla);  // generate code for init part value store in ACC
	}
	emit_push(ACC, s);
	let_case_scope.push_back(identifier);
	body->code(s, cla);
	let_case_scope.pop_back();
	emit_addiu(SP, SP, 4, s);
}

void plus_class::code(ostream &s, Symbol cla)
{
	e1->code(s, cla);
	emit_push(ACC, s);  // ACC is now the object returned by e1  
	let_case_scope.push_back(No_type);
	//++let_scope_offset;

	e2->code(s, cla);
	emit_jal("Object.copy", s); // get a copy of e2(e3) object, store in acc
	// first get the 
	emit_load(T1, 1, SP, s); // pop the stack to get e1 object
	emit_addiu(SP, SP, 4, s);
	let_case_scope.pop_back();
//	--let_scope_offset;
	emit_load(T1, 3, T1, s); // load the value of e1
	emit_load(T2, 3, ACC, s); // load the value of e2
	
	emit_addu(T1, T1, T2, s); // add t2 to t1  t1 = t1 + t2
	emit_store(T1, 3, ACC, s); // store the value
}

void sub_class::code(ostream &s, Symbol cla)
{
	e1->code(s, cla);
	emit_push(ACC, s);  // ACC is now the object returned by e1  
	let_case_scope.push_back(No_type);
	//++let_scope_offset;

	e2->code(s, cla);
	emit_jal("Object.copy", s); // get a copy of e2(e3) object, store in acc
	// first get the 
	emit_load(T1, 1, SP, s); // pop the stack to get e1 object
	emit_addiu(SP, SP, 4, s);
	let_case_scope.pop_back();
	//--let_scope_offset;
	emit_load(T1, 3, T1, s); // load the value of e1
	emit_load(T2, 3, ACC, s); // load the value of e2

	emit_sub(T1, T1, T2, s); // sub t2 from t1  t1 = t1 - t2
	emit_store(T1, 3, ACC, s); // store the value
	
}

void mul_class::code(ostream &s, Symbol cla)
{
	e1->code(s, cla);
	emit_push(ACC, s);  // ACC is now the object returned by e1  
	let_case_scope.push_back(No_type);
//	++let_scope_offset;

	e2->code(s, cla);
	emit_jal("Object.copy", s); // get a copy of e2(e3) object, store in acc
	// first get the 
	emit_load(T1, 1, SP, s); // pop the stack to get e1 object
	emit_addiu(SP, SP, 4, s);
//	--let_scope_offset;
	let_case_scope.pop_back();
	emit_load(T1, 3, T1, s); // load the value of e1
	emit_load(T2, 3, ACC, s); // load the value of e2

	emit_mul(T1, T1, T2, s); // sub t2 from t1  t1 = t1 - t2
	emit_store(T1, 3, ACC, s); // store the value
}

void divide_class::code(ostream &s, Symbol cla)
{
	e1->code(s, cla);
	emit_push(ACC, s);  // ACC is now the object returned by e1  
//	++let_scope_offset;
	let_case_scope.push_back(No_type);
	e2->code(s, cla);
	emit_jal("Object.copy", s); // get a copy of e2(e3) object, store in acc
	// first get the 
	emit_load(T1, 1, SP, s); // pop the stack to get e1 object
	emit_addiu(SP, SP, 4, s);
//	--let_scope_offset;
	let_case_scope.pop_back();
	emit_load(T1, 3, T1, s); // load the value of e1
	emit_load(T2, 3, ACC, s); // load the value of e2

	emit_div(T1, T1, T2, s); // sub t2 from t1  t1 = t1 - t2
	emit_store(T1, 3, ACC, s); // store the value
}

void neg_class::code(ostream &s, Symbol cla) 
{
	e1->code(s, cla);
	
	emit_jal("Object.copy", s); // get a copy of e2(e3) object, store in acc

	emit_load(T1, 3, ACC, s); // load the value of e2

	emit_load_imm(T2, 0, s);
	emit_sub(T1, T2, T1, s); // sub t2 from t1  t1 = t1 - t2
	emit_store(T1, 3, ACC, s); // store the value
	
}



void lt_class::code(ostream &s, Symbol cla)
{
	e1->code(s, cla);
	// save the acc, here cannot directly store it to T1 or T0
	// because T0 and T1 may be used in e2->code();
	emit_push(ACC, s);  // here do a push 
//	++let_scope_offset; // and let_scope_offset should also inc
	let_case_scope.push_back(No_type);
	e2->code(s, cla);
	// now we can use T2 and T1
	emit_move(T2, ACC, s);    // T1 and T2 for value of e1 and e2
	emit_load(T1, 1, SP, s);
	emit_addiu(SP, SP, 4, s);
//	--let_scope_offset;      // after pop the stack, dec let_scope_offset	
	let_case_scope.pop_back();
	
	int index = label_index++;
	emit_load(T1, 3, T1, s);
	emit_load(T2, 3, T2, s);
	emit_load_address(ACC,"bool_const1", s); 
	emit_blt(T1, T2, index, s);
	emit_load_address(ACC, "bool_const0", s);
	emit_label_def(index, s);
}


void eq_class::code(ostream &s, Symbol cla)
{
	e1->code(s, cla);
	// save the acc, here cannot directly store it to T1 or T0
	// because T0 and T1 may be used in e2->code();
	emit_push(ACC, s);  // here do a push 
//	++let_scope_offset; // and let_scope_offset should also inc
	let_case_scope.push_back(No_type);
	e2->code(s, cla);
	// now we can use T2 and T1
	emit_move(T2, ACC, s);    // T1 and T2 for value of e1 and e2
	emit_load(T1, 1, SP, s);
	emit_addiu(SP, SP, 4, s);
//	--let_scope_offset;      // after pop the stack, dec let_scope_offset	
	let_case_scope.pop_back();
	Symbol t = e1->get_type();// in semant check, type have checked
	if (t == Int || t == Bool || t == Str) {
		// see equality_test in trap.handler
		// a0 true , a1 false, if not equal equality_test will set a0 to false
		emit_load_address(ACC,"bool_const1", s); 
		emit_load_address(A1, "bool_const0", s);
		emit_jal("equality_test",s);
	}
	else {  // if not basic type, then check the address
		int index = label_index++;
		emit_load_address(ACC, "bool_const1",s); // load true to $a0
 		// if T1 == T2 , will jump , then ACC remains bool_const1(true)
		emit_beq(T1, T2, index, s); 
		// else not equal, not ACC will be set to bool_const0(false)
		emit_load_address(ACC, "bool_const0", s);
		emit_label_def(index, s);
	}
}


void leq_class::code(ostream &s, Symbol cla)
{
	e1->code(s, cla);
	// save the acc, here cannot directly store it to T1 or T0
	// because T0 and T1 may be used in e2->code();
	emit_push(ACC, s);  // here do a push 
//	++let_scope_offset; // and let_scope_offset should also inc
	let_case_scope.push_back(No_type);
	e2->code(s, cla);
	// now we can use T2 and T1
	emit_move(T2, ACC, s);    // T1 and T2 for value of e1 and e2
	emit_load(T1, 1, SP, s);
	emit_addiu(SP, SP, 4, s);
//	--let_scope_offset;      // after pop the stack, dec let_scope_offset	
	let_case_scope.pop_back();
	
	int index = label_index++;
	emit_load(T1, 3, T1, s);
	emit_load(T2, 3, T2, s);
	emit_load_address(ACC,"bool_const1", s); 
	emit_bleq(T1, T2, index, s);
	emit_load_address(ACC, "bool_const0", s);
	emit_label_def(index, s);
}

void comp_class::code(ostream &s, Symbol cla)
{
	int index = label_index++;

	e1->code(s, cla);
	emit_jal("Object.copy", s); // get a copy of e2(e3) object, store in acc
	emit_load(T1, 3, ACC, s);   // load the value of e2
	emit_beqz(T1, index, s);    // if e1 if false , then set true 
	emit_load_address(ACC, "bool_const0", s);
	emit_label_def(index, s);
	emit_load_address(ACC, "bool_const1", s); 
}

void int_const_class::code(ostream& s, Symbol cla)
{
	emit_load_int(ACC, inttable.lookup_string(token->get_string()), s);	
}


void string_const_class::code(ostream& s, Symbol cla)
{
	// all string constants are stored in a string table
	emit_load_string(ACC, stringtable.lookup_string(token->get_string()), s);
}

void bool_const_class::code(ostream& s, Symbol cla)
{
	emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(ostream &s, Symbol cla)
{
	if (type_name == SELF_TYPE) {
//		cerr << "will do it later" << endl;
		// first get the id of the class, then find it in the name table
		emit_move(T1, SELF, s);
		emit_load(T1, 0, T1, s);//  get class id
		emit_sll(T1, T1, 3, s);
		emit_load_address(T2, "class_objTab", s);
		emit_addu(T1, T1, T2, s);  // T1 = T2 + T1  entry in class_objTab
		emit_load(ACC, 0, T1, s);// ao is the name_protObj
		emit_load(T2, 1, T1, s); // t2 name_init 
		emit_push(T2, s);        // t2 will be used after Object.copy, so first push it
		emit_jal("Object.copy", s);
		emit_load(T2, 1, SP, s);
		emit_addiu(SP, SP, 4, s);
		emit_jalr(T2, s);       // call name_init method
	}
	else {
		// Object.copy must receive a parameter a0(the proto of the object)
		char object[64];
		char object_init[64];
		char *name = type_name->get_string();
		sprintf(object, "%s%s", name, PROTOBJ_SUFFIX);
		sprintf(object_init, "%s%s", name, CLASSINIT_SUFFIX);
		emit_load_address(ACC, object, s);  
		emit_jal("Object.copy", s);	 // a0 will store the new object
		emit_jal(object_init, s);    // init will not change a0
	}
}

void isvoid_class::code(ostream &s, Symbol cla)
{
	char str[64];
	int index = label_index++;
	e1->code(s, cla);  // here need a temporary
	emit_move(T1, ACC, s);
	sprintf(str, "%s%d", BOOLCONST_PREFIX, TRUE);
	emit_load_address(ACC, str, s);
	emit_beqz(T1, index, s);// if it is void then return Bool const ture 
	sprintf(str, "%s%d", BOOLCONST_PREFIX, FALSE);
	emit_load_address(ACC, str, s);
	emit_label_def(index, s);
}


void no_expr_class::code(ostream &s, Symbol cla)
{
	return;
}



void object_class::code(ostream &s, Symbol cla)
{
	if (name == self) {
		emit_move(ACC, SELF, s);
		return;
	}

	int offset;
	// if the object within a let scope
	std::vector<Symbol>::iterator itr;
	int idx = reverse_find_symbol_in_let(name);
	if (idx != -1) {
		//offset = let_scope.size() - idx + let_scope_offset;
		offset = let_case_scope.size() - idx ;
		emit_load(ACC, offset, SP, s);
		return;
	}
	
	// else if the object are the parameters
	if((itr = find(method_args.begin(), method_args.end(), name))
			!= method_args.end()) {
		offset = method_args.end() - itr  + 2;
		emit_load(ACC, offset, FP, s);  // notice here, it is FP not SP
		return;
	}

	// now we only consider parameter from attribute
	offset = attr_table[cla][name] + 3;
	emit_load(ACC, offset, SELF, s);
}

