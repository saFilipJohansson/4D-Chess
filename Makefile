#gcc -o test test.c -I/usr/local/include/SDL2 -L/usr/local/lib

CC = gcc

CPPFLAGS = -I/usr/local/include/SDL2
#CPPFLAGS = -I.
CFLAGS  = -Wall -Wextra -Wpedantic
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



main1.5: main.c chess_logic.c chess_init.c graphics.c chess.h graphics.h
	gcc -Wall -g -o main main.c chess_logic.c chess_init.c chess_utils.c graphics.c -I/usr/local/include/SDL2 -L/usr/local/lib -lSDL2 -lSDL2_image

test_chess_logic_c2: unit_tests/chess_logic_tests.c chess_init.c chess_logic.c
	gcc -o test_chess_logic_c unit_tests/chess_logic_tests.c chess_init.c


playground: playground.c
	gcc -Wall -g -o playground playground.c chess_logic.c chess_init.c graphics.c -I/usr/local/include/SDL2 -L/usr/local/lib -lSDL2 -lSDL2_image

main2: main.c chess_logic.c chess_init.c graphics.c chess.h graphics.h
	gcc -Wall -g -o main2 main.c chess_logic.c chess_init.c graphics.c \
	SDL2-2.30.4/src/*.c \
	SDL2-2.30.4/src/atomic/*.c \
	SDL2-2.30.4/src/audio/*.c \
	SDL2-2.30.4/src/audio/dummy/*.c \
	SDL2-2.30.4/src/cpuinfo/*.c \
	SDL2-2.30.4/src/events/*.c \
	SDL2-2.30.4/src/file/*.c \
	SDL2-2.30.4/src/haptic/*.c \
	SDL2-2.30.4/src/haptic/dummy/*.c \
	SDL2-2.30.4/src/hidapi/*.c \
	SDL2-2.30.4/src/joystick/*.c \
	SDL2-2.30.4/src/joystick/dummy/*.c \
	SDL2-2.30.4/src/loadso/dummy/*.c \
	SDL2-2.30.4/src/power/*.c \
	SDL2-2.30.4/src/filesystem/dummy/*.c \
	SDL2-2.30.4/src/locale/*.c \
	SDL2-2.30.4/src/locale/dummy/*.c \
	SDL2-2.30.4/src/misc/*.c \
	SDL2-2.30.4/src/misc/dummy/*.c \
	SDL2-2.30.4/src/render/*.c \
	SDL2-2.30.4/src/render/software/*.c \
	SDL2-2.30.4/src/sensor/*.c \
	SDL2-2.30.4/src/sensor/dummy/*.c \
	SDL2-2.30.4/src/stdlib/*.c \
	SDL2-2.30.4/src/libm/*.c \
	SDL2-2.30.4/src/thread/*.c \
	SDL2-2.30.4/src/thread/generic/*.c \
	SDL2-2.30.4/src/timer/*.c \
	SDL2-2.30.4/src/timer/dummy/*.c \
	SDL2-2.30.4/src/video/*.c \
	SDL2-2.30.4/src/video/yuv2rgb/*.c \
	SDL2-2.30.4/src/video/dummy/*.c \
	SDL2_image-2.8.2/src/*.c \
	-ISDL2-2.30.4/include -ISDL2_image-2.8.2/include

#-ISLD2-2.30.4/include -I/usr/local/include/SDL2 -L/usr/local/lib -lSDL2 -lSDL2_image
#-idirafter SDL2-2.30.4/include -idirafter SDL2_image-2.8.2/include
