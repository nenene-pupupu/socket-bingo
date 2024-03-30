CLIENT = client
SERVER = server
TARGET = $(CLIENT) $(SERVER)

SRCS_CLIENT = \
	client.c \
	board.c \

SRCS_SERVER = \
	server.c \

OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)
OBJS_SERVER = $(SRCS_SERVER:.c=.o)

OBJS = $(OBJS_CLIENT) $(OBJS_SERVER)
DEPS = $(OBJS:.o=.d)

CFLAGS = -Wall -Wextra
CFLAGS += -MMD -MP
CLFAGS += -O3
CFLAGS += -pthread

# for debugging
CFLAGS += -g -fsanitize=address
# CFLAGS += -g -fsanitize=thread

all: $(TARGET)

$(CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) $(OBJS) $(DEPS)
	$(RM) $(TARGET)

re: clean all

-include $(DEPS)
