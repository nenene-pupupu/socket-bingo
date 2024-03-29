#include <arpa/inet.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_CLIENT 10

#define END_COND 1

atomic_int cl_cnt = 0;

const char msg_turn[] = "#TURN";
const char msg_start[] = "#START";
const char msg_end[] = "#END";

enum game_status { READY, PLAYING, END };
typedef struct {
  atomic_int num_players;
  atomic_int p_scks[2];
  // atomic_int turn;
  int turn;
  enum game_status status;
} Setting;

void S_init(Setting *s) {
  s->num_players = 0;
  s->turn = 0;
  s->status = READY;
}

void error_handling(char *msg);
void *handle_first_client(void *args);
void *handle_second_client(void *args);

Setting sv_control;

int main(int argc, char *argv[]) {
  int sv_sck, cl_sck, str_len;
  char msg[BUF_SIZE];
  struct sockaddr_in sv_adr, cl_adr;
  socklen_t cl_adr_sz;
  pthread_t clients[MAX_CLIENT];

  if (argc != 2) {
    printf("Usage: %s <PORT>\n", argv[0]);
    exit(1);
  }

  if ((sv_sck = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    error_handling("socket() error");
  }

  memset(&sv_adr, 0, sizeof(struct sockaddr_in));
  sv_adr.sin_family = AF_INET;
  sv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
  sv_adr.sin_port = htons(atoi(argv[1]));

  if (bind(sv_sck, (struct sockaddr *)&sv_adr, sizeof(struct sockaddr_in)) ==
      -1) {
    error_handling("bind() error");
  }

  if (listen(sv_sck, 5) == -1) {
    error_handling("listen() error");
  }

  // cl_adr_sz = sizeof(struct sockaddr_in);
  S_init(&sv_control);

  while (1) {
    cl_adr_sz = sizeof(cl_adr);
    cl_sck = accept(sv_sck, (struct sockaddr *)&cl_adr, &cl_adr_sz);
    cl_cnt++;
    printf("[알림] %s 에서 접속했습니다. (현재인원:%d)\n",
           inet_ntoa(cl_adr.sin_addr), cl_cnt);

    if (cl_cnt % 2) {
      pthread_create(&clients[0], NULL, handle_first_client, (void *)&cl_sck);
      pthread_detach(clients[1]);
    } else {
      pthread_create(&clients[1], NULL, handle_second_client, (void *)&cl_sck);
      pthread_detach(clients[0]);
    }

    if (cl_cnt == 2) {
      sv_control.status = PLAYING;
      printf("0:%d\n", sv_control.p_scks[0]);
      write(sv_control.p_scks[0], msg_start, strlen(msg_start));
      write(sv_control.p_scks[1], msg_start, strlen(msg_start));
      write(sv_control.p_scks[0], msg_turn, strlen(msg_turn));
    }
  }

  close(sv_sck);

  return 0;
}

void *handle_first_client(void *args) {
  int cl_sck = *((int *)args);
  sv_control.p_scks[0] = cl_sck;
  sv_control.num_players++;

  printf("player 1 join the game. %d\n", cl_sck);

  int str_len = 0;
  char msg[BUF_SIZE];

  while (sv_control.status == READY);

  while (sv_control.status == PLAYING) {
    if (sv_control.turn == 2) continue;

    if ((str_len = read(cl_sck, msg, sizeof(msg))) != 0) {
      // 무조건 숫자가 넘어온다는 가정? (빙고수가 넘어와야 함)
      // 어떤 숫자가 왔다면?
      // 빙고인지 확인해야함
      int bingo = atoi(msg);
      printf("player 1's bingo: %d\n", bingo);

      if (bingo >= END_COND) {
        printf("player 1 wins!\n");
        write(sv_control.p_scks[0], msg_end, strlen(msg_end));
        write(sv_control.p_scks[1], msg_end, strlen(msg_end));
        sv_control.status = END;
        break;
      }
      sv_control.turn = 2;
      write(sv_control.p_scks[1], msg_turn, strlen(msg_turn));
    }
  }

  printf("player 1 quit\n");
  sv_control.num_players--;
  cl_cnt--;
  close(cl_sck);

  sv_control.status = READY;

  return NULL;
}

void *handle_second_client(void *args) {
  int cl_sck = *((int *)args);
  sv_control.p_scks[1] = cl_sck;
  sv_control.num_players++;

  printf("player 2 join the game. %d %d\n", cl_sck, sv_control.p_scks[0]);

  int str_len = 0;
  char msg[BUF_SIZE];

  while (sv_control.status == READY);

  while (sv_control.status == PLAYING) {
    if (sv_control.turn == 1) continue;

    if ((str_len = read(cl_sck, msg, sizeof(msg))) != 0) {
      // 무조건 숫자가 넘어온다는 가정? (빙고수가 넘어와야 함)
      // 어떤 숫자가 왔다면?
      // 빙고인지 확인해야함
      int bingo = atoi(msg);
      printf("player 2's bingo: %d\n", bingo);

      if (bingo >= END_COND) {
        printf("player 2 wins!\n");
        write(sv_control.p_scks[0], msg_end, strlen(msg_end));
        write(sv_control.p_scks[1], msg_end, strlen(msg_end));
        sv_control.status = END;
        break;
      }
      sv_control.turn = 1;
      write(sv_control.p_scks[0], msg_turn, strlen(msg_turn));
    }
  }

  printf("player 2 quit\n");
  sv_control.num_players--;
  cl_cnt--;
  close(cl_sck);

  sv_control.status = READY;

  return NULL;
}

void error_handling(char *msg) {
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}
