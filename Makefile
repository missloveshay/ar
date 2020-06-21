objects = ar.o arlib.o

ar:$(objects)
	gcc -o ar $(objects)

ar.o: ar.c ar.h 
	gcc -fno-builtin -O2 -c ar.c

arlib.o:arlib.c ar.h 
	gcc -fno-builtin -O2 -c arlib.c

clean:
	rm $(objects)
