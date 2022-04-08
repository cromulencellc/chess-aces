#ifndef __BUILTINS_HPP__
#define __BUILTINS_HPP__

#include "value.hpp"

// Writes the string to stdout and returns the number of bytes written
Value *puts_builtin( Value *a);

// Writes the integer to stdout and returns it
Value *puti_builtin( long long i);

// Writes the integer to stdout and returns it
Value *putd_builtin( double d);

// Writes the formated list to stdout and returns the length
Value *putl_builtin( Value *a);;

// Reads data from stdin
Value *gets_builtin( void );

// Returns the length of the string as an integer
Value *len_builtin( Value * a);

// Converts the string to an integer and returns it
Value *atoi_builtin( Value * a);

// returns a substring of the string argument
Value *substr_builtin( Value *a, int start, int length);

// Return an index into the string where the substring is found
Value *strstr_builtin( Value *a, Value *b);

// Set a character in the string to what is specified via the second argument
Value *setchr_builtin( Value *a, Value *b, int loc);

// Does not return
Value *exit_builtin( int exit_value );

// Converts an integer to a string
Value *itos_builtin( long long i );

// Converts a double to a string
Value *dtos_builtin( double i );

// Returns the type of a variable as a string
Value *type_builtin( Value *a);

// Appends a new element to the list and returns the count
Value *append_builtin( Value *l, Value *a);

// Prepends a new element to the list and returns the count
Value *prepend_builtin( Value *l, Value *a);

// pops a value from the end and returns it
Value *popend_builtin( Value *l );

// pops a value from the front and returns it
Value *popfront_builtin( Value *l );

// gets a value from the list based upon the index
Value *getbyindex_builtin( Value *l, int i );

// Returns 0 if the value exists in the list 1 if not
Value *exists_builtin( Value *l, Value *a );

// Deletes the element from the list specified by index. Returns the new length
Value *erase_builtin( Value *l, int index);

// Converts the value to a string and returns it
Value *str_builtin( Value *l );

// Parses the string into tokens and returns a list of the tokenized string
Value *strtok_builtin( Value *s, Value *t);

// Converts the characters to lower case
Value *tolower_builtin( Value *l );

// Converts the characters to upper case
Value *toupper_builtin( Value *l );

// Takes an integer and returns the hex representation as a string
Value *hex_builtin( int i);

// Takes an integer and returns the binary representation as a string
Value *bin_builtin( int i);

// Takes an integer and returns the octal representation as a string
Value *oct_builtin( int i);

// Takes a list of integers and returns the sum. An exception is thrown if there is a non-integer in the list
Value *sum_builtin( Value *l);

// Takes two integers and returns the max
Value *max_builtin( long long i, long long j);

// Takes two integers and returns the min
Value *min_builtin( long long i, long long j);

#endif