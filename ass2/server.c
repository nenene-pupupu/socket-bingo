#include <arpa/inet.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "board.h"
#include "common.h"

#define MAX_CLIENT 4

#define END_COND 2

atomic_int cl_cnt = 0;

enum game_status { READY, PLAYING, END };
typedef struct {
  atomic_int p_scks[2];
  int turn;
  enum game_status status;
} Setting;

typedef struct {
  int sock;
  int id;
} T_arg;

void S_init(Setting *s);
void *handle_client(void *args);

Setting sv_cntl;

int main(int argc, char *argv[]) {
  int sv_sck, cl_sck;
  struct sockaddr_in sv_adr, cl_adr;
  socklen_t cl_adr_sz;
  pthread_t clients[MAX_CLIENT];
  T_arg t_arg[MAX_CLIENT];

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

  S_init(&sv_cntl);

  while (1) {
    cl_adr_sz = sizeof(cl_adr);
    cl_sck = accept(sv_sck, (struct sockaddr *)&cl_adr, &cl_adr_sz);
    cl_cnt++;
    printf("[BINGO] %s enter the server (num players:%d)\n",
           inet_ntoa(cl_adr.sin_addr), cl_cnt);

    if (cl_cnt > 2) {
      printf("[BINGO] max_player exceed\n");
      write(cl_sck, MSG_EXCEED, strlen(MSG_EXCEED));
      close(cl_sck);
      cl_cnt--;
      continue;
    }

    if (cl_cnt % 2) {
      t_arg[0].id = 0;
      t_arg[0].sock = cl_sck;
      pthread_create(&clients[0], NULL, handle_client, &t_arg[0]);
      pthread_detach(clients[0]);
    } else {
      t_arg[1].id = 1;
      t_arg[1].sock = cl_sck;
      pthread_create(&clients[1], NULL, handle_client, &t_arg[1]);
      pthread_detach(clients[1]);
    }

    if (cl_cnt == 2) {
      // sleep 하지 않는 경우 p_scks[1] 값이 바뀌지 전에 write하는 경우가 생김
      // 세마포어로 해결하면 될듯
      // ex) 세마포어 초기 값을 2로 설정하고 player가 들어오면 값을 감소시키기
      sleep(1);
      sv_cntl.status = PLAYING;
      printf("p_scks: %d %d\n", sv_cntl.p_scks[0], sv_cntl.p_scks[1]);
      write(sv_cntl.p_scks[0], MSG_START, strlen(MSG_START));
      write(sv_cntl.p_scks[1], MSG_START, strlen(MSG_START));
      write(sv_cntl.p_scks[0], MSG_TURN, strlen(MSG_TURN));
    }
  }

  close(sv_sck);

  return 0;
}

void *handle_client(void *args) {
  int str_len = 0;
  char msg[BUF_SIZE];
  T_arg arg = *((T_arg *)args);
  int cl_sck = arg.sock;
  int me = arg.id;
  int other = 1 - me;
  sv_cntl.p_scks[me] = cl_sck;

  printf("player#%d(%d) joined the game.\n", me, cl_sck);

  while (sv_cntl.status == READY);

  int bingo;
  char other_num[3], other_bingo[3];
  while (sv_cntl.status == PLAYING) {
    if (sv_cntl.turn == other) continue;
    // 상대방이 고른 번호(2) + 상대방 빙고 수(2)
    if ((str_len = read(cl_sck, msg, sizeof(msg))) != 0) {
      printf("other's message: %s\n", msg);

      strncpy(other_num, msg, 2);
      other_num[2] = '\0';
      printf("player#%d's number: %s\n", other, other_num);
      write(sv_cntl.p_scks[other], MSG_OTHER, strlen(MSG_OTHER));

      // sleep하는 이유
      // write가 연속적으로 있는경우 하나의 메세지로 보내지는 경우가 있음
      sleep(1);

      write(sv_cntl.p_scks[other], other_num, strlen(other_num));
      strncpy(other_bingo, msg + 2, 2);
      bingo = atoi(other_bingo);
      printf("player#%d's bingo: %d\n", other, bingo);

      if (bingo >= END_COND) {
        // 비기는 것도 확인해야하는거 인지!
        printf("player %d wins!\n", other);
        write(sv_cntl.p_scks[me], MSG_WIN, strlen(MSG_WIN));
        write(sv_cntl.p_scks[other], MSG_LOSE, strlen(MSG_LOSE));
        sv_cntl.status = END;
        break;
      }
      sleep(1);
      sv_cntl.turn = other;
      write(sv_cntl.p_scks[other], MSG_TURN, strlen(MSG_TURN));
    }
  }

  printf("player#%d quit\n", me);
  cl_cnt--;
  close(cl_sck);

  S_init(&sv_cntl);

  return NULL;
}

void S_init(Setting *s) {
  s->turn = 0;
  s->status = READY;
}
