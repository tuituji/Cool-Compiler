#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"
// add
#include <map>
/////
#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;

// add
  std::map<Symbol, Class_> class_table;
  inline void add_class(Class_ c) {
    class_table[c->get_name()] = c;
  }

//////

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);

// add
  void install_inherent_features(Class_ cur_cla,
			SymbolTable <Symbol, Symbol> *meth_table,
			SymbolTable <Symbol, Symbol> *attr_table);
  inline bool class_in_classtable(Symbol name) {
	std::map<Symbol, Class_>::iterator itr;
	itr = class_table.find(name);
	return (itr != class_table.end());
  }
  inline Class_ get_class_(Symbol name)  {
    std::map<Symbol, Class_>::iterator itr;
	itr = class_table.find(name);
	return itr->second;	
  }
  Feature method_in_classtable(Symbol cla, Symbol method);

  bool class_inherent_legal(Symbol pa, Symbol ch);
  bool class_inherent_cycle(Symbol cla);
//////
};


#endif

