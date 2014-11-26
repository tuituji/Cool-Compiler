

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

#define SEMANT_ERROR \
	class_table->semant_error(class_table->get_class_(cla))

#define EXPR_PARAM0 \
	Symbol cla, SymbolTable<Symbol, Symbol> *attr, \
		SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table

#define EXPR_PARAM1 \
	cla, attr, meth, class_table 

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
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


bool ClassTable::class_inherent_cycle(Symbol cla)
{
	Class_ c1 ; // = get_class_(cla);
	Symbol name = cla;
	while (name != No_class) {
		c1 = get_class_(name);
		name = c1->get_parent();
		if(name == cla) return true;
	}
	return false;
}

ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) 
{
	// First install all the classes
	// those class will be used as types
    /* Fill this in */
	install_basic_classes();
	std::map<Symbol,Class_>::iterator itr;
	for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
		Class_ curr_class = classes->nth(i);
		Symbol name = curr_class->get_name();
		itr = class_table.find(name);// find the class in the classtable
		if (itr == class_table.end()) {
			add_class(curr_class);  // have add all other Classes
		}
		else {
			semant_error(curr_class) << "Redefinition of basic class Int."
				<< endl;
		}
	}
	// check if the class is inherent from Bool
	bool main = false;
	for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
		Class_ curr_class = classes->nth(i);
		Symbol name = curr_class->get_name();
		Symbol pa = curr_class->get_parent();

		if (name == Main) main = true;

		if (class_inherent_legal(SELF_TYPE, name)) {
			semant_error(curr_class) << "Class " << name 
				<< " cannot inherit class SELF_TYPE." << endl;
			continue;
		}
		if(!class_in_classtable(pa)) {
			semant_error(curr_class) << "Class " << name 
				<<" inherits from an undefined class " << pa <<"." << endl;
			continue;
		}
		if(class_inherent_cycle(name)) {  // notice 
			semant_error(curr_class) << "Class " << name 
				<< "inherent to a cycle." << endl;
			return;
			continue;
		}
		if(class_inherent_legal(Bool, name)) {
			semant_error(curr_class) << "Class " << name 
				<< " cannot inherit class Bool." << endl;
			continue;
		}
		if(class_inherent_legal(Str, name)) {
			semant_error(curr_class) << "Class " << name 
				<< " cannot inherit class String." << endl;
			continue;
		}
	}
	if (main == false) 
		semant_error() << "Class Main is not defined." << endl;
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.
	
	// see the definition for 
	// Class_ class_(Symbol name, Symbol parent, Features features, 
	//					Symbol filename)
    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
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
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
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
	       filename);
// add to class_table
	add_class(Object_class);
	add_class(IO_class);
	add_class(Int_class); 
	add_class(Bool_class);
	add_class(Str_class);
//////
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 


inline bool feature_type_is_method(Feature feature)
{
	method_class *method = dynamic_cast<method_class *>(feature);
	if(method == NULL) return 0;
	else return 1;
}

bool class__class::is_inherent_attr(Symbol attr, ClassTable * classtable)
{
	Symbol pa = get_parent();
	Class_ pac =  classtable->get_class_(pa);
	while (pa != No_class) {
		Features fs = pac->get_features();
		for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
			Feature f = fs->nth(i);
			if (f->get_name() == attr) {
				return true;
			}
		}
		pa = pac->get_parent();
		pac = classtable->get_class_(pa);
	}
	return false;
}

void method_class::add_feature(Class_ cla, SymbolTable <Symbol, Symbol> *attr, 
		SymbolTable<Symbol, Symbol> *meth, ClassTable *classtable)
{
	//cerr << "now try to add method: " << name << endl;
	if (meth->probe(name) != NULL) {
		//cerr << "  error"; dump_Symbol(cerr, 2, name); 
	} else if (meth->lookup(name) != NULL) {
		//cerr << "  error"; dump_Symbol(cerr, 2, name); 	
	} else {  // need to consider function override
		meth->addid(name, new Symbol(return_type));
	}
		
}

