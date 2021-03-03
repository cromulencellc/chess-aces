#ifndef DISK_TRACKER
#define DISK_TRACKER
int add_to_disk(char *spath, char *list_type, char *list_name, char *add_field);
int edit_disk_entry(char *spath, char *list_type, char *list_name, char *entry,
                    char *change);
int delete_disk_entry(char *spath, char *list_type, char *list_name,
                      char *entry);
#endif
