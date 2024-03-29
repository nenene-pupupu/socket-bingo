#include <arpa/inet.h>
#include <ctype.h>  //isdigit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include "board.h"

#define BUF_SIZE 1024

Board* board;

void error_handling(char *message);

// void bingo_message(char *msg) {
//   printf("[BINGO] %s\n", msg);
// }
int is_numeric(const char *str) {
  int i, len = strlen(str);
  for (i = 0; i < len; i++) {
    if (!isdigit(str[i])) {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  int sock, str_len;
  char message[BUF_SIZE];
  struct sockaddr_in serv_adr;

  if (argc != 3) {
    printf("Usage: %s <IP> <PORT>\n", argv[0]);
    exit(1);
  }

  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    error_handling("socket() error");
  }

  memset(&serv_adr, 0, sizeof(struct sockaddr_in));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_adr.sin_port = htons(atoi(argv[2]));

  if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(struct sockaddr_in)) ==
      -1) {
    error_handling("connect() error");
  } else {
    puts("Connected......");
  }

  // while (1) {
    puts("[BINGO] ready for other player...");

    // fputs("Input message(Q to quit): ", stdout);
    // fgets(message, BUF_SIZE, stdin);

    // write(sock, message, strlen(message));

    // if (!strcmp(message, "Q\n") || !strcmp(message, "q\n")) {
    //   break;
    // }

    // write(sock, message, strlen(message));
    while ((str_len = read(sock, message, BUF_SIZE - 1))) {
      // printf("Message from server: [%s]\n", message);

      if (!strncmp(message, "#START", str_len)) {
        puts("[BINGO] GAME STARTED!!");
        board = (Board *)malloc(sizeof(Board));
        B_init(board);
        B_print(board);
        break;
      }
    }

    int input_number;

    // game start
    while ((str_len = read(sock, message, BUF_SIZE - 1))) {
      // 서버에서 누가 이겼는 지는 안보내도 됨
      // client 상에서 먼저 빙고가 되면 자기가 이긴거 바로 확인 가능
      if (!strncmp(message, "#END", str_len)) {
        break;
      }

      if (strncmp(message, "#TURN", str_len) != 0) continue;

      // 대기중에 입력받는거 초기화하는 것도 필요할듯?
      while (1) {
        fputs("Input your number: ", stdout);
        fgets(message, BUF_SIZE, stdin);
        message[strlen(message) - 1] = 0;
        if (!is_numeric(message)) {
          printf("Not a number. ");
          continue;
        }
        input_number = atoi(message);
        if (!B_PUT(board, input_number)) {
          printf("Already exists or does not exist. ");
          continue;
        }
        break;
      }

      B_print(board);
      sprintf(message, "%d", B_bingo(board));
      write(sock, message, strlen(message));
    }

    // game end
    puts("Good bye~");
  // }

  close(sock);

  return 0;
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