void attr_class::add_feature(Class_ cla, SymbolTable <Symbol, Symbol> *attr, 
		SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	// check the attr cannot be self
	if(name == self) {
		class_table->semant_error(cla) << 
			"'self' cannot be the name of an attribute." << endl;
		return;
	}
	// check  cannot redefine the inerent attr
	if (cla->is_inherent_attr(name, class_table)){
		class_table->semant_error(cla) <<  "Attribute "<< name 
			<< " is an attribute of an inherited class." << endl;
		return;
	}
	
	// actually i do not use meth
	if (attr->probe(name) != NULL) {
		//cerr << "  error"; dump_Symbol(cerr, 2, name); 
	} else if (attr->lookup(name)) {
		//cerr << "  error"; dump_Symbol(cerr, 2, name); 	
	} else if (type_decl == SELF_TYPE) {
		attr->addid(name, new Symbol(name));
	} else {
		attr->addid(name, new Symbol(type_decl));
	}
}

method_class* search_inherent_method(Symbol cla, Symbol meth, ClassTable *class_table)
{
	Class_ pac = class_table->get_class_(cla);
	Symbol pa = pac->get_parent();
	while (pa != No_class) {
		pac = class_table->get_class_(pa);
		Features fs = pac->get_features();
		for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
			Feature feature = fs->nth(i);
			if (!feature_type_is_method(feature)) continue;
			if (feature->get_name() == meth) {
				return static_cast<method_class*>(feature);
			}
		}
		pa = pac->get_parent();
	}
	return NULL;
}

void method_class::feature_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
		SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table) 
{
	// name is the current class
	// many things to do
	attr->enterscope();  // do need ??

//	if(meth->lookup(name)) {
//		cerr << "method_class find "<< name << endl;
//	}
	method_class* method = search_inherent_method(cla, name, class_table);
	if (method != NULL) {
		int len1 = method->get_formals()->len();
		int len2 = formals->len();
		if (len1 != len2) {
			SEMANT_ERROR << "Incompatible number of formal parameters in redefined method "
					<< method->get_name() << "." << endl;
			return;
		}
		for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
			Formal f1 = method->get_formals()->nth(i);
			Formal f2 = formals->nth(i);
			Symbol t1 = f1->get_type();
			Symbol t2 = f2->get_type();
			if (t1 != t2) {
				SEMANT_ERROR << "In redefined method "<< name << ", parameter type "
						<< t2 << " is different from original type " << t1 << endl;
			}
		}
	}
	// check the return type
	Symbol t = get_type();
	if (t== SELF_TYPE) { t = cla; }
	else { 
		if (!class_table->class_in_classtable(t)) {
			SEMANT_ERROR << "Undefined return type "<< t
				<<" in method " << get_name() <<"." << endl;
		}
	}	
	std::map<Symbol, Symbol> parameters;
	std::map<Symbol, Symbol>::iterator itr;
	for (int i = formals->first(); formals->more(i); i = formals->next(i)) {
		Formal formal = formals->nth(i);
		Symbol name = formal->get_name();
		Symbol type = formal->get_type();
		if (name == self) {
			SEMANT_ERROR << "'self' cannot be the name of a formal parameter." << endl;
		}
		if (type == SELF_TYPE) {
			SEMANT_ERROR << "Formal parameter "<< name << " cannot have type SELF_TYPE." <<endl;
		}
		itr = parameters.find(name);
		if (itr != parameters.end()) {
			SEMANT_ERROR << "Formal parameter "<< name 
					<<" is multiply defined." << endl;
		}
		parameters[name] = type;
		attr->addid(formal->get_name(), new Symbol(formal->get_type()));
	}
	t = expr->expr_eval(cla, attr, meth, class_table); 
	// if both are SELF_TYPE  then do not need to check
	if (!(t== SELF_TYPE && return_type == SELF_TYPE)) {
		// else do check the infered type and declared type
		Symbol t1 = (t == SELF_TYPE)? cla: t;
		if (!class_table->class_inherent_legal(return_type, t1)){
			SEMANT_ERROR << "Inferred return type " << t <<" of method "<< name 
				<< " does not conform to declared return type "
				<< return_type << "." << endl ;
		}
	}
// here should check the return value type
	attr->exitscope();
}
// name           in the current class
// attr           attrtable
// meth           methodtable of this class
// class_table    all the class table
bool ClassTable::class_inherent_legal(Symbol pa, Symbol ch)
{	
	if (pa == ch) return true;
	std::map<Symbol, Class_>::iterator itr;
	while (ch != No_class) {
		itr = class_table.find(ch);
		if (itr == class_table.end()) return false;
		ch = itr->second->get_parent();
		if (ch == pa) return true;
	}
	return false;
}


