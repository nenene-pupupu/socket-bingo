#include <arpa/inet.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>

#include "board.h"
#include "common.h"

Board *board;

void ignore_input();
int is_numeric(const char *str);
void print_you_win();
void print_you_lose();
void print_bingo_count(int num);

int main(int argc, char *argv[]) {
  srand(time(NULL));

  int sock;
  struct sockaddr_in serv_adr;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <IP> <PORT>\n", argv[0]);
    exit(1);
  }

  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    error_handling("socket() error");
  }

  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_adr.sin_port = htons(atoi(argv[2]));

  if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1) {
    error_handling("connect() error");
  }

  printf("[BINGO] Connected!\n");
  printf("[BINGO] Ready for other player...\n");

  int n;
  char buf[BUF_SIZE];

  while (1) {
    if ((n = read(sock, buf, BUF_SIZE - 1)) == 0) {
      error_handling("server has been terminated");
    }
    buf[n] = 0;

    if (!strcmp(buf, MSG_START)) {
      break;
    }
  }

  system("clear");

  printf("[BINGO] GAME STARTED!!\n");

  Board *board = B_init();
  B_print(board);
  print_bingo_count(0);

  while (1) {
    if ((n = read(sock, buf, BUF_SIZE - 1)) == 0) {
      error_handling("server has been terminated");
    }
    buf[n] = 0;

    if (!strcmp(buf, MSG_ERROR)) {
      fprintf(stderr, "the other player has been disconnected\n");
      break;
    } else if (!strcmp(buf, MSG_WIN)) {
      print_you_win();
      break;
    } else if (!strcmp(buf, MSG_LOSE)) {
      print_you_lose();
      break;
    } else if (!strcmp(buf, MSG_TIE)) {
      printf("tie...\n");
      break;
    } else if (!strncmp(buf, MSG_OTHER, strlen(MSG_OTHER))) {
      // 상대방 어떤거 입력했는 지
      int num;
      sscanf(buf, "%*s %d", &num);

      system("clear");

      B_PUT(board, num);

      int bingo_count = bingo(board);
      B_print(board);
      print_bingo_count(bingo_count);

      if (bingo_count >= END_COND) {
        dprintf(sock, "%s", MSG_FIN);
      } else {
        dprintf(sock, "%s", MSG_NFIN);
      }
    } else if (!strcmp(buf, MSG_TURN)) {
      // 대기중에 입력받은 것들 무시
      ignore_input();

      int num;
      while (1) {
        printf("Input your number: ");
        scanf("%s", buf);

        if (!is_numeric(buf)) {
          printf("Not a number. ");
          continue;
        }

        num = atoi(buf);
        if (!B_PUT(board, num)) {
          printf("Already exists or does not exist. ");
          continue;
        }

        break;
      }

      system("clear");

      int bingo_count = bingo(board);
      B_print(board);
      print_bingo_count(bingo_count);

      dprintf(sock, "%d %d\n", num, bingo_count);
    } else {
      continue;
    }
  }

  printf("[BINGO] GAME OVER\n");

  close(sock);

  return 0;
}

void ignore_input() {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  while (1) {
    int stdin_status = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (stdin_status == -1) {
      error_handling("select() error");
    }

    if (stdin_status > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
      char str[100];
      fgets(str, sizeof(str), stdin);
    } else {
      break;
    }
  }
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
