#include <arpa/inet.h>
#include <ctype.h>  //isdigit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "board.h"
#include "common.h"

Board *board;

int is_numeric(const char *str);
void print_you_win();
void print_you_lose();
void print_bingo_count(int num);

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

    if (!strncmp(message, MSG_START, str_len)) {
      system("clear");
      puts("[BINGO] GAME STARTED!!");
      board = (Board *)malloc(sizeof(Board));
      B_init(board);
      B_print(board);
      print_bingo_count(0);
      break;
    }
  }

  int input_number;
  int bingo_count;
  // game start
  while ((str_len = read(sock, message, BUF_SIZE - 1))) {
    if (!strncmp(message, MSG_WIN, str_len)) {
      // puts("[BINGO] You WIN!!");
      print_you_win();
      break;
    }

    if (!strncmp(message, MSG_LOSE, str_len)) {
      // puts("[BINGO] You LOSE!!");
      print_you_lose();
      break;
    }

    // 상대방 어떤거 입력했는 지
    if (!strncmp(message, MSG_OTHER, strlen(MSG_OTHER))) {
      read(sock, message, BUF_SIZE - 1);
      message[2] = 0;
      system("clear");
      // printf("other player: %s\n", message);
      B_PUT(board, atoi(message));
      bingo_count = bingo(board);
      B_print(board);
      print_bingo_count(bingo_count);
      // if (B_bingo(board) >= END_COND) {
      if (bingo_count >= END_COND) {
        write(sock, MSG_FIN, strlen(MSG_FIN));
      } else {
        write(sock, MSG_NFIN, strlen(MSG_NFIN));
      }
      continue;
    }
    if (strncmp(message, MSG_TURN, str_len) != 0) continue;

    // 대기중에 입력받은 것들 무시
    fd_set fds;
    struct timeval tv;
    int stdin_status;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    while (1) {
      stdin_status = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
      if (stdin_status == -1) {
        perror("select");
        return 1;
      }
      if (stdin_status > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
        char str[100];
        fgets(str, sizeof(str), stdin);
      } else {
        // 입력 버퍼가 모두 사용 된 경우
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
    }
    system("clear");

    bingo_count = bingo(board);
    B_print(board);
    // 전송 형식: 내가고른 번호(2) + 내 빙고 수(2)
    // printf("my_bingos: %d\n", bingo_count);
    print_bingo_count(bingo_count);
    snprintf(message, 5, "%02d%02d", input_number, bingo_count);
    write(sock, message, 5);
  }

  puts("[BINGO] GMAE OVER");
  close(sock);
  return 0;
}

int is_numeric(const char *str) {
  for (int i = 0; str[i]; i++) {
    if (!isdigit(str[i])) {
      return 0;
    }
  }
  return 1;
}

void print_you_win() {
  printf("\033[1;32m");
  printf(" ██╗   ██╗ ██████╗ ██╗   ██╗   ██╗    ██╗██╗███╗   ██╗  ██╗\n");
  printf(" ██║   ██║██╔═══██╗██║   ██║   ██║    ██║██║████╗  ██║  ██║\n");
  printf(" ██║   ██║██║   ██║██║   ██║   ██║ █╗ ██║██║██╔██╗ ██║  ██║\n");
  printf("  ╚████╔╝ ██║   ██║██║   ██║   ██║███╗██║██║██║╚██╗██║  ╚═╝\n");
  printf("   ╚██╔╝  ╚██████╔╝╚██████╔╝   ╚███╔███╔╝██║██║ ╚████║  ██╗\n");
  printf("   ╚═══╝   ╚═════╝  ╚═════╝     ╚══╝╚══╝ ╚═╝╚═╝  ╚═══╝  ╚═╝\n");
  printf("\033[0m");
}

void print_you_lose() {
  printf("\033[1;32m");
  printf(
      " ██╗   ██╗ ██████╗ ██╗   ██╗   ██╗      ██████╗ ███████╗███████╗  ██╗"
      "\n");
  printf(
      " ██║   ██║██╔═══██╗██║   ██║   ██║     ██╔═══██╗██╔════╝██╔════╝  ██║"
      "\n");
  printf(
      " ██║   ██║██║   ██║██║   ██║   ██║     ██║   ██║███████╗█████╗    ██║"
      "\n");
  printf(
      "  ╚████╔╝ ██║   ██║██║   ██║   ██║     ██║   ██║╚════██║██╔══╝    ╚═╝"
      "\n");
  printf(
      "   ╚██╔╝  ╚██████╔╝╚██████╔╝   ███████╗╚██████╔╝███████║███████╗  ██╗"
      "\n");
  printf(
      "   ╚═══╝   ╚═════╝  ╚═════╝    ╚══════╝ ╚═════╝ ╚══════╝╚══════╝  ╚═╝"
      "\n");
  printf("\033[0m");
}

void print_bingo_count(int num) {
  if (num == 0) {
    printf(" ╔═╗  ╔═╦╦═╦═╦═╗ \n");
    printf(" ║║║  ║═╣║║║║║║║ \n");
    printf(" ╚═╝  ╚═╩╩╩╬╗╬═╝ \n");
    printf("           ╚═╝   \n");
  } else if (num == 1) {
    printf(" ═╗  ╔═╦╦═╦═╦═╗ \n");
    printf("  ║  ║═╣║║║║║║║ \n");
    printf(" ═╩═ ╚═╩╩╩╬╗╬═╝ \n");
    printf("          ╚═╝   \n");
  } else if (num == 2) {
    printf(" ══╗ ╔═╦╦═╦═╦═╗ \n");
    printf(" ╔═╝ ║═╣║║║║║║║ \n");
    printf(" ╚══ ╚═╩╩╩╬╗╬═╝ \n");
    printf("          ╚═╝   \n");
  } else if (num == 3) {
    printf(" ══╗ ╔═╦╦═╦═╦═╗ \n");
    printf(" ══╣ ║═╣║║║║║║║ \n");
    printf(" ══╝ ╚═╩╩╩╬╗╬═╝ \n");
    printf("          ╚═╝   \n");
  }
}
