#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 5
#define CELL_COUNT (BOARD_SIZE * BOARD_SIZE)

typedef struct {
  int **board;
  int **checked;
  int **bingo;
  // int bingo;
} Board;

Board *B_init();
void B_destroy(Board *b);
int B_PUT(Board *b, int n);
int B_bingo(Board *b);
int bingo(Board *b);
void color_red();
void color_reset();
void B_print(Board *b);
