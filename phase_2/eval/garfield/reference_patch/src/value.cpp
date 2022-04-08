#include "value.hpp"
#include <string.h>

std::string Value::typeString( )
{
	if ( t == integer_t ) {
		return "int";
	} else if ( t == double_t ) {
		return "double";
	} else if ( t == string_t ) {
		return "string";
	} else if ( t == list_t ) {
		return "list";
	}

	throw InvalidTypeException("unknown");
}

// I wonder if throwing an exception you could cause this to overflow
// It may be an interesting bug to do that
// TODO turn this into a bug!!
int depth = 0;
std::string Value::toString( )
{
	if ( depth > 3 ) {
		depth = 0;
		return "[]";
	}

	std::string result = "";
	switch (t) {
		case string_t:
			result = "\"" + std::string(s.s) + "\"";
			break;
		case integer_t:
			result = std::to_string( i );
			break;
		case double_t:
			result = std::to_string( d );
			break;
		case list_t:
			result = "[";

			for ( int i = 0; i < lt.index; i++ ) {
				depth += 1;
				result += lt.l[i]->toString();

				result += ", ";
			}

			if (lt.index != 0) {
				result.resize( result.size() - 2);
			}

			result += "]";
			break;
		default:
			result = "";
			break;
	};

	depth = 0;
	return result;
}

Value *Value::copy( )
{
	Value *v = NULL;

	switch ( t )
	{
		case integer_t:
			v = new Value( i );
			break;
		case double_t:
			v = new Value( d );
			break;
		case string_t:
			v = new Value( &s );
			break;
		case list_t:
			v = new Value ( &lt );
			break;
	};

	return v;
}

Value *Value::addValues( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	// If either of them is an integer then the result will be an integer
	// doubles will be  converted to ints
	if ( a->t == integer_t || this->t == integer_t ) {
		nv->t = integer_t;

		int j = ( a->t == integer_t) ? (a->i) : (a->d);
		int k = ( this->t == integer_t) ? (this->i) : (this->d);

		nv->i = j + k;
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = double_t;

		nv->d = a->d + this->d;
	} else if ( a->t == string_t && this->t == string_t ) {
		nv->t = string_t;

		nv->s.length = a->s.length + this->s.length;

		nv->s.s = (char*)malloc( nv->s.length + 1 );

		if ( nv->s.s == NULL ) {
			throw GenericException( "failed to allocate string");
		}

		memset(nv->s.s, 0, nv->s.length + 1);

		strcpy(nv->s.s, this->s.s);
		strncat(nv->s.s, a->s.s, a->s.length);

	} else if (a->t == list_t && this->t == list_t ) {
		nv->t = list_t;

		nv->lt.l = (Value**)calloc(1, sizeof(Value *) * (this->lt.max + a->lt.max) );
		nv->lt.max = this->lt.max + a->lt.max;
		nv->lt.index = 0;

		for ( int i = 0; i < this->lt.index; i++, nv->lt.index++ ) {
			nv->lt.l[nv->lt.index] = this->lt.l[i];
		}

		for ( int i = 0; i < a->lt.index; i++, nv->lt.index++ ) {
			nv->lt.l[nv->lt.index] = a->lt.l[i];
		}

	} else {
		throw InvalidOperationException("cannot add these two types");
	}

	return nv;
}

Value *Value::subValues( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	// If either of them is an integer then the result will be an integer
	// doubles will be  converted to ints
	if ( a->t == integer_t || this->t == integer_t ) {
		nv->t = integer_t;

		int j = ( a->t == integer_t) ? (a->i) : (a->d);
		int k = ( this->t == integer_t) ? (this->i) : (this->d);

		nv->i = k - j;
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = double_t;

		nv->d = this->d - a->d;
	} else {
		throw InvalidOperationException("cannot subtract these two types");
	}

	return nv;
}

Value *Value::mulValues( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	// If either of them is an integer then the result will be an integer
	// doubles will be  converted to ints
	if ( this->t == integer_t && (a->t != string_t)) {
		nv->t = integer_t;

		int j = ( a->t == integer_t) ? (a->i) : (a->d);
		int k = ( this->t == integer_t) ? (this->i) : (this->d);

		nv->i = j * k;
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = double_t;

		nv->d = a->d * this->d;
	} else if ( a->t == integer_t && this->t == string_t ) {
		nv->t = string_t;

		nv->s.length = this->s.length * a->i;

		nv->s.s = (char*)malloc(nv->s.length + 1);

		if ( nv->s.s == NULL ) {
			throw GenericException("string allocation failed");
		}

		memset(nv->s.s, 0, nv->s.length + 1);

		for ( int i = 0; i < a->i; i++ ) {
			strncat( nv->s.s, this->s.s, this->s.length);
		}

	} else {
		throw InvalidOperationException("cannot multiply these two types");
	}

	return nv;
}

