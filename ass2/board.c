#include "board.h"

#include "common.h"

Board *B_init() {
  Board *b = (Board *)malloc(sizeof(Board));

  b->board = (int **)malloc(sizeof(int *) * BOARD_SIZE);
  b->checked = (int **)malloc(sizeof(int *) * BOARD_SIZE);
  b->bingo = (int **)malloc(sizeof(int *) * BOARD_SIZE);

  for (int i = 0; i < BOARD_SIZE; i++) {
    b->board[i] = (int *)malloc(sizeof(int) * BOARD_SIZE);
    b->checked[i] = calloc(BOARD_SIZE, sizeof(int));
    b->bingo[i] = calloc(BOARD_SIZE, sizeof(int));
  }

  int chk[CELL_COUNT] = {};
  int cnt = 0;
  while (cnt != CELL_COUNT) {
    int r = rand() % CELL_COUNT;
    if (chk[r]) continue;
    chk[r] = 1;
    b->board[cnt / 5][cnt % 5] = r + 1;
    cnt++;
  }

  return b;
}

void B_destroy(Board *b) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    free(b->board[i]);
    free(b->checked[i]);
    free(b->bingo[i]);
  }
  free(b->board);
  free(b->checked);
  free(b->bingo);

  free(b);
}

int B_PUT(Board *b, int n) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (b->board[i][j] == n) {
        if (b->checked[i][j]) {
          // already put
          return 0;
        }
        b->checked[i][j] = 1;
        return 1;
      }
    }
  }
  // n is not in the board
  return 0;
}

// return : number of bingos
int B_bingo(Board *b) {
  int bingo = 0;

  int flag_ltrb = 1;
  int flag_lbrt = 1;
  // row bingo
  for (int i = 0; i < BOARD_SIZE; i++) {
    int flag_row = 1;
    int flag_col = 1;

    // left-top to right-bottom
    if (b->checked[i][i] == 0) flag_ltrb = 0;
    // left-bottom to right-top
    if (b->checked[BOARD_SIZE - i - 1][i] == 0) flag_lbrt = 0;

    for (int j = 0; j < BOARD_SIZE; j++) {
      // row
      if (b->checked[i][j] == 0) {
        flag_row = 0;
      }
      // col
      if (b->checked[j][i] == 0) {
        flag_col = 0;
      }
    }
    if (flag_col) bingo++;
    if (flag_row) bingo++;
  }

  if (flag_ltrb) bingo++;
  if (flag_lbrt) bingo++;

  return bingo;
}

int bingo(Board *b) {
  int bingo = 0;
  int flag_ltrb = 1;
  int flag_lbrt = 1;

  int flag_row = 1;
  int flag_col = 1;

  for (int i = 0; i < BOARD_SIZE; i++) {
    if (b->checked[i][i] == 0) flag_ltrb = 0;
    if (b->checked[BOARD_SIZE - i - 1][i] == 0) flag_lbrt = 0;
  }
  if (flag_ltrb) {
    for (int i = 0; i < BOARD_SIZE; i++) b->bingo[i][i] = 1;
    bingo++;
  }
  if (flag_lbrt) {
    for (int i = 0; i < BOARD_SIZE; i++) b->bingo[BOARD_SIZE - i - 1][i] = 1;
    bingo++;
  }

  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      // row
      if (b->checked[i][j] == 0) {
        flag_row = 0;
        break;
      }
    }

    if (flag_row) {
      bingo++;
      for (int k = 0; k < BOARD_SIZE; k++) {
        b->bingo[i][k] = 1;
      }
    }
    flag_row = 1;
  }

  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      // col
      if (b->checked[j][i] == 0) {
        flag_col = 0;
        break;
      }
    }

    if (flag_col) {
      bingo++;
      for (int k = 0; k < BOARD_SIZE; k++) {
        b->bingo[k][i] = 1;
      }
    }
    flag_col = 1;
  }

  return bingo;
}

void color_red() { printf("\033[1;31m"); }

void color_green() { printf("\033[1;32m"); }

void color_reset() { printf("\033[0m"); }

void B_print(Board *b) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (i == 0) {
      printf("+");
      for (int j = 0; j < BOARD_SIZE; j++) {
        printf("------+");
      }
      puts("");
    }

    printf("|");
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (b->checked[i][j]) {
        color_red();
        if (b->bingo[i][j] && bingo(b) == END_COND) {
          color_green();
          printf(" (%2d) ", b->board[i][j]);
        } else if (b->bingo[i][j])
          printf(" (%2d) ", b->board[i][j]);
        else
          printf("  %2d  ", b->board[i][j]);
        color_reset();
        printf("|");
      } else {
        printf("  %2d  |", b->board[i][j]);
      }
    }
    puts("");

    printf("+");
    for (int j = 0; j < BOARD_SIZE; j++) {
      printf("------+");
    }
    puts("");
  }
}
