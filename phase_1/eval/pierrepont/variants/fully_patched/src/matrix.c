#include "matrix.h"
extern FILE *file_out;
#define ROW_BEGIN 0
#define COL_START 1
#define COL_MID 2
int count_columns(char *row) {
  int cnt = 0;
  int length = 0;
  int index = 0;
  int state = 0;
  if (row == NULL) {
    return 0;
  }
  length = strlen(row);
  if (row[index] == '[') {
    index++;
  }
  while (row[index] == ' ') {
    index++;
  }
  while (index < length) {
    if (state == ROW_BEGIN) {
      if (row[index] == ' ') {
        index++;
        continue;
      } else if (isdigit(row[index]) || row[index] == '.' ||
                 row[index] == '-') {
        state = COL_START;
        continue;
      } else {
        return cnt;
      }
    } else if (state == COL_START) {
      state = COL_MID;
      index++;
      cnt++;
      continue;
    } else if (state == COL_MID) {
      if (isdigit(row[index]) || row[index] == '.') {
        index++;
        continue;
      } else {
        state = ROW_BEGIN;
        continue;
      }
    }
  }
  return cnt;
}
int dimension_check(char *m, int *r, int *c) {
  int result = 1;
  int col_expected = 0;
  int col_expected_found = 0;
  int rows = 0;
  int cols = 0;
  int index = 0;
  char *start = NULL;
  char *end = NULL;
  char *t = NULL;
  if (!m || !r || !c) {
    return result;
  }
  if (m[index] == '[') {
    index++;
  }
  while (m[index] == ' ') {
    index++;
  }
  start = m;
  end = NULL;
  while (start) {
    end = strchr(start, ';');
    if (end) {
      t = strndup(start, end - start);
      start = end + 1;
    } else {
      t = strdup(start);
      start = end;
    }
    if (!t) {
      return result;
    }
    if (!col_expected_found) {
      col_expected = count_columns(t);
      cols = col_expected;
      col_expected_found = 1;
    } else {
      cols = count_columns(t);
    }
    free(t);
    if (cols != col_expected) {
      return result;
    }
    rows += 1;
  }
  *r = rows;
  *c = cols;
  return 0;
}
int valid_number(char *n) {
  int length = 0;
  int index = 0;
  int decimal_count = 0;
  if (!n) {
    return 0;
  }
  length = strlen(n);
  if (n[index] == '-') {
    index++;
  }
  for (; index < length; index++) {
    if (isdigit(n[index])) {
      index++;
    } else if (n[index] == '.') {
      decimal_count++;
      index++;
    } else {
      return 0;
    }
  }
  if (decimal_count > 1) {
    return 0;
  }
  return 1;
}
void free_matrix(matrix *A) {
  if (!A) {
    return;
  }
  if (A->values) {
    free(A->values);
  }
  free(A);
  return;
}
matrix *allocate_matrix(int rows, int cols) {
  matrix *A = NULL;
  long double *v = NULL;
  if (!rows || !cols) {
    return NULL;
  }
  v = calloc(1, sizeof(long double) * (rows * cols));
  if (!v) {
    return NULL;
  }
  A = calloc(1, sizeof(matrix));
  if (!A) {
    free(v);
    return NULL;
  }
  A->rows = rows;
  A->columns = cols;
  A->values = v;
  return A;
}
#define LINE_START 0
#define VALUE_START 1
#define EAT_SPACE 2
#define END_LINE 3
#define ENDV 4
#define ENDM 5
#define ERROR -1
int parse_values(char *m, long double *values, int r, int c) {
  int state = LINE_START;
  int length = 0;
  int index = 0;
  int valindex = 0;
  char *valstart = NULL;
  char *valend = NULL;
  char *temp = NULL;
  if (!m) {
    return 0;
  }
  if (m[index++] != '[') {
    return 0;
  }
  length = strlen(m);
  while (index < length) {
    switch (state) {
    case LINE_START:
      if (m[index] == ' ' || m[index] == ';') {
        index++;
      } else if (isdigit(m[index]) || m[index] == '.' || m[index] == '-') {
        valstart = m + index;
        state = VALUE_START;
      } else if (m[index] == ']') {
        state = ENDM;
      } else {
        state = ERROR;
      }
      break;
    case VALUE_START:
      if (isdigit(m[index]) || m[index] == '.' || m[index] == '-') {
        index++;
      } else if (m[index] == ' ' | m[index] == ';' || m[index] == ']') {
        state = ENDV;
      } else {
        state = ERROR;
      }
      break;
    case ENDV:
      valend = m + index;
      temp = strndup(valstart, valend - valstart);
      if (!temp) {
        return 0;
      }
      if (!valid_number(temp)) {
        return 0;
      }
      values[valindex++] = strtold(temp, NULL);
      free(temp);
      valstart = NULL;
      valend = NULL;
      if (m[index] == ' ' || m[index] == ';') {
        state = LINE_START;
      } else if (m[index] == ']') {
        state = ENDM;
      } else {
        state = ERROR;
      }
      break;
    case ENDM:
      return 1;
    case ERROR:
      fprintf(file_out, "[ERROR] Some error in parse values\n");
      return 0;
    default:
      return 0;
    }
  }
  fprintf(file_out, "[ERROR] made it through without ending\n");
  return 0;
}
int is_scalar(matrix *A) {
  if (!A) {
    return 0;
  }
  if (A->rows == 1 && A->columns == 1) {
    return 1;
  }
  return 0;
}
int max_int(int x, int y) { return x > y ? x : y; }
long double get_matrix_element(matrix *A, int row, int column) {
  long double element = 0.0;
  if (!A) {
    return element;
  }
  if (A->rows <= row || A->columns <= column) {
    return element;
  }
  element = A->values[(row * A->columns) + column];
  return element;
}
void set_matrix_element(matrix *A, int row, int column, long double value) {
  if (!A) {
    return;
  }
  if (A->rows < row || A->columns < column) {
    return;
  }
  A->values[(row * A->columns) + column] = value;
  return;
}
void print_matrix(matrix *A) {
  long double value = 0.0;
  if (!A) {
    fprintf(file_out, "[ERROR] print_matrix NULL\n");
    return;
  }
  for (int i = 0; i < A->rows; i++) {
    fprintf(file_out, "\n\t");
    for (int j = 0; j < A->columns; j++) {
      value = get_matrix_element(A, i, j);
      if (value == 0.0) {
        value = 0.0;
        set_matrix_element(A, i, j, value);
      }
      if (isnan(value)) {
        value = NAN;
        set_matrix_element(A, i, j, value);
      }
      if (isinf(value)) {
        value = INFINITY;
        set_matrix_element(A, i, j, value);
      }
      fprintf(file_out, "%.3Lf ", value);
    }
  }
  fprintf(file_out, "\n\n");
}
matrix *matrix_transpose(matrix *A) {
  matrix *B = NULL;
  if (!A) {
    return NULL;
  }
  B = allocate_matrix(A->columns, A->rows);
  if (!B) {
    free_matrix(A);
    return NULL;
  }
  for (int i = 0; i < A->columns; i++) {
    for (int j = 0; j < A->rows; j++) {
      set_matrix_element(B, i, j, get_matrix_element(A, j, i));
    }
  }
  free_matrix(A);
  return B;
}
matrix *exp_matrix(matrix *A, matrix *B) {
  matrix *C = NULL;
  if (!A || !B) {
    if (A) {
      free_matrix(A);
    }
    if (B) {
      free_matrix(B);
    }
    return NULL;
  }
  if (!is_scalar(B)) {
    free_matrix(B);
    free_matrix(A);
    fprintf(file_out, "error -- invalid argument\n");
    return NULL;
  }
  C = allocate_matrix(A->rows, A->columns);
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return C;
  }
  for (int i = 0; i < A->rows; i++) {
    for (int j = 0; j < A->columns; j++) {
      set_matrix_element(C, i, j, powl(get_matrix_element(A, i, j),
                                       get_matrix_element(B, 0, 0)));
    }
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *exp_matrix_elw(matrix *A, matrix *B) {
  matrix *C = NULL;
  if (!A || !B) {
    if (A) {
      free_matrix(A);
    }
    if (B) {
      free_matrix(B);
    }
    return NULL;
  }
  if (is_scalar(B)) {
    C = exp_matrix(A, B);
    return C;
  }
  C = allocate_matrix(max_int(B->rows, A->rows),
                      max_int(B->columns, A->columns));
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return C;
  }
  if (A->columns == B->columns && A->rows == B->rows) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, powl(get_matrix_element(A, i, j),
                                         get_matrix_element(B, i, j)));
      }
    }
  } else if (A->columns == B->columns && A->rows == 1) {
    for (int i = 0; i < B->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, powl(get_matrix_element(A, 0, j),
                                         get_matrix_element(B, i, j)));
      }
    }
  } else if (A->rows == B->rows && A->columns == 1) {
    for (int i = 0; i < B->columns; i++) {
      for (int j = 0; j < A->rows; j++) {
        set_matrix_element(C, j, i, powl(get_matrix_element(A, j, 0),
                                         get_matrix_element(B, j, i)));
      }
    }
  } else if (A->columns == B->columns && B->rows == 1) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < B->columns; j++) {
        set_matrix_element(C, i, j, powl(get_matrix_element(A, i, j),
                                         get_matrix_element(B, 0, j)));
      }
    }
  } else if (A->rows == B->rows && B->columns == 1) {
    for (int i = 0; i < A->columns; i++) {
      for (int j = 0; j < B->rows; j++) {
        set_matrix_element(C, j, i, powl(get_matrix_element(A, j, i),
                                         get_matrix_element(B, j, 0)));
      }
    }
  } else {
    fprintf(file_out, "error: operator .^: nonconformant arguments (op1 is "
                      "%llux%llu, op2 is %llux%llu)\n",
            A->rows, A->columns, B->rows, B->columns);
    free_matrix(A);
    free_matrix(B);
    free_matrix(C);
    return NULL;
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *scalar_multiply_matrix(matrix *A, matrix *B) {
  matrix *scalar = NULL;
  matrix *m = NULL;
  matrix *C = NULL;
  if (!A || !B) {
    return NULL;
  }
  if (is_scalar(A)) {
    scalar = A;
    m = B;
  } else if (is_scalar(B)) {
    scalar = B;
    m = A;
  } else {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  C = allocate_matrix(m->rows, m->columns);
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return C;
  }
  for (int i = 0; i < m->rows; i++) {
    for (int j = 0; j < m->columns; j++) {
      set_matrix_element(C, i, j, get_matrix_element(scalar, 0, 0) *
                                      get_matrix_element(m, i, j));
    }
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *multiply_matrices(matrix *A, matrix *B) {
  matrix *C = NULL;
  if (!A || !B) {
    return C;
  }
  if (is_scalar(A) || is_scalar(B)) {
    return scalar_multiply_matrix(A, B);
  }
  if (A->columns != B->rows) {
    fprintf(file_out, "error: operator *: nonconformant arguments (op1 is "
                      "%llux%llu, op2 is %llux%llu)\n",
            A->rows, A->columns, B->rows, B->columns);
    free_matrix(A);
    free_matrix(B);
    return C;
  }
  C = allocate_matrix(A->rows, B->columns);
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return C;
  }
  for (int i = 0; i < A->rows; i++) {
    for (int j = 0; j < B->columns; j++) {
      long double sum = 0.0;
      for (int k = 0; k < A->columns; k++) {
        sum += get_matrix_element(A, i, k) * get_matrix_element(B, k, j);
      }
      set_matrix_element(C, i, j, sum);
    }
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *parse_matrix(char *m) {
  matrix *tm = NULL;
  int row, col;
  if (!m) {
    return NULL;
  }
  if (dimension_check(m, &row, &col)) {
    fprintf(file_out, "invalid matrix\n");
    return NULL;
  }
  tm = allocate_matrix(row, col);
  if (!tm) {
    return NULL;
  }
  if (!parse_values(m, tm->values, row, col)) {
    fprintf(file_out, "[ERROR] invalid matrix\n");
    free_matrix(tm);
    return NULL;
  }
  return tm;
}
matrix *scalar_to_matrix(long double scalar) {
  matrix *A = allocate_matrix(1, 1);
  if (!A) {
    return NULL;
  }
  A->values[0] = scalar;
  return A;
}
matrix *scalar_add_matrix(matrix *A, matrix *B) {
  matrix *scalar = NULL;
  matrix *m = NULL;
  matrix *C = NULL;
  if (!A || !B) {
    return NULL;
  }
  if (is_scalar(A)) {
    scalar = A;
    m = B;
  } else if (is_scalar(B)) {
    scalar = B;
    m = A;
  } else {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  C = allocate_matrix(m->rows, m->columns);
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return C;
  }
  for (int i = 0; i < m->rows; i++) {
    for (int j = 0; j < m->columns; j++) {
      set_matrix_element(C, i, j, get_matrix_element(scalar, 0, 0) +
                                      get_matrix_element(m, i, j));
    }
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *add_matrices(matrix *A, matrix *B) {
  matrix *C = NULL;
  if (!A || !B) {
    return NULL;
  }
  if (is_scalar(A) || is_scalar(B)) {
    return scalar_add_matrix(A, B);
  }
  C = allocate_matrix(max_int(B->rows, A->rows),
                      max_int(B->columns, A->columns));
  if (!C) {
    return NULL;
  }
  if (A->columns == B->columns && A->rows == B->rows) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) +
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (A->columns == B->columns && A->rows == 1) {
    for (int i = 0; i < B->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, 0, j) +
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (A->rows == B->rows && A->columns == 1) {
    for (int i = 0; i < B->columns; i++) {
      for (int j = 0; j < A->rows; j++) {
        set_matrix_element(C, j, i, get_matrix_element(A, j, 0) +
                                        get_matrix_element(B, j, i));
      }
    }
  } else if (A->columns == B->columns && B->rows == 1) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < B->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) +
                                        get_matrix_element(B, 0, j));
      }
    }
  } else if (A->rows == B->rows && B->columns == 1) {
    for (int i = 0; i < A->columns; i++) {
      for (int j = 0; j < B->rows; j++) {
        set_matrix_element(C, j, i, get_matrix_element(A, j, i) +
                                        get_matrix_element(B, j, 0));
      }
    }
  } else {
    fprintf(file_out, "error: operator +: nonconformant arguments (op1 is "
                      "%llux%llu, op2 is %llux%llu)",
            A->rows, A->columns, B->rows, B->columns);
    free_matrix(A);
    free_matrix(B);
    free_matrix(C);
    return NULL;
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *scalar_sub_matrix(matrix *A, matrix *B) {
  matrix *C = NULL;
  if (!A || !B) {
    return NULL;
  }
  if (is_scalar(A)) {
    C = allocate_matrix(B->rows, B->columns);
    if (!C) {
      free_matrix(A);
      free_matrix(B);
      return NULL;
    }
    for (int i = 0; i < B->rows; i++) {
      for (int j = 0; j < B->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, 0, 0) -
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (is_scalar(B)) {
    C = allocate_matrix(A->rows, A->columns);
    if (!C) {
      free_matrix(A);
      free_matrix(B);
      return NULL;
    }
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) -
                                        get_matrix_element(B, 0, 0));
      }
    }
  } else {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *sub_matrices(matrix *A, matrix *B) {
  matrix *C = NULL;
  if (!A || !B) {
    return NULL;
  }
  if (is_scalar(A) || is_scalar(B)) {
    return scalar_sub_matrix(A, B);
  }
  C = allocate_matrix(max_int(B->rows, A->rows),
                      max_int(B->columns, A->columns));
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  if (A->columns == B->columns && A->rows == B->rows) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) -
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (A->columns == B->columns && A->rows == 1) {
    for (int i = 0; i < B->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, 0, j) -
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (A->rows == B->rows && A->columns == 1) {
    for (int i = 0; i < B->columns; i++) {
      for (int j = 0; j < A->rows; j++) {
        set_matrix_element(C, j, i, get_matrix_element(A, j, 0) -
                                        get_matrix_element(B, j, i));
      }
    }
  } else if (A->columns == B->columns && B->rows == 1) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < B->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) -
                                        get_matrix_element(B, 0, j));
      }
    }
  } else if (A->rows == B->rows && B->columns == 1) {
    for (int i = 0; i < A->columns; i++) {
      for (int j = 0; j < B->rows; j++) {
        set_matrix_element(C, j, i, get_matrix_element(A, j, i) -
                                        get_matrix_element(B, j, 0));
      }
    }
  } else {
    fprintf(file_out, "error: operator -: nonconformant arguments (op1 is "
                      "%llux%llu, op2 is %llux%llu)",
            A->rows, A->columns, B->rows, B->columns);
    free_matrix(A);
    free_matrix(B);
    free_matrix(C);
    return NULL;
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *dot_multiply_matrices(matrix *A, matrix *B) {
  matrix *C = NULL;
  if (!A || !B) {
    return NULL;
  }
  C = allocate_matrix(max_int(B->rows, A->rows),
                      max_int(B->columns, A->columns));
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  if (A->columns == B->columns && A->rows == B->rows) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) *
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (A->columns == B->columns && A->rows == 1) {
    for (int i = 0; i < B->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, 0, j) *
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (A->rows == B->rows && A->columns == 1) {
    for (int i = 0; i < B->columns; i++) {
      for (int j = 0; j < A->rows; j++) {
        set_matrix_element(C, j, i, get_matrix_element(A, j, 0) *
                                        get_matrix_element(B, j, i));
      }
    }
  } else if (A->columns == B->columns && B->rows == 1) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < B->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) *
                                        get_matrix_element(B, 0, j));
      }
    }
  } else if (A->rows == B->rows && B->columns == 1) {
    for (int i = 0; i < A->columns; i++) {
      for (int j = 0; j < B->rows; j++) {
        set_matrix_element(C, j, i, get_matrix_element(A, j, i) *
                                        get_matrix_element(B, j, 0));
      }
    }
  } else {
    fprintf(file_out, "error: operator .*: nonconformant arguments (op1 is "
                      "%llux%llu, op2 is %llux%llu)\n",
            A->rows, A->columns, B->rows, B->columns);
    free_matrix(A);
    free_matrix(B);
    free_matrix(C);
    return NULL;
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *dot_divide_matrices(matrix *A, matrix *B) {
  matrix *C = NULL;
  if (!A || !B) {
    if (A) {
      free_matrix(A);
    }
    if (B) {
      free_matrix(B);
    }
    return NULL;
  }
  C = allocate_matrix(max_int(B->rows, A->rows),
                      max_int(B->columns, A->columns));
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  if (A->columns == B->columns && A->rows == B->rows) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        if (!get_matrix_element(B, i, j)) {
          free_matrix(A);
          free_matrix(B);
          free_matrix(C);
          return NULL;
        }
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) /
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (A->columns == B->columns && A->rows == 1) {
    for (int i = 0; i < B->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        if (!get_matrix_element(B, i, j)) {
          free_matrix(A);
          free_matrix(B);
          free_matrix(C);
          return NULL;
        }
        set_matrix_element(C, i, j, get_matrix_element(A, 0, j) /
                                        get_matrix_element(B, i, j));
      }
    }
  } else if (A->rows == B->rows && A->columns == 1) {
    for (int i = 0; i < B->columns; i++) {
      for (int j = 0; j < A->rows; j++) {
        if (!get_matrix_element(B, j, i)) {
          fprintf(file_out, "[ERROR] divide by zero\n");
          free_matrix(A);
          free_matrix(B);
          free_matrix(C);
          return NULL;
        }
        set_matrix_element(C, j, i, get_matrix_element(A, j, 0) /
                                        get_matrix_element(B, j, i));
      }
    }
  } else if (A->columns == B->columns && B->rows == 1) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < B->columns; j++) {
        if (!get_matrix_element(B, 0, j)) {
          fprintf(file_out, "[ERROR] divide by zero\n");
          free_matrix(A);
          free_matrix(B);
          free_matrix(C);
          return NULL;
        }
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) /
                                        get_matrix_element(B, 0, j));
      }
    }
  } else if (A->rows == B->rows && B->columns == 1) {
    for (int i = 0; i < A->columns; i++) {
      for (int j = 0; j < B->rows; j++) {
        if (!get_matrix_element(B, j, 0)) {
          fprintf(file_out, "[ERROR] divide by zero\n");
          free_matrix(A);
          free_matrix(B);
          free_matrix(C);
          return NULL;
        }
        set_matrix_element(C, j, i, get_matrix_element(A, j, i) /
                                        get_matrix_element(B, j, 0));
      }
    }
  } else {
    fprintf(file_out, "error: operator ./: nonconformant arguments (op1 is "
                      "%llux%llu, op2 is %llux%llu)",
            A->rows, A->columns, B->rows, B->columns);
    free_matrix(A);
    free_matrix(B);
    free_matrix(C);
    return NULL;
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
