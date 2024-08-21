CC = gcc
CFLAGS = -Wall -pthread

SRC = src/main.c src/proxy.c src/thread_pool.c src/lru_cache.c src/http_parser.c
OBJ = $(SRC:.c=.o)
TARGET = proxy_server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
