CSTD   := c11
CPPSTD := c++11

ifeq "$(CXX)" "g++"
	GCCVERSIONLT48 := $(shell expr `gcc -dumpversion` \< 4.8)
	ifeq "$(GCCVERSIONLT48)" "1"
		CSTD := c99
		CPPSTD := c++0x
	endif
endif

VALGRIND := valgrind

CFLAGS   := -pedantic -std=$(CSTD) -Wall -Werror -O3
CPPFLAGS := -pedantic -std=$(CPPSTD) -Wall -Werror -O3
LIBFLAGS  := -pthread -fopenmp

all : aes

aes : main.o aes.o
	$(CXX) $(CPPFLAGS) -o aes main.o aes.o $(LIBFLAGS)
	

%.o : %.cc
	$(CXX) $(CPPFLAGS) -MD -c $*.cc

test : all
	./aes t
	@echo
	@echo "Hello World!" | md5sum
	@echo "Hello World!" | tr -dc '[:print:]\n\t' | md5sum
	@echo "Hello World!" | ./aes e | ./aes d | md5sum
	@echo "Hello World!" | ./aes e | ./aes d | tr -dc '[:print:]\n\t' | md5sum
	@echo
	@echo "abcdefghijklmno" | md5sum
	@echo "abcdefghijklmno" | ./aes e | md5sum
	@echo "abcdefghijklmno" | ./aes e | ./aes d | md5sum

speedtest : test
	dd if=/dev/zero of=temp bs=4k count=0 seek=16k 2> /dev/null
	pv < temp | ./aes e > /dev/null
	pv < temp | ./aes d > /dev/null
	rm -f temp

clean :
	rm -f *.d
	rm -f *.o
	rm -f aes

-include *.d