void attr_class::feature_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
		SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table) 
{
	Symbol ret = init->expr_eval(cla, attr, meth, class_table);
	if (!class_table->class_in_classtable(type_decl)) {  // the type is not defined
		cerr << "Error type" <<endl;
		return;
	}
	// if init is empty, ret is No_type , type_decl can be any legal type
	// if not  then check  if type_decl is legal
	if (ret == No_type) return;
	if (ret == SELF_TYPE) ret = cla;
	if (!class_table->class_inherent_legal(type_decl, ret)) {
		cerr << "attr_class "<< cla << "  "<<name << "  " << ret <<"  " << type_decl << " Type inherent error" << endl;
		return;
	}
}


Symbol no_expr_class::expr_eval(Symbol, SymbolTable<Symbol, Symbol> *,
                        SymbolTable<Symbol, Symbol> *, ClassTable *class_table)
{
	type = No_type;
	return type;
}

Symbol new__class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	// neec to check whether the type is ok
	if (type_name == SELF_TYPE) {
		set_type(SELF_TYPE);
		return  SELF_TYPE;
	}
	if(! class_table->class_in_classtable(type_name)) {
		SEMANT_ERROR << "'new' used with undefined class " << type_name 
				<< "." << endl;
	}
	type = type_name; 
	return type;
}


Symbol isvoid_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1 = e1->expr_eval(EXPR_PARAM1);

	set_type(t1);
	return t1;
}

Symbol string_const_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	type = Str;
	return type;
}

Symbol bool_const_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	type = Bool;
	return type;
}

Symbol int_const_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	type = Int;
	return type;
}

Symbol comp_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{	
	Symbol t1;
	t1 = e1->expr_eval(EXPR_PARAM1);
	if (t1 != Bool) {
		SEMANT_ERROR << "comp op must be Bool." << endl;
	}
	set_type(Bool);
	return Bool;
}

Symbol leq_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1, t2;
	t1 = e1->expr_eval(EXPR_PARAM1);
	t2 = e2->expr_eval(EXPR_PARAM1);
	// need to check the type t1 t2 here ??
	set_type(Bool);
	return Bool;
}

Symbol eq_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1, t2;
	t1 = e1->expr_eval(cla, attr, meth, class_table);
	t2 = e2->expr_eval(cla, attr, meth, class_table);
	if ((t1 == Int || t1 == Str || t1 == Bool) &&  
		(t2 == Int || t2 == Str || t2 == Bool)){     // basic type compare
		if(t1 != t2) {  // just compare int  str and bool
			class_table->semant_error(class_table->get_class_(cla)) 
				<< "Illegal comparison with a basic type." << endl;
			set_type(Bool);	
			return Object;
		}
	}
	set_type(Bool);	
	return Bool;
}

Symbol lt_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1, t2;
	t1 = e1->expr_eval(EXPR_PARAM1);
	t2 = e2->expr_eval(EXPR_PARAM1);
	// need to check the type t1 t2 here ??

	set_type(Bool);
	return Bool;

}

Symbol neg_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t = e1->expr_eval(cla, attr, meth, class_table);
	if (t != Int) SEMANT_ERROR << "neg error type " << t << endl;
	set_type(Int);  // ??
	return Int;
}

Symbol divide_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1 = e1->expr_eval(cla, attr, meth, class_table);
	Symbol t2 = e2->expr_eval(cla, attr, meth, class_table);	
// now we only care Int and Str
	type = t1;
	if (t1 == Int) {
		if(t2 != Int) {
			class_table ->semant_error(class_table->get_class_(cla)) 
				<< "non-Int arguments: "<< Int << " + "<< t2 << endl;
			return Object;
		}
	}
	if(t1 == Str) {
		if(t2 != Int) {
			class_table ->semant_error(class_table->get_class_(cla)) 
				<< "non-Str arguments: "<< Str << " + "<< t2 << endl;
			return Object;
		}
	}
	return type;
}

Symbol mul_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1 = e1->expr_eval(cla, attr, meth, class_table);
	Symbol t2 = e2->expr_eval(cla, attr, meth, class_table);	
