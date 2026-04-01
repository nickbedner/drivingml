#include <stdio.h>

#include "mana/core/math/advmath.h"

#define TABLE_SIZE 1000
#define TABLE_NAME "SIN_TABLE"

int main() {
  // Open the output file
  FILE* f = fopen("sin_table.h", "w");

  // Print the preprocessor define to the output file
  fprintf(f, "#define %s {", TABLE_NAME);

  // Generate the table
  for (int i = 0; i < TABLE_SIZE; i++) {
    r64 x = (R32_PI * i) / (TABLE_SIZE / 2);
    r64 y = real64_sin(x);
    fprintf(f, "%ff, ", y);
  }

  // Close the preprocessor define
  fprintf(f, "}");

  // Close the output file
  fclose(f);

  return 0;
}