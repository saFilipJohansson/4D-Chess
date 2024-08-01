#gcc -o test test.c -I/usr/local/include/SDL2 -L/usr/local/lib

CC = gcc

CPPFLAGS = -I/usr/local/include/SDL2
#CPPFLAGS = -I.
CFLAGS  = -Wall -Wextra -Wpedantic
CFLAGS	+= -std=c99
#CFLAGS  += -O2
CFLAGS  += -O0 -g
LDFLAGS = -L/usr/local/lib
#LDFLAGS += -g
LDLIBS  = -lSDL2 -lSDL2_image

TARGETS = main test_chess_logic

all: $(TARGETS)

#main: main.o chess_logic.o chess_init.o graphics.o
main: main.c chess_init.c chess_logic.c graphics.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o main main.c chess_init.c chess_logic.c chess_utils.c graphics.c

# Note: .c file chess_logic.c included in chess_logic_tests. Is dependency on chess_logic.c necessary? (probably)
test_chess_logic: chess_logic.c unit_tests/chess_logic_tests.c chess_init.c chess_logic.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o test_chess_logic unit_tests/chess_logic_tests.c chess_init.c chess_utils.c

test: test_chess_logic
	./test_chess_logic

clean:
	rm -f *.o $(TARGETS)

distclean: clean
	rm *.d

.PHONY: all test clean distclean

