all:
	$(CC) $(CFLAGS) -lm -std=c99 -Wall -o bin/kentropy src/*.c

clean:
	rm bin/kentropy
