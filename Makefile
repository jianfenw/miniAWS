# the compiler: gcc for C program, g++ for C++ program
CC = gcc
# compiler flags:
# -g 	add debugging information to the executable file
# -Wall	turns on most, but not all, compiler warnings
CFLAGS = -g -Wall 
SOCKFLAGS = -lsocket -lnsl -lresolv
# build all executable files
# They are:
# client from client.c
# aws from aws.c
# serverX from serverX.c (X = A, B, C)
all: aws_exe client serverA_exe serverB_exe serverC_exe

aws:
	./aws_exe
aws_exe: aws.o
	$(CC) $(CFLAGS) $(SOCKFLAGS) -o aws_exe aws.o
aws.o: aws.c aws.h
	$(CC) $(CFLAGS) -c aws.c

serverA:
	./serverA_exe
serverA_exe: serverA.o
	$(CC) $(CFLAGS) $(SOCKFLAGS) -o serverA_exe serverA.o
serverA.o: serverA.c serverA.h
	$(CC) $(CFLAGS) -c serverA.c

serverB:
	./serverB_exe
serverB_exe: serverB.o
	$(CC) $(CFLAGS) $(SOCKFLAGS) -o serverB_exe serverB.o
serverB.o: serverB.c serverB.h
	$(CC) $(CFLAGS) -c serverB.c

serverC:
	./serverC_exe
serverC_exe: serverC.o
	$(CC) $(CFLAGS) $(SOCKFLAGS) -o serverC_exe serverC.o
serverC.o: serverC.c serverC.h
	$(CC) $(CFLAGS) -c serverC.c

client: client.o
	$(CC) $(CFLAGS) $(SOCKFLAGS) -o client client.o
client.o: client.c client.h
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o *~ *.gch *_exe client serverA serverB serverC aws