// now we only care Int and Str
	type = t1;
	if (t1 == Int) {
		if(t2 != Int) {
			class_table ->semant_error(class_table->get_class_(cla)) 
				<< "non-Int arguments: "<< Int << " + "<< t2 << endl;
			return Object;
		}
	}
	if(t1 == Str) {
		if(t2 != Int) {
			class_table ->semant_error(class_table->get_class_(cla)) 
				<< "non-Str arguments: "<< Str << " + "<< t2 << endl;
			return Object;
		}
	}
	return type;
}

Symbol sub_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1 = e1->expr_eval(cla, attr, meth, class_table);
	Symbol t2 = e2->expr_eval(cla, attr, meth, class_table);	
	// now we only care Int and Str Ok ??
	type = t1;
	if (t1 == Int) {
		if(t2 != Int) {
			class_table ->semant_error(class_table->get_class_(cla)) 
				<< "non-Int arguments: "<< Int << " + "<< t2 << endl;
			return Object;
		}
	}
	if(t1 == Str) {
		if(t2 != Int) {
			class_table ->semant_error(class_table->get_class_(cla)) 
				<< "non-Str arguments: "<< Str << " + "<< t2 << endl;
			return Object;
		}
	}
	return type;
}

Symbol plus_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1 = e1->expr_eval(cla, attr, meth, class_table);
	Symbol t2 = e2->expr_eval(cla, attr, meth, class_table);	
	// now we only care Int and Str  OK ??
	type = t1;
	if (t1 == Int) {
		if(t2 != Int) {
			class_table ->semant_error(class_table->get_class_(cla)) 
				<< "non-Int arguments: "<< Int << " + "<< t2 << endl;
			return Object;
		}
	}
	if(t1 == Str) {
		if(t2 != Int) {
			class_table ->semant_error(class_table->get_class_(cla)) 
				<< "non-Str arguments: "<< Str << " + "<< t2 << endl;
			return Object;
		}
	}
	return type;
}

Symbol let_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
#if 0
	let_list :
    OBJECTID ':' TYPEID IN expression_single
    { $$ = let($1,$3,no_expr(),$5);}
    | OBJECTID ':' TYPEID ASSIGN expression_single IN expression_single
    { $$ = let($1,$3,$5,$7);}
    | OBJECTID ':' TYPEID ',' let_list
    { $$ = let($1,$3,no_expr(),$5);}
    | OBJECTID ':' TYPEID ASSIGN expression_single ',' let_list
    { $$ = let($1,$3,$5,$7);}

   let_class(Symbol a1, Symbol a2, Expression a3, Expression a4) {
#endif

	Symbol t;
	if (type_decl != SELF_TYPE) {
		if (!class_table->class_in_classtable(type_decl)) {
			class_table->semant_error(class_table->get_class_(cla)) 
				<< "error let type" << type_decl << " in "<< cla << endl;
			return Object;
		}
	}
	
	if (identifier == self) {
		SEMANT_ERROR << "'self' cannot be bound in a 'let' expression." << endl;
		return Object;
	}
	// notice the scope
	attr->enterscope();
	attr->addid(identifier, new Symbol(type_decl));
	t = init->expr_eval(cla, attr, meth, class_table);
	if (t != No_type) {
		if (!class_table->class_inherent_legal(type_decl, t)) {
			SEMANT_ERROR << "Inferred type "<< t <<" of initialization of " 
					<< identifier << " does not conform to identifier's declared type " 
					<< type_decl << "." << endl;
			return Object;
		}
	}
	t = body->expr_eval(cla, attr, meth, class_table);
	attr->exitscope(); // when the whole let finished exit the scope 

	set_type(t);
	return t;  // return what ??
}


Symbol inherent_parent(Symbol t1, Symbol t2, ClassTable *class_table)
{
	if(t1 == t2 ) return t1;

	Symbol t11 = t1;
	Symbol t12 = t2;
	Class_ c1 = class_table->get_class_(t1);
	Class_ c2 = class_table->get_class_(t2);
	while (t11 != No_class) {
		if (t11 == t2) return t2;
		t11 = c1->get_parent();
		c1 = class_table->get_class_(t11);
	}
	while (t12 != No_class) {
		if (t12 == t1) return t1;
		t12 = c2->get_parent();
		c2 = class_table->get_class_(t12);
	}
	return Object;  // ??
}

