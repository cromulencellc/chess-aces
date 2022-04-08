#include "node.h"
#include "context.hpp"
#include "parser.hpp"
#include "value.hpp"
#include "builtins.hpp"

extern Context context_g;

Value *NInteger::execute(Context* context )
{
	Value *v = new Value( value );

	return v;
}

Value* NDouble::execute(Context* context )
{
	Value *v = new Value( value );
	return v;
}

Value* NString::execute(Context* context )
{
	char *ns = new char[value.size() + 1];
	memcpy(ns, value.c_str(), value.size());

	Value *v = new Value( value );
	return v;
}

Value* NList::execute(Context* context )
{
	Value *v = new Value( list_t );

	for ( auto i: elements ) {
		append_builtin( v, i->execute(context) );
	}

	return v;
}

Value* NIdentifier::execute(Context* context)
{
	NVariable* id = NULL;

	if ( context ) {
		id = context->getVariableIdentifier( name );
	}

	if ( id == NULL) {
		id = context_g.getVariableIdentifier( name );
	}

	if ( id == NULL ) {
		throw InvalidIdentifierException(name);
	}

	return id->v;
}

Value* NMethodCall::execute(Context* context )
{
	// Search the context for the function id
	// execute it
	NFunction *f = NULL;

	if ( context ) {
		f = context->getFunctionIdentifier( id.name );
	} 

	if ( f == NULL) {
		f = context_g.getFunctionIdentifier( id.name );
	}

	if ( f == NULL ) {
		// Last chance since it could still be a built in function
		NBuiltInFunction *nbf = context_g.getBuiltInIdentifier(id.name);

		if ( nbf == NULL ) {
			throw UnknownFunctionException(id.name);
		}
		
		if ( arguments.size() != nbf->arguments.size() ) {
			throw InvalidFunctionArgCountException(id.name);
		}

		std::vector<Value *> args;

		// Check argument types and evaluate
		for (int i = 0; i < arguments.size(); i++ ) {
			Value *nv = arguments[i]->execute( context );

			if ( nv == NULL ) {
				throw GenericException("failed resolving an argument expression");
			}

			if ( nv->typeString() != nbf->arguments[i]->type.name && nbf->arguments[i]->type.name != "*") {
				throw InvalidTypeException("invald type for an argument");
			}

			args.push_back( nv );
		}

		return nbf->execute_builtin( args );
	}

	// Make sure the argument count is correct
	if ( arguments.size() != f->arguments.size() ) {
		throw InvalidFunctionArgCountException(id.name);
	}

	Context locals(id.name);

	// Check argument types
	for (int i = 0; i < arguments.size(); i++ ) {
		NVariableDeclaration *nvd = new NVariableDeclaration( f->arguments[i]->type, f->arguments[i]->id, arguments[i]);
		nvd->execute(&locals);

	}

	Value *rv = f->execute(&locals);

	return rv;
}


Value* NBinaryOperator::execute(Context* context )
{
	Value *vl = lhs.execute(context);
	Value *vr = rhs.execute(context);

	Value *result = NULL;

	if ( vr == NULL ) {
		throw GenericException( "failed to resolve right side of expression" );
	}

	if (vl == NULL ) {
		throw GenericException( "failed to resolve left side of expression" );
	}

	// TODO probably a switch statement here.
	if ( op == TPLUS ) {
		result = vl->addValues(vr);
	} else if ( op == TMINUS ) {
		result = vl->subValues(vr);
	} else if ( op == TMUL ) {
		result = vl->mulValues(vr);
	} else if ( op == TDIV ) {
		result = vl->divValues(vr);
	} else if ( op == TMOD ) {
		result = vl->modValues(vr);
	} else if ( op == TCEQ ) {
		result = vl->isEqual(vr);
	} else if ( op == TCNE ) {
		result = vl->notEqual(vr);
	} else if ( op == TCLT ) {
		result = vl->lessThan(vr);
	} else if ( op == TCLE ) {
		result = vl->lessThanEqual(vr);
	} else if ( op == TCGT ) {
		result = vl->greaterThan(vr);
	} else if ( op == TCGE ) {
		result = vl->greaterThanEqual(vr);
	} else {
		return NULL;
	}

	return result;
}

