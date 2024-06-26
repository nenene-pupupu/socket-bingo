#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 1024

#define END_COND 3

#define MSG_START "#START"
#define MSG_TURN "#TURN"
#define MSG_EXCEED "#EXCEED"
#define MSG_OTHER "#OTHER"
#define MSG_FNSH "#FNSH"
#define MSG_NFIN "#NFIN"
#define MSG_WIN "#WIN"
#define MSG_LOSE "#LOSE"
#define MSG_TIE "#TIE"
#define MSG_ERROR "#ERROR"

void error_handling(char *message);
