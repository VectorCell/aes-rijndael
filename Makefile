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
	@./test.sh

speedtest : test
	dd if=/dev/zero of=temp bs=4k count=0 seek=16k 2> /dev/null
	pv < temp | ./aes e > /dev/null
	pv < temp | ./aes d > /dev/null
	rm -f temp

paddingtest : test
	echo "Hello world, this is Brandon! I'm so pleased to meet you!" \
		| ./aes e 2> encryption.log      \
		| ./aes d 2> decryption.log      \
		| tee output.log                 \
		| hexdump
	@echo
	cat encryption.log
	cat decryption.log
	cat output.log
	@rm -f encryption.log decryption.log output.log

clean :
	rm -f *.d
	rm -f *.o
	rm -f aes

-include *.d
