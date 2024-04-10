#include <arpa/inet.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/socket.h>

#include "board.h"
#include "common.h"

#define MAX_CLIENT 4

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

pthread_mutex_t turn_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t turn_cond = PTHREAD_COND_INITIALIZER;

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
      dprintf(cl_sck, "%s", MSG_EXCEED);
      close(cl_sck);
      cl_cnt--;
      continue;
    }

    int i = 1 - cl_cnt % 2;
    t_arg[i].id = i;
    t_arg[i].sock = cl_sck;
    sv_cntl.p_scks[i] = cl_sck;
    pthread_create(&clients[i], NULL, handle_client, &t_arg[i]);
    pthread_detach(clients[i]);

    if (cl_cnt == 2) {
      // sleep 하지 않는 경우 p_scks[1] 값이 바뀌지 전에 write하는 경우가 생김
      // 세마포어로 해결하면 될듯
      // ex) 세마포어 초기 값을 2로 설정하고 player가 들어오면 값을 감소시키기
      usleep(200);
      sv_cntl.status = PLAYING;
      printf("p_scks: %d %d\n", sv_cntl.p_scks[0], sv_cntl.p_scks[1]);
      dprintf(sv_cntl.p_scks[0], "%s", MSG_START);
      dprintf(sv_cntl.p_scks[1], "%s", MSG_START);
      dprintf(sv_cntl.p_scks[0], "%s", MSG_TURN);
    }
  }
  close(sv_sck);

  pthread_mutex_destroy(&turn_mutex);
  pthread_cond_destroy(&turn_cond);

  return 0;
}

void *handle_client(void *args) {
  char msg[BUF_SIZE];
  T_arg arg = *((T_arg *)args);
  int cl_sck = arg.sock;
  int me = arg.id;
  int other = 1 - me;

  printf("player#%d(%d) joined the game.\n", me, cl_sck);

  while (sv_cntl.status == READY)
    ;

  int bingo;
  char other_num[3], other_bingo[3];
  while (sv_cntl.status == PLAYING) {
    pthread_mutex_lock(&turn_mutex);
    while (sv_cntl.turn == other) {
      pthread_cond_wait(&turn_cond, &turn_mutex);
    }
    pthread_mutex_unlock(&turn_mutex);

    // 상대방이 고른 번호(2) + 상대방 빙고 수(2)
    if (read(cl_sck, msg, sizeof(msg)) != 0) {
      printf("other's message: %s\n", msg);

      strncpy(other_num, msg, 2);
      other_num[2] = '\0';
      printf("player#%d's number: %s\n", other, other_num);
      dprintf(sv_cntl.p_scks[other], "%s", MSG_OTHER);

      // sleep하는 이유
      // write가 연속적으로 있는경우 하나의 메세지로 보내지는 경우가 있음
      usleep(200);

      dprintf(sv_cntl.p_scks[other], "%s", other_num);

      usleep(200);

      strncpy(other_bingo, msg + 2, 2);
      bingo = atoi(other_bingo);
      printf("player#%d's bingo: %d\n", other, bingo);

      // 상대방이 고른 번호로 인해 빙고 조건을 만족했는 지 확인
      int flag = 0;
      if (read(sv_cntl.p_scks[other], msg, sizeof(msg)) != -1) {
        if (!strncmp(msg, MSG_FIN, strlen(MSG_FIN))) {
          // 내가 고른 것으로 인해 상대방이 빙고 조건을 만족함
          flag = 1;
        }
      }

      // 내가 빙고 조건을 만족한 경우
      if (bingo >= END_COND) {
        if (flag) {
          // 상대방도 내가 고른 번호로 빙고 조건을 만족한 경우, 무승부
          printf("TIE!\n");
          dprintf(sv_cntl.p_scks[me], "%s", MSG_TIE);
          dprintf(sv_cntl.p_scks[other], "%s", MSG_TIE);
        } else {
          // 상대방은 아직 빙고 조건을 만족하지 못함, 내 승리
          printf("player %d wins!\n", other);
          dprintf(sv_cntl.p_scks[me], "%s", MSG_WIN);
          dprintf(sv_cntl.p_scks[other], "%s", MSG_LOSE);
        }
        sv_cntl.status = END;
        break;
      }

      // 내가 고른 것으로 인해 상대방이 먼저 빙고 조건을 만족한 경우
      if (flag) {
        printf("player %d wins!\n", me);
        dprintf(sv_cntl.p_scks[other], "%s", MSG_WIN);
        dprintf(sv_cntl.p_scks[me], "%s", MSG_LOSE);
      }

      pthread_mutex_lock(&turn_mutex);
      sv_cntl.turn = other;
      dprintf(sv_cntl.p_scks[other], "%s", MSG_TURN);
      pthread_cond_signal(&turn_cond);
      pthread_mutex_unlock(&turn_mutex);
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
