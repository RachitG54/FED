# ***********************************************
#                    JustGarble
# ***********************************************

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
TESTDIR   = test
OBJECTFULL = obj/*.o

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

IDIR =../include
CC=gcc 
CFLAGS= -std=gnu89 -g -lm -lrt -lpthread -maes -msse4 -lmsgpackc -march=native -I$(IDIR)


AES = AESFullTest
LARGE = LargeCircuitTest
FILE = CircuitFileTest
FILE2 = CircuitFileTest2
HAM = HammingTest
HAM2 = HammingTest2

TEST = Test

rm = rm --f

all: AES LARGE FILE FILE2 HAM HAM2 TEST

AES: $(OBJECTS) $(TESTDIR)/$(AES).c
	$(CC) $(OBJECTFULL) $(TESTDIR)/$(AES).c -o $(BINDIR)/$(AES).out $(LIBS) $(CFLAGS) 

LARGE: $(OBJECTS) $(TESTDIR)/$(LARGE).c
	$(CC) $(OBJECTFULL) $(TESTDIR)/$(LARGE).c -o $(BINDIR)/$(LARGE).out $(LIBS) $(CFLAGS) 

FILE: $(OBJECTS) $(TESTDIR)/$(FILE).c
	$(CC) $(OBJECTFULL) $(TESTDIR)/$(FILE).c -o $(BINDIR)/$(FILE).out $(LIBS) $(CFLAGS) 

FILE2: $(OBJECTS) $(TESTDIR)/$(FILE2).c
	$(CC) $(OBJECTFULL) $(TESTDIR)/$(FILE2).c -o $(BINDIR)/$(FILE2).out $(LIBS) $(CFLAGS) 

HAM: $(OBJECTS) $(TESTDIR)/$(HAM).c
	$(CC) $(OBJECTFULL) $(TESTDIR)/$(HAM).c -o $(BINDIR)/$(HAM).out $(LIBS) $(CFLAGS)

HAM2: $(OBJECTS) $(TESTDIR)/$(HAM2).c
	$(CC) $(OBJECTFULL) $(TESTDIR)/$(HAM2).c -o $(BINDIR)/$(HAM2).out $(LIBS) $(CFLAGS) 

TEST: $(OBJECTS) $(TESTDIR)/$(TEST).c
	$(CC) $(OBJECTFULL) $(TESTDIR)/$(TEST).c -o $(BINDIR)/$(TEST).out $(LIBS) $(CFLAGS) 


$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $< -o $@ $(LIBS) $(CFLAGS) 

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@$(rm) $(BINDIR)/$(AES)
	@$(rm) $(BINDIR)/$(LARGE)
	@$(rm) $(BINDIR)/$(FILE)

