CC = gcc
CFLAGS = -Wall -Werror -pthread

# Source files
SRCS = Queue_B.c Queue_U.c News.c Consumer_Producer.c

# Object files
OBJS = $(SRCS:.c=.o)

# Binary executable
TARGET = Consumer_Producer.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.out
