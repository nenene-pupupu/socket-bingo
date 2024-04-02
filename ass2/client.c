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

const char MSG_START[] = "#START";
const char MSG_TURN[] = "#TURN";
const char MSG_EXCEED[] = "#EXCEED";
const char MSG_OTHER[] = "#OTHER";
const char MSG_WIN[] = "#WIN";
const char MSG_LOSE[] = "#LOSE";
const char MSG_TIE[] = "#TIE";

Board *board;

void error_handling(char *message);
int is_numeric(const char *str);

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
    puts("[BINGO] Connected!");
  }

  puts("[BINGO] Ready for other player...");

  while ((str_len = read(sock, message, BUF_SIZE - 1))) {
    if (!strncmp(message, MSG_EXCEED, str_len)) {
      puts("[BINGO] Error: The game already started");
      close(sock);
      return 0;
    }

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
    // 서버에서 누가 이겼는 지는 안보내도 되려나?
    // client 상에서 먼저 빙고가 되면 자기가 이긴거 바로 확인 가능
    // 비기는 것도 확인해야하는거 인지!
    if (!strncmp(message, MSG_WIN, str_len)) {
      puts("[BINGO] You WIN!!");
      break;
    }

    if (!strncmp(message, MSG_LOSE, str_len)) {
      puts("[BINGO] You LOSE!!");
      break;
    }

    // 상대방 어떤거 입력했는 지
    if (!strncmp(message, MSG_OTHER, strlen(MSG_OTHER))) {
      read(sock, message, BUF_SIZE - 1);
      message[2] = 0;
      printf("other player: %s\n", message);
      B_PUT(board, atoi(message));
      B_print(board);
      continue;
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
    // 내가 고른 번호와 빙고 개수를 보내줘야함
    printf("my_bingos: %d\n", B_bingo(board));
    snprintf(message, 5, "%02d%02d", input_number, B_bingo(board));
    write(sock, message, 5);
  }

  puts("[BINGO] Good bye~");
  close(sock);
  return 0;
}

int is_numeric(const char *str) {
  int i, len = strlen(str);
  for (i = 0; i < len; i++) {
    if (!isdigit(str[i])) {
      return 0;
    }
  }
  return 1;
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
