CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99
OBJECTS = main.o reverse.o
TARGET = invert

all: $(TARGET)

$(TARGET): $(OBJECTS)
  $(CC) $(CFLAGS) -o $@ $^

main.o: main.c reverse.h
  $(CC) $(CFLAGS) -c main.c

reverse.o: reverse.c reverse.h
  $(CC) $(CFLAGS) -c reverse.c

clean:
  rm -f $(OBJECTS) $(TARGET)

test: $(TARGET)
  @echo "Running tests..."
  @echo -n "hello" > test_input.txt
  @./$(TARGET) -i test_input.txt -o test_output.txt
  @echo -n "olleh" > expected_output.txt
  @cmp test_output.txt expected_output.txt && echo "Test 1 passed" || echo "Test 1 failed"
  @rm -f test_input.txt test_output.txt expected_output.txt
  @echo -n "abc" > test_input.txt
  @./$(TARGET) < test_input.txt > test_output.txt
  @echo -n "cba" > expected_output.txt
  @cmp test_output.txt expected_output.txt && echo "Test 2 passed" || echo "Test 2 failed"
  @rm -f test_input.txt test_output.txt expected_output.txt
