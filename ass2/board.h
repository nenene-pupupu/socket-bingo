#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 5

typedef struct {
  int **board;
  int **checked;
  // int bingo;
} Board;

void B_init(Board *b);
void B_destroy(Board *b);
int B_PUT(Board *b, int n); 
int B_bingo(Board *b);
void color_red();
void color_reset();
void B_print(Board *b);