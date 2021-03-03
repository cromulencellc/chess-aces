#ifndef __MATRIX_FUNCS__
#define __MATRIX_FUNCS__
#include <malloc.h>
#include <math.h>
#include "matrix.h"
matrix *handle_func_single_arg(char *name, matrix *m);
matrix *handle_func_two_arg(char *name, matrix *A, matrix *B);
matrix *magnitude(matrix *m);
#endif