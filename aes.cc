#include <iostream>
#include <vector>

#include <cstdio>
#include <cstdint>

#include "aes.h"

using namespace std;

int main (int argc, char *argv[])
{
	vector<uint8_t> key(16);
	AESEngine engine(key);
	return 0;
}
