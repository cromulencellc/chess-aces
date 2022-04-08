#ifndef __CONTEXT_HPP__
#define __CONTEXT_HPP__

// Context needs:
// Currently declared functions
// Currently declared variables
// A way do push a child context
// A way to pop a context
// A way to search for an identifier from child through parent contexts

// identifiers are not specific to functions or variables

#include <iostream>
#include <map>

#include "node.h"
#include "value.hpp"

class Context {
	std::string name;

	std::map<std::string, NFunction *> functionIdentifiers;
	std::map<std::string, NBuiltInFunction *> builtInIdentifiers;
	std::map<std::string, NVariable *> variableIdentifiers;

	Value *returnValue;
public:
	Context();
	Context(std::string name) : name(name) { returnValue = NULL; };

	void addFunctionIdentifier( std::string id, NFunction *func);
	void addBuiltInIdentifier( std::string id, NBuiltInFunction *func);
	void addVariableIdentifier( std::string id, NVariable *var);

	void setReturnValue( Value *returnValue) { this->returnValue = returnValue; }
	Value *getReturnValue( void ) { return this->returnValue; }
	int returnSet( void ) { return this->returnValue != NULL; }
	void clearReturn( void ) { returnValue = NULL; }

	NFunction *getFunctionIdentifier( std::string id );
	NBuiltInFunction *getBuiltInIdentifier( std::string id );
	NVariable *getVariableIdentifier( std::string id );
};

#endif