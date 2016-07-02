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

aes : aes.cc
	$(CXX) $(CPPFLAGS) -o aes aes.cc $(LIBFLAGS)

test : all
	./aes t
	echo "Hello World!" | md5sum
	echo "Hello World!" | ./aes e | ./aes d | md5sum
	echo "abcdefghijklmno" | md5sum
	echo "abcdefghijklmno" | ./aes e | ./aes d | md5sum

clean :
	rm -f *.d
	rm -f *.o
	rm -f aes

-include *.d
