#GCC Makefile for Buzz
CC = gcc
LD = gcc

# Add the following compiler flags for your particular platform
# -m64 = 64-bits compile
# -m32 = 32-bits compile
# -static = static linkage
CFLAGS = -g -x c -DNDEBUG -D_POSIX -O3 -w -static


#Debugging Compiler-flags
#CFLAGS = -g -x c -DNDEBUG -D_POSIX -O0 -w -static

# Add the following compiler flags for your particular platform
# -m64 = 64-bits compile
# -m32 = 32-bits compile
# -static = static linkage
# -lm = math library
LDFLAGS = -s -pthread -lm

#Debugging Linker Flags
#LDFLAGS = -pthread -static -g

# use this command to erase files.
RM = /bin/rm -f
# list of generated object files.
OBJS = bench.o bitinstructions.o board.o book.o consolecolors.o eval.o hash.o log.o magicmoves.o main.o movegen.o moveordering.o mt19937-64.o pgn.o recog.o resultanalysis.o search.o thread.o timemanager.o xboard.o
# program executable file name.
PROG = Buzz

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

# meta-rule for compiling source files.
%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm *.o *~
