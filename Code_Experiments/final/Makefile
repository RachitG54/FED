COMPILER = $(CXX)
CPPFLAGS = -Wall -g -std=c++11
# LIBS = -lgmp -lm -lcrypto -lssl -lpthread -lboost_system -lpthread -lboost_serialization -lboost_thread -lgmp -lmsgpackc #-lpaillier
# OBJS = 
# ADDITIONAL_FILES = pubkey.txt seckey.txt ciphertext1.txt ciphertext2.txt
TARGET=$(patsubst %.cpp, %, $(wildcard *.cpp))

.PHONY: all clean 

all: $(TARGET)

%: %.cpp
	$(COMPILER) $(CPPFLAGS) $< -o $@

clean:
	rm $(TARGET) output* queries* ans*

cleanoutput:
	rm data/output* queries*
