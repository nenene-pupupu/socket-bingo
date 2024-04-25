CLIENT = client
SERVER = server
TARGET = $(CLIENT) $(SERVER)

SRCS_COMMON = \
	common.c \

SRCS_CLIENT = \
	client.c \
	board.c \

SRCS_SERVER = \
	server.c \

OBJS_COMMON = $(SRCS_COMMON:.c=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)
OBJS_SERVER = $(SRCS_SERVER:.c=.o)

OBJS = $(OBJS_COMMON) $(OBJS_CLIENT) $(OBJS_SERVER)
DEPS = $(OBJS:.o=.d)

CFLAGS = -Wall -Wextra
CFLAGS += -MMD -MP
CLFAGS += -O3
CFLAGS += -pthread

all: $(TARGET)

$(CLIENT): $(OBJS_COMMON) $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER): $(OBJS_COMMON) $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(TARGET)

re: clean all

-include $(DEPS)
