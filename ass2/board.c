#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BOARD_SIZE 5

typedef struct {
  int **board;
  int **checked;
  // int bingo;
} Board;

void B_init(Board *b) {
  b->board = (int **)malloc(sizeof(int *) * BOARD_SIZE);
  b->checked = (int **)malloc(sizeof(int *) * BOARD_SIZE);
  for (int i = 0; i < BOARD_SIZE; ++i) {
    b->board[i] = (int *)malloc(sizeof(int) * BOARD_SIZE);
    b->checked[i] = (int *)malloc(sizeof(int) * BOARD_SIZE);
  }

  int chk[BOARD_SIZE * BOARD_SIZE] = {
      0,
  };
  int cnt = 0;
  while (cnt != BOARD_SIZE * BOARD_SIZE) {
    int r = rand() % (BOARD_SIZE * BOARD_SIZE);
    if (chk[r]) continue;
    chk[r] = 1;
    b->board[cnt / 5][cnt % 5] = r + 1;
    cnt++;
  }
}

void B_destroy(Board *b) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    free(b->board[i]);
    free(b->checked[i]);
  }
  free(b->board);
  free(b->checked);

  free(b);
}

int B_PUT(Board *b, int n) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
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

// return : bingo 개수
int B_bingo(Board *b) {
  int bingo = 0;

  int flag_ltrb = 1;
  int flag_lbrt = 1;
  // row bingo
  for (int i = 0; i < BOARD_SIZE; ++i) {
    int flag_row = 1;
    int flag_col = 1;

    // left-top to right-bottom
    if (b->checked[i][i] == 0) flag_ltrb = 0;
    // left-bottom to right-top
    if (b->checked[BOARD_SIZE - i - 1][i] == 0) flag_lbrt = 0;

    for (int j = 0; j < BOARD_SIZE; ++j) {
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

void color_red() { printf("\033[1;31m"); }

void color_reset() { printf("\033[0m"); }

void B_print(Board *b) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    if (i == 0) {
      printf("+");
      for (int j = 0; j < BOARD_SIZE; ++j) {
        printf("------+");
      }
      puts("");
    }

    printf("|");
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (b->checked[i][j]) {
        color_red();
        printf("  %2d  ", b->board[i][j]);
        color_reset();
        printf("|");
      } else {
        printf("  %2d  |", b->board[i][j]);
      }
    }
    puts("");

    printf("+");
    for (int j = 0; j < BOARD_SIZE; ++j) {
      printf("------+");
    }
    puts("");
  }
}

int main() {
  srand(time(NULL));

  Board *my_board = (Board *)malloc(sizeof(Board));

  B_init(my_board);

  int num;
  B_print(my_board);

  while (1) {
    scanf("%d", &num);
    if (num == 0) break;
    B_PUT(my_board, num);
    B_print(my_board);
    printf("Bingo: %d\n", B_bingo(my_board));
  }

  B_destroy(my_board);

  return 0;
}