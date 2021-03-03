#ifndef __MATRIX__
#define __MATRIX__
#include <ctype.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
typedef struct matrix {
  unsigned long long rows;
  unsigned long long columns;
  long double *values;
} matrix;
matrix *allocate_matrix(int rows, int cols);
matrix *parse_matrix(char *m);
void free_matrix(matrix *A);
matrix *scalar_to_matrix(long double scalar);
long double get_matrix_element(matrix *A, int row, int column);
void set_matrix_element(matrix *A, int row, int column, long double value);
matrix *add_matrices(matrix *A, matrix *B);
matrix *sub_matrices(matrix *A, matrix *B);
matrix *multiply_matrices(matrix *A, matrix *B);
matrix *dot_multiply_matrices(matrix *A, matrix *B);
matrix *dot_divide_matrices(matrix *A, matrix *B);
matrix *matrix_transpose(matrix *A);
void print_matrix(matrix *A);
int is_scalar(matrix *A);
matrix *exp_matrix(matrix *A, matrix *B);
matrix *exp_matrix_elw(matrix *A, matrix *B);
int max_int(int x, int y);
#endif