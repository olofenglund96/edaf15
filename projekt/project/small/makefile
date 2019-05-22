CC	= gcc
CFLAGS	= -m32 -Os -Wall -Wextra -std=c11 -mcpu=970 -mtune=970 -Wno-format -Wno-parentheses
CFLAGS	= -g -Wall -Wextra -std=c11 -Wno-parentheses
OUT	= fm
NOTE	= note
FILE	= small

execute: compile
	./fm

compile: clean $(FILE).o malloc.o main.o error.o
	$(CC) $(CFLAGS) -o $(OUT) -static $(FILE).o malloc.o main.o error.o -lm
	size --common $(FILE).o 

$(FILE).o: $(FILE).c
	$(CC) -v 2>&1 | tail -1 > $(NOTE)
	echo >> $(NOTE)
	echo CFLAGS = $(CFLAGS) >> $(NOTE)
	echo >> $(NOTE)
	$(CC) -include forsete.h -c $(FILE).c $(CFLAGS) >> $(NOTE)

malloc.o: malloc.c
	$(CC) -c malloc.c $(CFLAGS) 

clean:
	rm -f *.s *.o $(OUT) $(NOTE) result score note output