Symbol common_parent(Symbol t1, Symbol t2, ClassTable *class_table)
{
	if(t1 == t2 ) return t1;

	Symbol t11 = t1;
	Symbol t12 = t2;
	
	Class_ c1 = class_table->get_class_(t1);
	Class_ c2 = class_table->get_class_(t2);
	while (t11 != No_class) {

		c2 = class_table->get_class_(t2);
		t12 = t2;
		while(t12 != No_class) {
			if (t12 == t11) return t12;
			c2 = class_table->get_class_(t12);
			t12 = c2->get_parent();
		}
		c1 = class_table->get_class_(t11);
		t11 = c1->get_parent();
	}

	return Object;  // ??
}

Symbol typcase_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol ret;
	Symbol t1 = expr->expr_eval(cla, attr, meth, class_table);
	std::map<Symbol, bool> case_types;
	//attr->enterscope();
	ret = No_type;
	for (int i = cases->first(); cases->more(i); i = cases->next(i)) {
		Case case_ = cases->nth(i);
		Symbol t = case_->get_type();
		std::map<Symbol, bool>::iterator itr;
		itr = case_types.find(t);
		if(itr != case_types.end()) {
			SEMANT_ERROR << "Duplicate branch "<< t
					<< " in case statement." << endl;
			continue;
		}
		case_types[t] = true;
		attr->enterscope();
		attr->addid(case_->get_name(), new Symbol(t)); // need ??
		Symbol t1 = case_->get_expr()->expr_eval(cla, attr, meth, class_table);
		attr->exitscope();
		// need to check t and t1 here ?? test case do not test this one
		if (!class_table->class_inherent_legal(t, t1)) {
			SEMANT_ERROR << "error case decl_type and expr_type."  << endl;
			set_type(Object);
			return Object;
		}
		if (ret == No_type) ret = t1;  // the first case 
		else ret = common_parent(ret, t1, class_table);
		//ret = inherent_parent(ret, t, class_table);
	}
	//attr->exitscope();
	// evaluate the each branch
	// notice the return type 
	set_type(ret);
//	cerr << "case ret " << ret << endl;
	return ret;
}

Symbol loop_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1 = pred->expr_eval(cla, attr, meth, class_table);
	if (t1 != Bool) {
		SEMANT_ERROR << "Loop condition does not have type Bool." << endl;
	}
	body->expr_eval(cla, attr, meth, class_table);
	set_type(Object);
	return Object;
}

Symbol cond_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol t1 ,t2, t3;
	t1 = pred->expr_eval(EXPR_PARAM1);
	if (t1 != Bool) {
		SEMANT_ERROR << "pred is not Bool" << endl;
	}
	t2 = then_exp->expr_eval(EXPR_PARAM1);
	t3 = else_exp->expr_eval(EXPR_PARAM1);
	if (t2 == SELF_TYPE) t2 = cla;
	if (t3 == SELF_TYPE) t3 = cla;
	t1 = common_parent(t2, t3, class_table);// to find which is the parent
	set_type(t1);  // notice the return type here
	return t1;
}


Feature ClassTable::method_in_classtable(Symbol cla, Symbol method)
{
    std::map<Symbol, Class_>::iterator itr;
    itr = class_table.find(cla);
    if (itr == class_table.end()) return NULL;

	Feature result = NULL;
	Symbol pa = cla;
	Class_ pac = itr->second;
	while (pa != No_class) {
		Features fs = pac->get_features();
		for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
			Feature feature = fs->nth(i);
			if (feature->get_name() == method) {
				result = feature;
				break;
			}	
		}
		pa = pac->get_parent();
		itr = class_table.find(pa);
		pac = itr->second;
	}
// finish it later
	
    return result;
}