Value* NAssignment::execute(Context* context)
{
	NVariable *var = NULL;

	if ( context ) {
		var = context->getVariableIdentifier( lhs.name );
	}

	if ( var == NULL ) {
		var = context_g.getVariableIdentifier( lhs.name );
	}

	if ( var == NULL ) {
		throw InvalidIdentifierException( lhs.name );
	}

	Value *v = rhs.execute(context);

	if ( v == NULL ) {
		throw GenericException("assignment expression failed");
	}

	if ( var->v->typeString() != v->typeString() ) {
		throw InvalidTypeException( v->typeString() );
	}

	if ( var->v == NULL ) {
		var->v = v->copy();
	} else {
		var->v->setValue(v);
	}
	
	return NULL;
}

Value* NBlock::execute(Context* context )
{
	Value *v = NULL;

	// Todo. I need a way to exit the block in the middle of statments
	for ( auto it = statements.begin(); it != statements.end(); ++it) {
    	v = (**it).execute(context);


    	// The return should only ever be set if we are in a local context
    	if ( context ) {
    		if ( context->returnSet() ) {
    		return context->getReturnValue();
    		}
    	}
    }

	return NULL;
}

Value* NReturnStatement::execute(Context* context )
{
	// Have to make sure that it is executing inside of a function call

	if ( context == NULL ) {
		throw InvalidOperationException("return is not inside of a function");
	}

	Value *v = expression.execute(context);

	if (v == NULL ) {
		throw GenericException("return value expression failed");
	}

	context->setReturnValue( v );

	return v;
}

Value* NExpressionStatement::execute(Context* context )
{
	// TODO make sure that I should be returning this
	Value *v = expression.execute(context);

	return v;
}

NVariable::~NVariable ( )
{
	if (v ) {
		delete v;
	}
}

Value* NVariable::execute(Context* context )
{
	return v;
}

Value* NVariableDeclaration::execute(Context* context )
{
	// Add a new variable to the context and assign a value if possible
	NVariable *nv = new NVariable( type, id );

	if ( type.name != "void" && type.name != "int" && type.name != "string" && type.name != "double" && type.name != "list" && type.name != "*") {
		delete nv;
		throw InvalidTypeException("invalid type in variable declaration");
	}

	if ( assignmentExpr ) {
		nv->v = assignmentExpr->execute(context );

		/// TODO this could be a bug
		if ( nv->v == NULL ) {
			throw GenericException("assignment failed");
		}

		if ( nv->v->typeString() != type.name && type.name != "*") {
			throw InvalidTypeException( nv->v->typeString() );
		}
	}

	if ( context ) {
		context->addVariableIdentifier(id.name, nv);
	} else {
		context_g.addVariableIdentifier( id.name, nv);
	}

	return NULL;
}

Value *NForStatement::execute( Context* context )
{
	Value *v = NULL; 
	int con = 1;

	/// Execute the init
	init.execute(context);

	// Check the first condition
	v = condition.execute(context);

	if ( v == NULL ) {
		throw GenericException("for condition expression failed");
	}

	if ( v->t == integer_t ) {
		con = v->i;
	} else if ( v->t == double_t ) {
		con = v->d;
	} else {
		throw InvalidTypeException("for condition expression is of an invalid type");
	}

	while ( con ) {
		/// execute the block
		forBlock.execute(context);

		increment.execute(context);

		v = condition.execute(context);

		if ( v == NULL ) {
			throw GenericException("for condition expression failed");
		}

		if ( v->t == integer_t ) {
			con = v->i;
		} else if ( v->t == double_t ) {
			con = v->d;
		} else {
			throw InvalidTypeException("for condition expression is of an invalid type");
		}
	}

	return v;
}

