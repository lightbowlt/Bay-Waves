exec = bay.out
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
#flags = -g


%.o: %.c include/%.h
	gcc -c -g $< -o $@

$(exec): $(objects)
	gcc $(objects) -g -o $(exec)



install:
	make
	cp ./bay.out /usr/local/bin/bay

clean:
	-rm *.out
	-rm *.o
	-rm src/*.o