Value *Value::divValues( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	// If either of them is an integer then the result will be an integer
	// doubles will be  converted to ints
	if ( a->t == double_t && this->t == double_t ) {
		nv->t = double_t;

		if ( a->d == 0.0 ) {
			throw DivByZeroException("div by zero");
		}

		nv->d = this->d / a->d;
	} else if ( ( a->t == integer_t || a->t == double_t) && (this->t == integer_t || this->t == double_t)) {
		nv->t = integer_t;

		int j = ( a->t == integer_t) ? (a->i) : (a->d);
		int k = ( this->t == integer_t) ? (this->i) : (this->d);

		if ( j == 0 ) {
			throw DivByZeroException("div by zero");
		}

		nv->i = k / j;
	} else {
		throw InvalidOperationException("cannot divide these two types");
	}

	return nv;
}

Value *Value::modValues( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	// If either of them is an integer then the result will be an integer
	// doubles will be  converted to ints
	if ( a->t == integer_t && this->t == integer_t ) {
		nv->t = integer_t;

		if ( a->i == 0 ) {
			throw DivByZeroException("div by zero");
		}

		nv->i = this->i % a->i;
	} else {
		delete nv;
		throw InvalidOperationException("cannot mod these two types");
	}

	return nv;
}

Value *Value::isEqual( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	if ( a->t == integer_t && this->t == integer_t ) {
		nv->t = integer_t;

		nv->i = (a->i == this->i);
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = integer_t;

		nv->i = (a->d == this->d);
	} else if ( a->t == string_t && this->t == string_t ) {
		nv->t = integer_t;

		nv->i = (strncmp(a->s.s, this->s.s, this->s.length) == 0);
	} else {
		throw InvalidOperationException("cannot compare these two types");
	}

	return nv;
}

Value *Value::notEqual( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	if ( a->t == integer_t && this->t == integer_t ) {
		nv->t = integer_t;

		nv->i = (a->i != this->i);
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = integer_t;

		nv->i = (a->d != this->d);
	} else if ( a->t == string_t && this->t == string_t ) {
		nv->t = integer_t;

		nv->i = (strncmp(a->s.s, this->s.s, this->s.length) != 0);
	} else {
		throw InvalidOperationException("cannot compare these two types");
	}

	return nv;
}

Value *Value::lessThan( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	if ( a->t == integer_t && this->t == integer_t ) {
		nv->t = integer_t;

		nv->i = (a->i > this->i);
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = integer_t;

		nv->i = (a->d > this->d);
	} else {
		delete nv;

		throw InvalidOperationException("cannot compare these two types");
	}

	return nv;
}

Value *Value::lessThanEqual( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	if ( a->t == integer_t && this->t == integer_t ) {
		nv->t = integer_t;

		nv->i = (a->i >= this->i);
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = integer_t;

		nv->i = (a->d >= this->d);
	} else {
		throw InvalidOperationException("cannot compare these two types");
	}

	return nv;
}

Value *Value::greaterThan( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	if ( a->t == integer_t && this->t == integer_t ) {
		nv->t = integer_t;

		nv->i = (a->i < this->i);
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = integer_t;

		nv->i = (a->d < this->d);
	} else {
		throw InvalidOperationException("cannot compare these two types");
	}

	return nv;
}

Value *Value::greaterThanEqual( Value *a )
{
	if (a == NULL ) {
		return NULL;
	}

	Value *nv = new Value( );

	if ( nv == NULL ) {
		return NULL;
	}

	if ( a->t == integer_t && this->t == integer_t ) {
		nv->t = integer_t;

		nv->i = (a->i <= this->i);
	} else if ( a->t == double_t && this->t == double_t ) {
		nv->t = integer_t;

		nv->i = (a->d <= this->d);
	} else {
		throw InvalidOperationException("cannot compare these two types");
	}

	return nv;
}

void Value::print( )
{
	std::cout << "value: ";

	switch (t) {
		case integer_t:
			std::cout << i;
			break;
		case double_t:
			std::cout << d;
			break;
		case string_t:
			std::cout << s.s;
			break;
		default:
			std::cout << "unknown";
			break;
	};

	std::cout << std::endl;

	return;
}

Value *Value::setValue( Value *a )
{
	if ( a == NULL ) {
		return NULL;
	}

	if (a->t != this->t) {
		return NULL;
	}

	int new_length = 0;

	switch(this->t) {
		case integer_t:
			this->i = a->i;
			break;
		case double_t:
			this->d = a->d;
			break;
		case string_t:
			new_length = strlen(a->s.s);

			if ( new_length < this->s.length ) {
				memcpy( this->s.s, a->s.s, new_length );

				this->s.length = new_length;
			} else {
				if ( this->s.s) {
					delete this->s.s;
				}

				this->s.s = new char[new_length + 1];

				memset( this->s.s, 0, new_length + 1);

				this->s.length = new_length;

				memcpy( this->s.s, a->s.s, new_length );
			}

			this->s.s[new_length] = 0x00;
			
			break;
		case list_t:
			lt.l = (Value**)calloc(1, sizeof(Value*) * a->lt.max );

			lt.index = a->lt.index;
			lt.max = a->lt.max;

			for ( int i = 0; i < a->lt.index; i++) {
				if ( a->lt.l[i] == NULL ) {
					lt.index = i;
					break;
				}

				lt.l[i] = a->lt.l[i];
			}

			break;
	};

	return this;
}