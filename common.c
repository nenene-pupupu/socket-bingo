#include "common.h"

void error_handling(char *message) {
  fprintf(stderr, "%s\n", message);
  exit(1);
}