Symbol static_dispatch_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
// expr[@TYPE].ID( [ expr [[, expr]]* )
// well, first we should check expr type and @TYPE, then check id is in @TYPE
// then parameters

	// now check if the function is in 
	Symbol t1;
	Feature feature;
	type = Object;
	
	
	t1 = expr->expr_eval(cla, attr, meth, class_table);	
	bool legal = false;
	if (t1 == SELF_TYPE) {
		legal = class_table->class_inherent_legal(type_name, cla);
	}
	else {
		legal = class_table->class_inherent_legal(type_name, t1);
		
	}
	if(!legal) {
		SEMANT_ERROR <<"Expression type "<< t1 <<" does not conform to "
			<< "declared static dispatch type "<< type_name << "." <<endl;
		set_type(Object);
		return Object;
	}

	// check the method and get the return type of the method
    feature = class_table->method_in_classtable(t1, name);
	if (feature == NULL) {
		type = Object; 
		return type;
	}
	else {  // find the method  and set the type to the return type of the method 
		type = feature->get_type();  
	}
	Formals formals = feature->get_formals();
 
	// check the parameter
	int len1 = actual->len();
    int len2 = formals->len();
	if (len1 != len2) {
		cerr << "in static_dispatch_class::expr_eval error parameter list len" << endl;
		type = Object;	
	}
	else {
		for(int i = formals->first(); formals->more(i); i = formals->next(i)) {
			Formal f1 = formals->nth(i);
			Expression f2 = actual->nth(i);
			Symbol t1 = f1->get_type();
			Symbol t2 = f2->expr_eval(cla, attr, meth, class_table);
			//cerr << "t1: "<< t1 << ", t2: " << t2 <<endl;
			if(!class_table->class_inherent_legal(t1, t2)) {
				type = Object;
				cerr << "in static_dispatch_class::expr_eval parameter type error" << endl;
			}
		}
	}
	return type;
}

Symbol dispatch_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                 SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
//   Expression expr;
//   Symbol name;
//   Expressions actual; 
	Symbol t1;
	Feature feature;
	// t1 is the expr type
	t1 = expr->expr_eval(cla, attr, meth, class_table);
	if (t1 == SELF_TYPE) {
		// expr is empty call the method  of itself method
		feature = class_table->method_in_classtable(cla, name);// name is the method  
	}
	else { 
		// expr if of type Class t1  find the method in Class t1
		feature = class_table->method_in_classtable(t1, name);// name is the method  
	}
	if (feature == NULL) {
		Class_ cc = class_table->get_class_(cla);
		if (cc == NULL) cerr << "error class " << cla <<endl;
		class_table->semant_error(class_table->get_class_(cla))
			<< "Dispatch to undefined method " << name << "."<< endl;
		type = Object; 
		return type;
	}
	// find the method  and set the type to the return type of the method 
	type = feature->get_type();
	if (type == SELF_TYPE) {
		type = t1;
	}
	
	Formals formals = feature->get_formals();
	// check the parameter
	int len1 = actual->len();
    int len2 = formals->len();
	if (len1 != len2) {
		cerr << "in dispatch_class::expr_eval error parameter list "
			 << len1 << " " << len2 << endl;
		type = Object;
		return type;	
	}
	else {
		for(int i = formals->first(); formals->more(i); i = formals->next(i)) {
			Formal f1 = formals->nth(i);
			Expression f2 = actual->nth(i);
			Symbol t1 = f1->get_type();
			Symbol t2 = f2->expr_eval(cla, attr, meth, class_table);
			if (t2 == SELF_TYPE) {
				if (!class_table->class_inherent_legal(t1, cla)) {
					SEMANT_ERROR << "In call of method " << name 
						<< ", type SELF_TYPE of parameter "
						<< f1->get_name() << " does not conform to declared type "
						<< t1 << "." << endl;
					set_type(Object);
					return Object;
				}
			}
			else {
				if(!class_table->class_inherent_legal(t1, t2)) {
					SEMANT_ERROR << "In call of method " << name 
						<<", type Object of parameter "<<  f1->get_name() 
						<<" does not conform to declared type "<< t1 << "." <<endl;
					set_type(Object);
					return Object;
				}
			}
		}
	}
	return type;
}



Symbol object_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
                        SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	Symbol *t1;
	type = Object;

	if (name == self) type = SELF_TYPE;
	else {
		t1 = attr->lookup(name); // or first use probe ??
		if (t1 == NULL) {
			class_table->semant_error(class_table->get_class_(cla)) << 
				"Undeclared identifier "<< name << "."<<endl;
		}
		else {
			type = *t1;
		}
	}
	return type;
}      

