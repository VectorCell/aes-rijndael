#pragma once

#ifndef AES_HEADER_INCLUDED
#define AES_HEADER_INCLUDED

#include <iostream>
#include <vector>
#include <exception>

#include <cstdio>
#include <cstdint>
#include <cstring>

using namespace std;

#define AES_BLOCK_SIZE 16

const uint8_t *inverseLookupTable (const uint8_t[256]);


class AESEngine
{
public:
	enum AES_Mode {
		AES_128_ECB,
		AES_192_ECB,
		AES_256_ECB,
		AES_128_CBC,
		AES_192_CBC,
		AES_256_CBC
	};

private:

	const AES_Mode mode;

	vector<uint8_t> key;
	vector<vector<uint8_t>> schedule;

	vector<uint8_t> prev;

	int nrounds;

public:

	AESEngine (const AES_Mode m, const vector<uint8_t>& k);
	~AESEngine ();

	vector<vector<uint8_t>> keyExpansion ();

	void encryptBlock (uint8_t *block);
	void decryptBlock (uint8_t *block);

	size_t encryptFile (FILE *in, FILE *out);
	void decryptFile (FILE *in, FILE *out, size_t padding = 0);

	void encryptSubBytes (uint8_t *block);
	void decryptSubBytes (uint8_t *block);

	void encryptShiftRows (uint8_t *block);
	void decryptShiftRows (uint8_t *block);

	void encryptMixColumns (uint8_t *block);
	void decryptMixColumns (uint8_t *block);

	void encryptAddRoundKey (uint8_t *block, uint8_t *roundkey);
	void decryptAddRoundKey (uint8_t *block, uint8_t *roundkey);

	void encryptCBC (uint8_t *block, uint8_t *prev);
	void decryptCBC (uint8_t *block, uint8_t *prev);

	void mixColumn (uint32_t *col);
	void mixColumnInv (uint32_t *col);

	void transpose (uint8_t *block);

	bool isModeECB ();
	bool isModeCBC ();
};


class IllegalAESBlockSize : public exception
{
private:

public:

	virtual const char* what() const throw()
	{
		return "AES blocks must be 16 bytes";
	}
};


template <typename T>
T rotl (T x, unsigned int n)
{
	const decltype(n) mask = (8 * sizeof(x) - 1);
	n &= mask;
	return (x << n) | (x >> ((-n)&mask));
}

template <typename T>
T rotr (T x, unsigned int n)
{
	const decltype(n) mask = (8 * sizeof(x) - 1);
	n &= mask;
	return (x >> n) | (x << ((-n)&mask));
}


/*
**  debugging aids
*/


template <typename OS, typename T>
void output_hex (OS& os, T n)
{
	static const char *DIGITS = "0123456789abcdef";
	for (int d = sizeof(n) - 1; d >= 0; --d) {
		int byte = (n >> (d << 3));
		os << DIGITS[(byte & 0xf0) >> 4] << DIGITS[byte & 0x0f];
	}	
}


template <typename OS, typename C>
void output_block (OS& os, const C& block)
{
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			output_hex(os, block[(col << 2) + row]);
			os << " ";
		}
		os << endl;
	}
	os << endl;
}

#endif // AES_HEADER_INCLUDED
