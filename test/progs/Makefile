all: main1 registers.o args.o

main1: add.o hello.o main1.c
	gcc -gdwarf-2 -o main1 add.o hello.o main1.c

add.o: add.c
	gcc -gdwarf-2 -c -o add.o add.c

struct.o: struct.c
	gcc -gdwarf-2 -c -o struct.o struct.c

registers.o: registers.c
	gcc -gdwarf-2 -c -o registers.o registers.c

args.o: args.c
	gcc -gdwarf-2 -c -o args.o args.c

hello.o: hello.c
	gcc -gdwarf-2 -c -o hello.o hello.c 

tidy:
	rm -f *.o

clean: tidy
	rm -f main1

bad:
	aoueaoeu
