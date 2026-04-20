CC = gcc
CFLAGS = -O2 -Wall -I ..

all: adder adder_11_10 adder_11_12

adder: adder.c
	$(CC) $(CFLAGS) -o adder adder.c

adder_11_10: adder_11_10.c
	$(CC) $(CFLAGS) -o adder_11_10 adder_11_10.c

adder_11_12: adder_11_12.c
	$(CC) $(CFLAGS) -o adder_11_12 adder_11_12.c

clean:
	rm -f adder adder_11_10 adder_11_12 *~