Value *NForEachStatement::execute( Context* context )
{
	Value *v = NULL; 

	v = loopList.execute(context);

	if ( v == NULL ) {
		throw GenericException("foreach expression invalid");
	}

	if ( v->t != list_t ) {
		throw GenericException("foreach expects a list");
	}


	NIdentifier nid( "$$" );

	for (int i = 0; i < v->lt.index; i++) {
		Value *vcopy = v->lt.l[i]->copy();

		if ( vcopy == NULL ) {
			throw GenericException("failed to copy value");
		}

		NVariable *nv = new NVariable( vcopy->typeString(), nid, vcopy);

		if ( context ) {
			context->addVariableIdentifier( "$$", nv );
		} else {
			context_g.addVariableIdentifier( "$$", nv );
		}

		forBlock.execute(context);
	}

	return NULL;
}

/// TODO make a "break" statement
Value *NWhileStatement::execute( Context* context )
{
	Value *v = NULL; 
	int con = 1;

	while ( con ) {
		v = condition.execute(context);

		if ( v == NULL ) {
			throw GenericException("while condition expression failed");
		}

		if ( v->typeString() == "int" ) {
			con = v->i;
		} else if ( v->typeString() == "double" ) {
			con = v->d;
		} else {
			throw InvalidTypeException("while condition expression is of an invalid type");
		}

		if ( con ) {
			whileBlock.execute( context );
		}
	}

	return v;
}

Value *NDoWhileStatement::execute( Context* context )
{
	Value *v = NULL; 
	int con = 1;

	do {
		whileBlock.execute( context );

		v = condition.execute(context);

		if ( v == NULL ) {
			throw GenericException("while condition expression failed");
		}

		if ( v->typeString() == "int" ) {
			con = v->i;
		} else if ( v->typeString() == "double" ) {
			con = v->d;
		} else {
			throw InvalidTypeException("while condition expression is of an invalid type");
		}

	} while ( con ); 

	return v;
}

Value *NBuiltInFunction::execute_builtin( std::vector<Value *> args )
{
	Value * result = NULL;

	/// By the time this is reached it is expected that both the argument count and type have been validated
	if ( id.name == "len") {
		result = len_builtin( args[0] );
	} else if ( id.name == "puts" ) {
		result = puts_builtin( args[0] );
	} else if ( id.name == "puti" ) {
		result = puti_builtin( args[0]->i );
	} else if ( id.name == "putd" ) {
		result = putd_builtin( args[0]->d );
	} else if ( id.name == "putl" ) {
		result = putl_builtin( args[0] );
	} else if ( id.name == "gets" ) {
		result = gets_builtin( );
	} else if ( id.name == "append" ) {
		result = append_builtin( args[0], args[1] );
	} else if ( id.name == "prepend" ) {
		result = prepend_builtin( args[0], args[1] );
	} else if ( id.name == "popend" ) {
		result = popend_builtin( args[0]  );
	} else if ( id.name == "popfront" ) {
		result = popfront_builtin( args[0]  );
	} else if ( id.name == "getbyindex" ) {
		result = getbyindex_builtin( args[0], args[1]->i );
	} else if ( id.name == "exists" ) {
		result = exists_builtin( args[0], args[1] );
	} else if ( id.name == "erase" ) {
		result = erase_builtin( args[0], args[1]->i );
	} else if ( id.name == "exit" ) {
		result = exit_builtin( args[0]->i );
	} else if ( id.name == "atoi" ) {
		result = atoi_builtin( args[0] );
	} else if ( id.name == "hex" ) {
		result = hex_builtin( args[0]->i );
	} else if ( id.name == "itos" ) {
		result = itos_builtin( args[0]->i );
	} else if ( id.name == "dtos" ) {
		result = dtos_builtin( args[0]->d );
	} else if ( id.name == "str" ) {
		result = str_builtin( args[0] );
	} else if ( id.name == "substr" ) {
		result = substr_builtin( args[0], args[1]->i, args[2]->i );
	} else if ( id.name == "strstr" ) {
		result = strstr_builtin( args[0], args[1] );
	} else if ( id.name == "setchr" ) {
		result = setchr_builtin( args[0], args[1], args[2]->i );
	} else if ( id.name == "type" ) {
		result = type_builtin( args[0] );
	} else if ( id.name == "tolower" ) {
		result = tolower_builtin( args[0] );
	} else if ( id.name == "toupper" ) {
		result = toupper_builtin( args[0] );
	} else if ( id.name == "sum" ) {
		result = sum_builtin( args[0] );
	} else if ( id.name == "max" ) {
		result = max_builtin( args[0]->i, args[1]->i );
	} else if ( id.name == "min" ) {
		result = min_builtin( args[0]->i, args[1]->i );
	} else if ( id.name == "bin" ) {
		result = bin_builtin( args[0]->i );
	} else if ( id.name == "strtok" ) {
		result = strtok_builtin( args[0], args[1] );
	} else if ( id.name == "oct" ) {
		result = oct_builtin( args[0]->i );
	} else {
		throw GenericException("unknown builtin function");
	}

	return result;
}

