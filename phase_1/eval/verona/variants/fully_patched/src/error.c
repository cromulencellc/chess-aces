#include "error.h"
#include <stdlib.h>
#include <string.h>
char error_description[ERROR_DESCRIPTION_SIZE];
struct error_string {
  int error;
  const char *string;
};
struct error_string error_strings[] = {
    {ERROR_INSTRUCTION_DECODE, "Error decoding instruction"},
    {ERROR_INVALID_OPERANDS, "Operands were invalid for instruction type"},
    {ERROR_AA_TREE_DUPLICATE,
     "Attempted to insert duplicate entries into aa tree"},
    {ERROR_JIT_STORE_IMMEDIATE, "ERROR_JIT_STORE_IMMEDIATE"},
    {ERROR_BINARY_TOO_SMALL, "Binary is too small"},
    {ERROR_BINARY_CODE_TOO_LARGE, "Binary code section too large"},
    {ERROR_BINARY_DATA_TOO_LARGE, "Binary data section too large"},
    {ERROR_LEXER, "ERROR LEXER"},
    {ERROR_STACK_EXHAUSTED, "ERROR STACK EXHAUSTED"},
    {ERROR_OOM, "ERROR OUT OF MEMORY"},
    {ERROR_PARSER_INTERNAL, "ERROR PARSER"},
    {ERROR_PARSER_INVALID_SECTION, "ERROR INVALID SECTION"},
    {ERROR_PARSER_DUPLICATE_LABEL, "ERROR DUPLICATE LABEL"},
    {ERROR_PARSER_LABEL_RESOLVE, "Failed to resolve a label"},
    {ERROR_INVALID_ESCAPE_CHAR, "Invalid escape character"},
    {ERROR_UNTERMINATED_STRING, "Unterminated string"},
    {ERROR_STRING_TOO_LONG, "String was too long"},
    {ERROR_TOO_MANY_INSTRUCTIONS,
     "Jit tried to execute too many instructions"}};
const char *describe_error(int error) {
  unsigned int i;
  for (i = 0; i < sizeof(error_strings) / sizeof(struct error_string); i++) {
    if (error_strings[i].error == error) {
      return error_strings[i].string;
    }
  }
  return NULL;
}
void error_init() { memset(error_description, 0, ERROR_DESCRIPTION_SIZE); }