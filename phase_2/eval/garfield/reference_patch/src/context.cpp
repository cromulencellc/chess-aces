#include "context.hpp"
#include "exceptions.hpp"

Context::Context()
{
	
	return;
}

void Context::addFunctionIdentifier( std::string id, NFunction *func)
{
	auto bit = builtInIdentifiers.find(id);

	if ( bit != builtInIdentifiers.end() ) {
		throw GenericException("cannot overwrite a built-in function");
	}

	auto it = functionIdentifiers.find(id);

	if ( it != functionIdentifiers.end() ) {
		functionIdentifiers.erase(it);
	}

	functionIdentifiers.insert ( std::pair<std::string, NFunction *>(id, func) );

	return;
}

void Context::addBuiltInIdentifier( std::string id, NBuiltInFunction *func)
{

	auto it = builtInIdentifiers.find(id);

	if ( it != builtInIdentifiers.end() ) {
		builtInIdentifiers.erase(it);
	}

	builtInIdentifiers.insert ( std::pair<std::string, NBuiltInFunction *>(id, func) );

	return;
}

void Context::addVariableIdentifier( std::string id, NVariable *var)
{	
	auto bit = builtInIdentifiers.find(id);

	if ( bit != builtInIdentifiers.end() ) {
		throw GenericException("cannot overwrite a built-in function");
	}

	auto it = variableIdentifiers.find(id);

	if ( it != variableIdentifiers.end() ) {
		variableIdentifiers.erase(it);
	}

	variableIdentifiers.insert ( std::pair<std::string, NVariable *>(id, var) );

	return;
}

NBuiltInFunction *Context::getBuiltInIdentifier( std::string id )
{
	auto it = builtInIdentifiers.find(id);

	if ( it == builtInIdentifiers.end() ) {
		return NULL;
	}

	return it->second;
}

NFunction *Context::getFunctionIdentifier( std::string id )
{
	auto it = functionIdentifiers.find(id);

	if ( it == functionIdentifiers.end() ) {
		return NULL;
	}

	return it->second;
}

NVariable *Context::getVariableIdentifier( std::string id )
{
	auto it = variableIdentifiers.find(id);

	if ( it == variableIdentifiers.end() ) {
		return NULL;
	}

	return it->second;
}