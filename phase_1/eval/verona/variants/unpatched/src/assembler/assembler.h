#ifndef parser_HEADER
#define parser_HEADER
#include "container/binary.h"
#include "container/list.h"
enum relocation_type { RELOCATION_ABSOLUTE, RELOCATION_PCREL };
enum section { SECTION_INVALID, SECTION_CODE, SECTION_DATA };
struct relocation {
  enum relocation_type relocation_type;
  uint16_t location;
  uint16_t addend;
  enum section location_section;
  char *target;
};
struct relocation *relocation_create(enum relocation_type relocation_type,
                                     uint16_t location, uint16_t addend,
                                     enum section location_section,
                                     const char *target);
struct label_location {
  char *label;
  uint16_t offset;
  enum section section;
};
struct label_location *label_location_create(const char *label, uint16_t offset,
                                             enum section section);
int parse(const struct list *tokens, struct binary **binary);
#endif