Symbol assign_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
             SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{

	if (name == self) {
		SEMANT_ERROR << "Cannot assign to 'self'." << endl;
		set_type(cla);
		return cla;
	}

	Symbol *t1, t2;
	t1 = attr->lookup(name);
	type = *t1;
	if (t1 == NULL ) {
		cerr << "error assign_class expr_eval" << endl;
		type = Object; // set the type
	}
	else {
		t2 = expr->expr_eval(cla, attr, meth, class_table);
		if(class_table->class_inherent_legal(*t1, t2)) {
			type = t2;
		}
		else {
			class_table->semant_error(class_table->get_class_(cla)) << "Type "
				<< t2 << " of assigned expression does not conform to declared type "
				<< *t1 <<" of identifier " << name << "." << endl;
			type = Object;
		}
	}
	return type;
}

Symbol block_class::expr_eval(Symbol cla, SymbolTable<Symbol, Symbol> *attr,
             SymbolTable<Symbol, Symbol> *meth, ClassTable *class_table)
{
	//those exp_eval calls will set the type of each expression
	for (int i = body->first(); body->more(i); i = body->next(i)) {
		type = body->nth(i)->expr_eval(cla, attr, meth, class_table);
	}
	return type;
}


// notice this recursive function
// it will do the wright sequence
// add all attrs and methods to the tables
void ClassTable::install_inherent_features(Class_ cur_cla,
		SymbolTable <Symbol, Symbol> *meth_table, 
		SymbolTable <Symbol, Symbol> *attr_table)
{
	Symbol pa = cur_cla->get_parent();  // get the parent class
	if (pa != No_class) {
		std::map<Symbol,Class_>::iterator itr = class_table.find(pa);
		install_inherent_features(itr->second, meth_table, attr_table);
	}
	meth_table->enterscope();
	attr_table->enterscope();
	Features fs = cur_cla->get_features();
	for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
		Feature feature = fs->nth(i);
		feature->add_feature(cur_cla, attr_table, meth_table, this);
	}
}

// if do like this , the seqence of the table is wrong
/*
void ClassTable::install_inherent_features(Class_ cla,
		SymbolTable <Symbol, Symbol> *meth_table, 
		SymbolTable <Symbol, Symbol> *attr_table)
{
	std::map<Symbol,Class_>::iterator itr;
	Class_ cur_cla = cla;
	Symbol cur_sym = cur_cla->get_name();
	while((cur_sym) != No_class) {
		meth_table->enterscope();
		attr_table->enterscope();
		Features fs = cur_cla->get_features();
		for (int i = fs->first(); fs->more(i); i = fs->next(i)) {
			Feature feature = fs->nth(i);
			if (feature_type_is_method(feature)) {
				feature->add_feature(cur_cla->get_name(), meth_table);
			}
			else {
				feature->add_feature(cur_cla->get_name(), attr_table);
			}
		}
		cur_sym = cur_cla->get_parent(); // find the symbol of parent
		itr = class_table.find(cur_sym); // trans to Class_ of the parent
		cur_cla = itr->second;
	}
}
*/

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes);

	if (classtable->errors()) {
		cerr << "Compilation halted due to static semantic errors." << endl;
		exit(1);
    }
    /* some semantic analysis code may go here */

	for(int i = classes->first(); classes->more(i); i = classes->next(i)) {
		// put them in the feature class
		SymbolTable <Symbol, Symbol> *meth_table 
				= new SymbolTable<Symbol, Symbol>();
		SymbolTable <Symbol, Symbol> *attr_table
				= new SymbolTable<Symbol, Symbol>();
		Class_ curr_class = classes->nth(i);

		classtable->install_inherent_features(curr_class, 
					meth_table, attr_table);

		Features fs = curr_class->get_features(); 
		for (int j = fs->first(); fs->more(j); j = fs->next(j)) {
			meth_table->enterscope();
			attr_table->enterscope();
			fs->nth(j)->feature_eval(curr_class->get_name(), 
				attr_table, meth_table, classtable);  // chcek 
			meth_table->exitscope();
			attr_table->exitscope();
		}
		delete meth_table;
		delete attr_table;
	}

	if (classtable->errors()) {
		cerr << "Compilation halted due to static semantic errors." << endl;
		exit(1);
    }
	
}


