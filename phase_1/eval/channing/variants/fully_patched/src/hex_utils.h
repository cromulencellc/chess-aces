
#ifndef hex_utils_h
#define hex_utils_h
int ishex(char x);
char fromHex(char *hexdigits);
int toHex(unsigned char value, char *output);
int chunkHexToInt(char *hexdigits);
#endif