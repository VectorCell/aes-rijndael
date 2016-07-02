#include <iostream>
#include <vector>

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include "aes.h"


vector<uint8_t> random_block ()
{
	vector<uint8_t> block(16);
	for (int k = 0; k < 16; ++k) {
		block[k] = (uint8_t)(0xff & rand());
	}
	return block;
}


int runtests ()
{
	vector<uint8_t> key(16, 0);
	AESEngine engine(AESEngine::AES_Mode::AES_128_ECB, key);

	vector<uint8_t> block(16);
	vector<uint8_t> copy = block;

	cout << "testing subBytes" << endl;
	block = random_block();
	copy = block;
	engine.encryptSubBytes(&block[0]);
	for (int k = 0; k < 16; ++k)
		if (block[k] == copy[k])
			return 1;
	engine.decryptSubBytes(&block[0]);
	for (int k = 0; k < 16; ++k)
		if (block[k] != copy[k])
			return 1;

	cout << "testing transpose" << endl;
	block = random_block();
	copy = block;
	engine.transpose(&block[0]);
	engine.transpose(&block[0]);
	for (int k = 0; k < 16; ++k)
		if (block[k] != copy[k])
			return 1;

	cout << "testing shiftRows" << endl;
	block = random_block();
	copy = block;
	engine.encryptShiftRows(&block[0]);
	engine.decryptShiftRows(&block[0]);
	for (int k = 0; k < 16; ++k)
		if (block[k] != copy[k])
			return 1;

	cout << "testing mixColumn" << endl;
	for (int k = 0; k < 8; ++k) {
		uint32_t n = rand();
		uint32_t copy = n;
		engine.mixColumn(&n);
		engine.mixColumnInv(&n);
		if (n != copy)
			return 1;
	}

	cout << "testing mixColumns" << endl;
	block = random_block();
	copy = block;
	engine.encryptMixColumns(&block[0]);
	engine.decryptMixColumns(&block[0]);
	for (int k = 0; k < 16; ++k)
		if (block[k] != copy[k])
			return 1;

	cout << endl << "PASS" << endl;

	return 0;
}


int main (int argc, char *argv[])
{
	vector<uint8_t> key(16, 0);
	AESEngine engine(AESEngine::AES_Mode::AES_128_ECB, key);
	if (argc >= 2) {
		if (argv[1][0] == 'e') {
			engine.encryptFile(stdin, stdout);
		} else if (argv[1][0] == 'd') {
			engine.decryptFile(stdin, stdout);
		} else if (argv[1][0] == 't') {
			int ret = runtests();
			if (ret != 0) {
				cout << endl << "FAIL" << endl;
				return ret;
			}
		}
	}
	return 0;
}
