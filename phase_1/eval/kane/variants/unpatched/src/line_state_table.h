#ifndef LINE_STATE_TABLE_H
#define LINE_STATE_TABLE_H
typedef struct LineStateTable {
  int *line_offset;
  int *state;
  int entry_count;
} LineStateTable;
LineStateTable *initalize_ls_table();
void mark_for_delete(LineStateTable *lstable);
void print_lstable(LineStateTable *lstable);
void destroy_lstable(LineStateTable *lstable);
#endif
