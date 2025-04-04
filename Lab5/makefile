CC=gcc
CFLAGS=-Wall -Wextra -pthread
TARGET=lab5

all: $(TARGET)
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f $(TARGET)