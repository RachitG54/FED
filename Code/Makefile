TARGET   = SSE
DEBUG=n
CC       = g++

CFLAGS   = -std=gnu++11 -I/usr/local/include/boost -I/usr/local/ssl/include 
ifeq ($(DEBUG),y)
	CFLAGS += -g -pg
else
	CFLAGS += -O3
endif
LINKER   = g++ -L/usr/local/lib -L/usr/local/ssl/lib

LFLAGS   = -lcrypto -lssl -lpthread -lboost_system -lpthread -lboost_serialization -lboost_thread -lgmp -lmsgpackc -ldl -lntl -lz

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
 
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
rm        = rm -f

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(rm) $(OBJECTS) 

cleanDB:
	$(rm) src/DB/* src/EDB/*

cleanlog:
	$(rm) logs/SingleDO/log/* logs/SingleDO/relevantlog/* logs/SingleDO/errorlog/* logs/SingleDO/correctnesslog/* logs/MultiDO/OPRF/log/* logs/MultiDO/OPRF/relevantlog/* logs/MultiDO/OPRF/errorlog/* logs/MultiDO/OPRF/correctnesslog/* logs/MultiDO/EqCheck/log/* logs/MultiDO/EqCheck/relevantlog/* logs/MultiDO/EqCheck/errorlog/* logs/MultiDO/EqCheck/correctnesslog/*

cleanfull: clean cleanlog cleanDB
	$(rm) ./bin/SSE

.PHONY: remove
remove: clean
	$(rm) $(BINDIR)/$(TARGET)
