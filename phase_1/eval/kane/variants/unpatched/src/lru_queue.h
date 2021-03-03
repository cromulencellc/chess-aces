#ifndef LRU_QUEUE
#define LRU_QUEUE
#include <stdbool.h>
#include <time.h>
typedef struct Cache_Entry {
  char *file_name;
  int time[3];
  char *contents;
} Cache_Entry;
typedef struct LRU_Queue {
  int size;
  Cache_Entry *entry;
} LRU_Queue;
LRU_Queue *create_queue();
void intialize_queue(LRU_Queue *queue);
void enqueue(LRU_Queue *queue, char *fn);
void dequeue(LRU_Queue *queue);
void move_back(LRU_Queue *queue, int index);
int search_queue(LRU_Queue *queue, char *name_of_file);
void destroy_queue(LRU_Queue *target);
void print_queue(LRU_Queue *queue);
#endif