Value *NBuiltInFunction::execute( Context *context )
{
	return NULL;
}

Value *NIfStatement::execute( Context* context )
{
	Value *v = condition.execute(context);

	if ( v == NULL ) {
		throw GenericException("if condition expression failed");
	}

	int con = 0;

	if ( v->typeString() == "int" ) {
		con = v->i;
	} else if ( v->typeString() == "double" ) {
		con = v->d;
	} else {
		throw InvalidTypeException("if condition expression is of an invalid type");
	}

	if ( con ) {
		ifBlock.execute( context );
	}

	return v;
}

Value *NIfElseStatement::execute( Context* context )
{
	Value *v = condition.execute(context);

	if ( v == NULL ) {
		throw GenericException("if condition expression failed");
	}

	int con = 0;

	if ( v->typeString() == "int" ) {
		con = v->i;
	} else if ( v->typeString() == "double" ) {
		con = v->d;
	} else {
		throw InvalidTypeException("if condition expression is of an invalid type");
	}

	if ( con ) {
		ifBlock.execute( context );
	} else {
		elseBlock.execute( context );
	}

	return v;
}

Value* NFunction::execute(Context* context )
{
	// Add a new function to the context
	Context *locals = NULL;
	Value *returnValue = NULL;

	// The block execution will handle modifying the correct context
	//	return value. The locals variable will be the one holding it.
	if ( context == NULL ) {
		locals = &context_g;
	} else {
		locals = context;
	}

	returnValue = block.execute(context);

	/// This should only be set if a return is hit
	///  Once hit though it needs to confirm that the return type is the same as the function declaration
	if (locals->returnSet()) {
		returnValue = locals->getReturnValue();

		if (returnValue->typeString() != type.name ) {
			throw InvalidTypeException("return value is not the same as the defined function return type");
		}

		locals->clearReturn();

		return returnValue;
	} else {
		// If function execution completed without a return value make sure that it is a void type
		if ( type.name != "void") {
			throw InvalidTypeException("function expected a return value");
		}
	}

	return returnValue;
}

Value* NFunctionDeclaration::execute(Context* context )
{
	NFunction *nf = new NFunction(type, id, arguments, block);

	if ( type.name != "void" && type.name != "int" && type.name != "string" && type.name != "double") {
		delete nf;
		throw InvalidTypeException("invalid type in function declaration");
	}

	// Add a new function to the context

	if ( context == NULL ) {
		context_g.addFunctionIdentifier(id.name, nf);
	} else {
		context->addFunctionIdentifier(id.name, nf);
	}
	
	return NULL;
}