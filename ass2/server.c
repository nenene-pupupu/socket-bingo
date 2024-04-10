#include <arpa/inet.h>
#include <pthread.h>

#include "common.h"

int cl_cnt;
int cl_scks[2];

void *handle_game(void *arg);

int main(int argc, char *argv[]) {
  int sv_sck, cl_sck;
  struct sockaddr_in sv_adr, cl_adr;
  socklen_t cl_adr_sz;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <PORT>\n", argv[0]);
    exit(1);
  }

  if ((sv_sck = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    error_handling("socket() error");
  }

  memset(&sv_adr, 0, sizeof(sv_adr));
  sv_adr.sin_family = AF_INET;
  sv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
  sv_adr.sin_port = htons(atoi(argv[1]));

  if (bind(sv_sck, (struct sockaddr *)&sv_adr, sizeof(sv_adr)) == -1) {
    error_handling("bind() error");
  }

  if (listen(sv_sck, 5) == -1) {
    error_handling("listen() error");
  }

  cl_adr_sz = sizeof(cl_adr);

  while (1) {
    cl_sck = accept(sv_sck, (struct sockaddr *)&cl_adr, &cl_adr_sz);
    if (cl_sck == -1) {
      error_handling("accept() error");
    }

    cl_scks[cl_cnt++ % 2] = cl_sck;

    printf("[BINGO] %s enter the server (player#%d)\n",
           inet_ntoa(cl_adr.sin_addr), cl_cnt);

    if (cl_cnt % 2 == 0) {
      int *args = (int *)malloc(sizeof(int) * 3);
      args[0] = cl_cnt - 1;
      memcpy(args + 1, cl_scks, sizeof(int) * 2);

      pthread_t tid;
      pthread_create(&tid, NULL, handle_game, args);
      pthread_detach(tid);
    }
  }

  close(sv_sck);

  return 0;
}

void *handle_game(void *arg) {
  int *args = (int *)arg;

  int offset = args[0];
  int *sockets = args + 1;

  int n;
  char buf[BUF_SIZE];

  dprintf(sockets[0], "%s", MSG_START);
  dprintf(sockets[1], "%s", MSG_START);

  int turn = 0;
  while (1) {
    int other = 1 - turn;

    dprintf(sockets[turn], "%s", MSG_TURN);

    if ((n = read(sockets[turn], buf, BUF_SIZE - 1)) == 0) {
      fprintf(stderr, "player#%d disconnected\n", offset + turn);
      dprintf(sockets[other], "%s", MSG_ERROR);
      break;
    }
    buf[n] = 0;

    int num, bingo_count;
    sscanf(buf, "%d %d", &num, &bingo_count);

    dprintf(sockets[other], "%s %d", MSG_OTHER, num);

    if ((n = read(sockets[other], buf, BUF_SIZE - 1)) == 0) {
      fprintf(stderr, "player#%d disconnected\n", offset + other);
      dprintf(sockets[turn], "%s", MSG_ERROR);
      break;
    }
    buf[n] = 0;

    int is_turn_fin = bingo_count >= END_COND;
    int is_other_fin = !strcmp(buf, MSG_FIN);

    if (!is_turn_fin && !is_other_fin) {
      turn = other;
      continue;
    } else if (is_turn_fin && is_other_fin) {
      printf("TIE!\n");
      dprintf(sockets[0], "%s", MSG_TIE);
      dprintf(sockets[1], "%s", MSG_TIE);
    } else if (is_turn_fin) {
      printf("player#%d wins!\n", offset + turn);
      dprintf(sockets[turn], "%s", MSG_WIN);
      dprintf(sockets[other], "%s", MSG_LOSE);
    } else {
      printf("player#%d wins!\n", offset + other);
      dprintf(sockets[other], "%s", MSG_WIN);
      dprintf(sockets[turn], "%s", MSG_LOSE);
    }

    break;
  }

  close(sockets[0]);
  close(sockets[1]);
  free(args);

  return NULL;
}
