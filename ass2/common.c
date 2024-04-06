#include "common.h"

void write_string(int fd, const char *str) { write(fd, str, strlen(str)); }

void error_handling(char *message) {
  fprintf(stderr, "%s\n", message);
  exit(1);
}
