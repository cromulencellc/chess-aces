#ifndef __VALUE_HPP__
#define __VALUE_HPP__

#include "node.h"
#include "exceptions.hpp"
#include <string.h>

enum valueType {
	integer_t = 0x73746e69,
	double_t = 0x6c627564,
	string_t = 0x6e727473,
	list_t = 0x7473696c,
};

typedef struct string {
	char *s;
	int length;
} string;

typedef struct list {
	Value **l;
	int max;
	int index;
} list;

class Value {
public:
	long long i;
	double d;
	string s;

	list lt;

	//std::vector< Value *> l;

	valueType t;

	~Value( ) { }
	Value( ) { }
	Value( valueType t) :t(t) { if ( t == list_t) { lt.l = NULL; lt.index = 0; lt.max=0;} }
	Value( long long i ) : i(i) { t = integer_t; }
	Value( double d ) : d(d) { t = double_t; }
	Value( string *ns ) { t = string_t; this->s.length = ns->length; this->s.s = new char[ns->length + 1]; memset( this->s.s, 0, this->s.length + 1); memcpy( this->s.s, ns->s, ns->length); }
	Value( char *ns ) { t = string_t; this->s.length = strlen(ns); this->s.s = ns; }
	Value( std::string s ) { t = string_t; this->s.length = s.size(); this->s.s = new char[s.size() + 1]; memset( this->s.s, 0, s.size() + 1); memcpy( this->s.s, s.c_str(), s.size()); }
	//Value( std::vector< Value *> l) { t = list_t; }
	Value( list *nl) { t = list_t; lt.l = (Value **)calloc( 1, nl->max * sizeof(Value *)); lt.max = nl->max; lt.index = nl->index; memcpy( lt.l, nl->l, nl->max * sizeof(Value *)); }
	
	Value *addValues( Value *a );

	Value *subValues( Value *a );

	Value *mulValues( Value *a );
	Value *divValues( Value *a );
	Value *modValues( Value *a );
	Value *isEqual( Value *a );
	Value *notEqual( Value *a );
	Value *lessThan( Value *a );
	Value *lessThanEqual( Value *a );
	Value *greaterThan( Value *a );
	Value *greaterThanEqual( Value *a );

	Value *setValue( Value *a );
	Value *copy( );

	void print( );

	std::string typeString( );
	std::string toString( );
};



#endif