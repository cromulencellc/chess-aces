#include "builtins.hpp"

extern FILE *file_out;
extern FILE *file_in;

Value *puts_builtin( Value *a )
{
	Value *v = new Value( (long long)strlen(a->s.s) );

	fprintf(file_out, "%s\n", a->s.s);
	fflush(file_out);

	return v;
}

Value *puti_builtin( long long i )
{
	Value *v = new Value( i );

	fprintf(file_out, "%lld\n", i);
	fflush(file_out);

	return v;
}

Value *putd_builtin( double d)
{
	Value *v = new Value( d );

	fprintf(file_out, "%.6f\n", d);
	fflush(file_out);

	return v;
}

Value *putl_builtin( Value *a )
{
	if ( a->t != list_t ) {
		throw GenericException("type is not a list");
	}

	std::string formatted_list = a->toString();

	fprintf(file_out, "%s\n", formatted_list.c_str());
	fflush(file_out);

	Value *v = new Value( (long long)formatted_list.size() );

	return v;
}

Value *gets_builtin( void )
{
	char data[128];

	std::string s = "";

	fgets( data, 128, file_in );
	s = std::string( data );

	Value *v = new Value( s );

	return v;
}

Value *len_builtin( Value *a )
{
	Value *v = NULL;

	switch ( a->t ) {
		case integer_t:
			v = new Value( (long long)sizeof(a->i));
			break;
		case double_t:
			v = new Value( (long long)sizeof(a->d));
			break;
		case string_t:
			v = new Value ( (long long)a->s.length);
			break;
		case list_t:
			v = new Value( (long long)a->lt.index);
			break; 
	}

	return v;

}

Value *atoi_builtin( Value *a)
{
	Value *v = new Value( strtoll( a->s.s, NULL, 10 ) );

	return v;
}

Value *substr_builtin( Value *a, int start, int length)
{
	string temps;

	if (a->s.length <= start ) {
		throw GenericException( "string start out of bounds");
	}

	if ( length < 0 ) {
		throw GenericException("negative lengths not allowed");
	}

	char * c = new char[length + 1];

	if ( c == NULL ) {
		throw GenericException("failed to create substring");
	}

	temps.s = c;
	temps.length = length;

	if ( length > a->s.length - start ) {
		temps.length = a->s.length - start;
	}

	memcpy( c, a->s.s + start, temps.length);

	Value *v = new Value( &temps );

	return v;
}

Value *strstr_builtin( Value *a, Value *b)
{
	char *c = strstr( a->s.s, b->s.s);
	Value *v = NULL;

	if ( c == NULL ) {
		v = new Value( (long long)-1 );
		return v;
	}

	for ( int i = 0; i <= (strlen(a->s.s) - strlen(b->s.s)); i++ ) {
		if ( strncmp(a->s.s + i, b->s.s, strlen(b->s.s)) == 0 ) {
			v = new Value( (long long)i );
			return v;
		}
	}

	v = new Value( (long long)-1 );

	return v;
}

Value *setchr_builtin( Value *a, Value *b, int loc)
{
	if ( a->s.length <= loc ) {
		throw GenericException( "location out of bounds");
	}

	if ( b->s.length != 1 ) {
		throw GenericException( "invalid character argument");
	}

	a->s.s[loc] = b->s.s[0];

	return NULL;
}

Value *exit_builtin( int exit_value )
{
	fclose(file_out);
	fclose(file_in);

	exit(exit_value);

	return NULL;
}

Value *itos_builtin( long long i )
{
	Value *v = new Value( std::to_string(i) );

	return v;
}

Value *dtos_builtin( double i )
{
	Value *v = new Value( std::to_string(i) );

	return v;
}

Value *type_builtin( Value * a)
{
	return new Value(a->typeString());
}

Value *append_builtin( Value *l, Value *a)
{
	if (l->lt.l == NULL ) {
		l->lt.l = (Value**)calloc(1, sizeof(Value *) * 10);
		l->lt.max = 10;
		l->lt.index = 0;
	} else if ( l->lt.max == l->lt.index ) {
		l->lt.l = (Value**)realloc( l->lt.l, (l->lt.max * 2) * sizeof(Value *) );
		l->lt.max *= 2;
	}

	l->lt.l[l->lt.index++] = a->copy();

	Value *v = new Value( (long long)l->lt.index );

	return v;
}

Value *prepend_builtin( Value *l, Value *a)
{
	if (l->lt.l == NULL ) {
		l->lt.l = (Value**)calloc(1, sizeof(Value *) * 10);
		l->lt.max = 10;
		l->lt.index = 0;
	} else if ( l->lt.max == l->lt.index ) {
		l->lt.l = (Value**)realloc( l->lt.l, l->lt.max * 2 );
		l->lt.max *= 2;
	}

	memcpy( l->lt.l + 1, l->lt.l, l->lt.index * sizeof(Value *));

	l->lt.l[0] = a->copy();
	l->lt.index++;

	return new Value( (long long)l->lt.index );
}

Value *popend_builtin( Value *l )
{
	Value *v = NULL;

	if (l->lt.index == 0 ) {
		throw GenericException("list is empty");
	}

	v = l->lt.l[l->lt.index-1];
	l->lt.l[l->lt.index-1] = NULL;
	l->lt.index--;

	return v;
}

Value *popfront_builtin( Value *l )
{
	Value *v = NULL;

	if (l->lt.index == 0 ) {
		throw GenericException("list is empty");
	}

	v = l->lt.l[0];
	l->lt.index--;

	memcpy( l->lt.l, l->lt.l + 1, l->lt.index * sizeof(Value *));

	return v;
}

