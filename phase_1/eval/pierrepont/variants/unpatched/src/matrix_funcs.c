#include "matrix_funcs.h"
extern FILE *file_out;
matrix *sigmoid(matrix *A) {
  matrix *B = NULL;
  long double e = 2.7182818284;
  if (!A) {
    return NULL;
  }
  B = allocate_matrix(A->rows, A->columns);
  if (!B) {
    free_matrix(A);
    return B;
  }
  for (int i = 0; i < B->rows; i++) {
    for (int j = 0; j < B->columns; j++) {
      set_matrix_element(B, i, j,
                         1 / (1 + powl(e, get_matrix_element(A, i, j) * -1)));
    }
  }
  free_matrix(A);
  return B;
}
matrix *log_base_two(matrix *A) {
  matrix *B = NULL;
  if (!A) {
    return NULL;
  }
  B = allocate_matrix(A->rows, A->columns);
  if (!B) {
    free_matrix(A);
    return B;
  }
  for (int i = 0; i < B->rows * B->columns; i++) {
    B->values[i] = log2l(A->values[i]);
  }
  free_matrix(A);
  return B;
}
matrix *log_base_ten(matrix *A) {
  matrix *B = NULL;
  if (!A) {
    return NULL;
  }
  B = allocate_matrix(A->rows, A->columns);
  if (!B) {
    free_matrix(A);
    return B;
  }
  for (int i = 0; i < B->rows * B->columns; i++) {
    B->values[i] = log10l(A->values[i]);
  }
  free_matrix(A);
  return B;
}
matrix *sqrtm(matrix *A) {
  matrix *B = NULL;
  if (!A) {
    return NULL;
  }
  B = allocate_matrix(A->rows, A->columns);
  if (!B) {
    free_matrix(A);
    return B;
  }
  for (int i = 0; i < B->rows * B->columns; i++) {
    B->values[i] = sqrtl(A->values[i]);
  }
  free_matrix(A);
  return B;
}
matrix *get_columns(matrix *A) {
  int r = 0;
  if (!A) {
    return NULL;
  }
  r = A->columns;
  free_matrix(A);
  return scalar_to_matrix(r);
}
matrix *get_rows(matrix *A) {
  int r = 0;
  if (!A) {
    return NULL;
  }
  r = A->rows;
  free_matrix(A);
  return scalar_to_matrix(r);
}
matrix *get_identity(matrix *A) {
  matrix *identity = NULL;
  if (!A) {
    return NULL;
  }
  if (A->rows != 1 || A->columns != 1) {
    fprintf(file_out, "error -- invalid argument\n");
    free_matrix(A);
    return NULL;
  }
  identity = allocate_matrix(A->values[0], A->values[0]);
  free_matrix(A);
  if (!identity) {
    return NULL;
  }
  for (int i = 0; i < identity->rows; i++) {
    set_matrix_element(identity, i, i, 1);
  }
  return identity;
}
matrix *magnitude(matrix *A) {
  matrix *mag = NULL;
  long double t = 0.0;
  if (!A) {
    return NULL;
  }
  if (A->columns != 1) {
    fprintf(file_out, "error -- invalid argument\n");
    free_matrix(A);
    return NULL;
  }
  mag = allocate_matrix(1, 1);
  if (!mag) {
    free_matrix(A);
    return NULL;
  }
  for (int i = 0; i < A->rows; i++) {
    t += powl(A->values[i], 2);
  }
  mag->values[0] = sqrtl(t);
  free_matrix(A);
  return mag;
}
matrix *zeros(matrix *A, matrix *B) {
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
  if (!is_scalar(A) || !is_scalar(B)) {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  C = allocate_matrix(get_matrix_element(A, 0, 0), get_matrix_element(B, 0, 0));
  free_matrix(A);
  free_matrix(B);
  if (!C) {
    return NULL;
  }
  return C;
}
matrix *ones(matrix *A, matrix *B) {
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
  if (!is_scalar(A) || !is_scalar(B)) {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  C = allocate_matrix(get_matrix_element(A, 0, 0), get_matrix_element(B, 0, 0));
  free_matrix(A);
  free_matrix(B);
  if (!C) {
    return NULL;
  }
  for (int i = 0; i < C->rows * C->columns; i++) {
    C->values[i] = 1.0;
  }
  return C;
}
matrix *add_func(matrix *A, matrix *B) {
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
    return NULL;
  }
  if (A->columns == B->columns && A->rows == B->rows) {
    for (int i = 0; i < A->rows; i++) {
      for (int j = 0; j < A->columns; j++) {
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) +
                                        get_matrix_element(B, i, j));
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
matrix *sub_func(matrix *A, matrix *B) {
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
        set_matrix_element(C, i, j, get_matrix_element(A, i, j) -
                                        get_matrix_element(B, i, j));
      }
    }
  } else {
    fprintf(file_out, "error: %%sum: nonconformant arguments (op1 is "
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
matrix *reshape(matrix *A, matrix *B) {
  matrix *C = NULL;
  int Arow = 0;
  int Acol = 0;
  int Crow = 0;
  int Ccol = 0;
  if (!A || !B) {
    if (A) {
      free_matrix(A);
    }
    if (B) {
      free_matrix(B);
    }
    return NULL;
  }
  if (B->rows != 1 || B->columns != 2) {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  Crow = get_matrix_element(B, 0, 0);
  Ccol = get_matrix_element(B, 0, 1);
  if (Crow <= 0 || Ccol <= 0 || (Crow * Ccol) > (A->rows * A->columns)) {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  C = allocate_matrix(Crow, Ccol);
  if (!C) {
    free_matrix(A);
    free_matrix(B);
    return NULL;
  }
  for (int i = 0; i < A->rows * A->columns; i++) {
    Arow = i / A->columns;
    Acol = i % A->columns;
    C->values[i] = get_matrix_element(A, Arow, Acol);
  }
  free_matrix(A);
  free_matrix(B);
  return C;
}
matrix *sum_cols(matrix *A) {
  matrix *result = NULL;
  if (!A) {
    return NULL;
  }
  result = allocate_matrix(1, A->columns);
  if (!result) {
    return NULL;
  }
  for (int i = 0; i < A->columns; i++) {
    for (int j = 0; j < A->rows; j++) {
      set_matrix_element(result, 0, i, get_matrix_element(A, j, i) +
                                           get_matrix_element(result, 0, i));
    }
  }
  return result;
}
matrix *sum_rows(matrix *A) {
  matrix *result = NULL;
  if (!A) {
    return NULL;
  }
  result = allocate_matrix(A->rows, 1);
  if (!result) {
    return NULL;
  }
  for (int i = 0; i < A->rows; i++) {
    for (int j = 0; j < A->columns; j++) {
      set_matrix_element(result, i, 0, get_matrix_element(A, i, j) +
                                           get_matrix_element(result, i, 0));
    }
  }
  return result;
}
matrix *sum_matrix(matrix *A, long double dimension) {
  matrix *result = NULL;
  if (!A) {
    return NULL;
  }
  if (dimension == 0.0) {
    if (A->rows > 1) {
      result = sum_rows(A);
      free_matrix(A);
      return result;
    } else if (A->columns > 1) {
      result = sum_cols(A);
      free_matrix(A);
      return result;
    } else {
      return A;
    }
  } else if (dimension == 1.0) {
    result = sum_rows(A);
    free_matrix(A);
    return result;
  } else if (dimension == 2.0) {
    result = sum_cols(A);
    free_matrix(A);
    return result;
  }
  return A;
}
matrix *handle_func_single_arg(char *name, matrix *m) {
  if (!name || !m) {
    if (m) {
      free_matrix(m);
    }
    return NULL;
  }
  if (strcmp(name, "%sig") == 0) {
    return sigmoid(m);
  } else if (strcmp(name, "%cols") == 0) {
    return get_columns(m);
  } else if (strcmp(name, "%rows") == 0) {
    return get_rows(m);
  } else if (strcmp(name, "%lg") == 0) {
    return log_base_two(m);
  } else if (strcmp(name, "%log") == 0) {
    return log_base_ten(m);
  } else if (strcmp(name, "%sqrt") == 0) {
    return sqrtm(m);
  } else if (strcmp(name, "%mag") == 0) {
    return magnitude(m);
  } else if (strcmp(name, "%I") == 0) {
    return get_identity(m);
  } else if (strcmp(name, "%sum") == 0) {
    return sum_matrix(m, 0);
  } else {
    fprintf(file_out, "error %s -- unknown function\n", name);
    free_matrix(m);
  }
  return NULL;
}
matrix *handle_func_two_arg(char *name, matrix *A, matrix *B) {
  long double t = 0.0;
  if (!name || !A || !B) {
    if (B) {
      free_matrix(B);
    }
    if (A) {
      free_matrix(A);
    }
    return NULL;
  }
  if (strcmp(name, "%zeros") == 0) {
    return zeros(A, B);
  } else if (strcmp(name, "%ones") == 0) {
    return ones(A, B);
  } else if (strcmp(name, "%add") == 0) {
    return add_func(A, B);
  } else if (strcmp(name, "%sub") == 0) {
    return sub_func(A, B);
  } else if (strcmp(name, "%reshape") == 0) {
    return reshape(A, B);
  } else if (strcmp(name, "%sum") == 0) {
    if (!is_scalar(B)) {
      fprintf(file_out, "error -- invalid argument\n");
      free_matrix(A);
      free_matrix(B);
      return NULL;
    }
    t = get_matrix_element(B, 0, 0);
    free_matrix(B);
    return sum_matrix(A, t);
  } else {
    fprintf(file_out, "error %s -- unknown function\n", name);
    free_matrix(A);
    free_matrix(B);
  }
  return NULL;
}