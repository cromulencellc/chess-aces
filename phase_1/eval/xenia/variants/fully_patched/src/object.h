#ifndef object_HEADER
#define object_HEADER
typedef void (*object_delete_f)(void *);
typedef void *(*object_copy_f)(const void *);
typedef int (*object_cmp_f)(const void *, const void *);
struct object {
  object_delete_f delete;
  object_copy_f copy;
  object_cmp_f cmp;
};
void *object_not_copyable(const void *);
int object_not_comparable(const void *, const void *);
int object_is(const void *object, const struct object *object_type);
const char *object_type_debug(const struct object *type);
const char *object_debug(const void *object);
#define object_type_verify(XXX, YYY)                                           \
  do {                                                                         \
    if (*((struct object **)XXX) != YYY) {                                     \
      object_type_verify_(XXX, YYY, __FILE__, __FUNCTION__, __LINE__);         \
    }                                                                          \
  } while (0);
void object_type_verify_(const void *object, const struct object *object_type,
                         const char *file, const char *function,
                         unsigned int line);
void object_delete(void *);
void *object_copy(const void *);
int object_cmp(const void *, const void *);
#endif