Value *getbyindex_builtin( Value *l, int i )
{
	if ( (l->lt.index - 1) < i || i < 0) {
		throw GenericException("index beyond bounds of list");
	}

	return l->lt.l[i]->copy();
}

Value *exists_builtin( Value *l, Value *a )
{
	for ( int i = 0; i < l->lt.index; i++ ) {
		if ( l->lt.l[i]->t != a->t ) {
			continue;
		}

		switch ( l->lt.l[i]->t ) {
			case integer_t:
				if ( l->lt.l[i]->i == a->i ) {
					return new Value( (long long)1);
				}
				break;
			case double_t:
				if ( l->lt.l[i]->d == a->d ) {
					return new Value( (long long)1);
				}
				break;
			case string_t:
				if ( strncmp(l->lt.l[i]->s.s, a->s.s, a->s.length) == 0 ) {
					return new Value( (long long) 1);
				}
				break;
			case list_t:
				throw GenericException("not checking lists");
				break;
		};
	}

	return new Value( (long long) 0);
}

Value *erase_builtin( Value *l, int index)
{
	if ( l->lt.index == 0) {
		throw GenericException("list is empty");
	}

	if ( (l->lt.index - 1) < index || index < 0) {
		throw GenericException("index out of bounds");
	}

	delete l->lt.l[index];

	l->lt.index--;

	memcpy( l->lt.l + index, l->lt.l + index + 1, (l->lt.index - (index)) * sizeof(Value *));

	return NULL;

}

Value *str_builtin( Value *l )
{
	return new Value( l->toString() );
}

Value *strtok_builtin( Value *s, Value *t)
{
	Value *v = new Value( list_t );

	char *result = NULL;
	char *saveptr = NULL;

	char *temp = strdup(s->s.s);

	for ( result = temp; ; result = NULL ) {
		result = strtok_r( result, t->s.s, &saveptr);

		if ( result ) {
			append_builtin(v, new Value(result) );
		} else {
			break;
		}
	}

	free(t);

	return v;
}

Value *tolower_builtin( Value *l )
{
	for ( int i = 0; i < l->s.length; i++) {
		l->s.s[i] = tolower( l->s.s[i] );
	}

	return NULL;
}

Value *toupper_builtin( Value *l )
{
	for ( int i = 0; i < l->s.length; i++) {
		l->s.s[i] = toupper( l->s.s[i] );
	}

	return NULL;
}

Value *hex_builtin( int i)
{
	std::string result = "";
	int f = 0;

	if ( i == 0 ) {
		return new Value(std::string("0x0"));
	}

	int neg = 0;

	if (i < 0) {
		i *= -1;
		neg = 1;
	}

	char c = 0;

	while ( i ) {
		f = i % 16;

		i = i / 16;

		switch (f){
			case 0:
				c = '0';
				break;
			case 1:
				c = '1';
				break;
			case 2:
				c = '2';
				break;
			case 3:
				c = '3';
				break;
			case 4:
				c = '4';
				break;
			case 5:
				c = '5';
				break;
			case 6:
				c = '6';
				break;
			case 7:
				c = '7';
				break;
			case 8:
				c = '8';
				break;
			case 9:
				c = '9';
				break;
			case 10:
				c = 'a';
				break;
			case 11:
				c = 'b';
				break;
			case 12:
				c = 'c';
				break;
			case 13:
				c = 'd';
				break;
			case 14:
				c = 'e';
				break;
			case 15:
				c = 'f';
				break;
		};

		result = c + result;
	}

	result = "0x" + result;

	if (neg) {
		result = "-" + result;
	}

	return new Value(result);
}

Value *bin_builtin( int i)
{
	std::string result = "";
	int f = 0;

	if ( i == 0 ) {
		return new Value(std::string("0b00"));
	}

	int neg = 0;

	if (i < 0) {
		i *= -1;
		neg = 1;
	}

	char c = 0;

	while ( i ) {
		f = i % 2;

		i = i / 2;

		switch (f){
			case 0:
				c = '0';
				break;
			case 1:
				c = '1';
				break;
		};

		result = c + result;
	}

	result = "0b" + result;

	if (neg) {
		result = "-" + result;
	}

	return new Value(result);
}

Value *oct_builtin( int i)
{
	std::string result = "";
	int f = 0;

	if ( i == 0 ) {
		return new Value(std::string("0o00"));
	}

	int neg = 0;

	if (i < 0) {
		i *= -1;
		neg = 1;
	}

	char c = 0;

	while ( i ) {
		f = i % 8;

		i = i / 8;

		switch (f){
			case 0:
				c = '0';
				break;
			case 1:
				c = '1';
				break;
			case 2:
				c = '2';
				break;
			case 3:
				c = '3';
				break;
			case 4:
				c = '4';
				break;
			case 5:
				c = '5';
				break;
			case 6:
				c = '6';
				break;
			case 7:
				c = '7';
				break;
			case 8:
				c = '8';
				break;
		};

		result = c + result;
	}

	result = "0o" + result;

	if (neg) {
		result = "-" + result;
	}

	return new Value(result);
}

Value *sum_builtin( Value *l)
{
	long long result = 0;

	for ( int i = 0; i < l->lt.index; i++ ) {
		if ( l->lt.l[i]->t != integer_t ) {
			throw GenericException("encountered a non-int type");
		}

		result += l->lt.l[i]->i;
	}

	return new Value(result);
}

Value *max_builtin( long long i, long long j)
{
	long long result = 0;

	if ( i > j ) {
		result = i;
	} else {
		result = j;
	}

	return new Value( result );
}

Value *min_builtin( long long i, long long j)
{
	long long result = 0;

	if ( i < j ) {
		result = i;
	} else {
		result = j;
	}

	return new Value( result );
}
