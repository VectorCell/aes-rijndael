#pragma once

//#ifndef AES_HEADER_INCLUDED
//#define AES_HEADER_INCLUDED

#include <iostream>
#include <vector>
#include <exception>

#include <cstdio>
#include <cstdint>
#include <cstring>

using namespace std;


#define AES_BLOCK_SIZE 16


class AESEngine
{
public:
	enum AESMode {
		AES_128_ECB,
		AES_192_ECB,
		AES_256_ECB,
		AES_128_CBC,
		AES_192_CBC,
		AES_256_CBC
	};

private:

	const AESMode mode;

	vector<uint8_t> key;
	vector<vector<uint8_t>> schedule;

	vector<uint8_t> prev;

	int nrounds;

public:

	AESEngine (const AESMode m, const vector<uint8_t>& k);
	~AESEngine ();

	vector<vector<uint8_t>> keyExpansion ();

	void encryptBlock (uint8_t *block);
	void decryptBlock (uint8_t *block);

	void encryptFile (FILE *in, FILE *out);
	void decryptFile (FILE *in, FILE *out);

	static void encryptSubBytes (uint8_t *block);
	static void decryptSubBytes (uint8_t *block);

	static void encryptShiftRows (uint8_t *block);
	static void decryptShiftRows (uint8_t *block);

	static void encryptMixColumns (uint8_t *block);
	static void decryptMixColumns (uint8_t *block);

	static void encryptAddRoundKey (uint8_t *block, uint8_t *roundkey);
	static void decryptAddRoundKey (uint8_t *block, uint8_t *roundkey);

	static void encryptCBC (uint8_t *block, uint8_t *prev);
	static void decryptCBC (uint8_t *block, uint8_t *prev);

	static void mixColumn (uint32_t *col);
	static void mixColumnInv (uint32_t *col);

	static void transpose (uint8_t *block);

	static vector<uint8_t> loadKey (const char* filename, AESMode m);

	vector<uint8_t> generateKey ();
	static vector<uint8_t> generateKey (AESMode m);

	bool isModeECB ();
	static bool isModeECB (AESMode m);
	bool isModeCBC ();
	static bool isModeCBC (AESMode m);

	size_t keySize ();
	static size_t keySize (AESMode m);
};


class IllegalAESBlockSize : public exception
{

private:

	const char *msg;

public:

	IllegalAESBlockSize (const char *m = "AES blocks must be 16 bytes")
		: msg(m)
	{}

	virtual const char* what() const throw()
	{
		return msg;
	}
};


class IllegalAESMode : public exception
{
public:

	virtual const char* what() const throw()
	{
		return "illegal AES mode";
	}
};


class KeyGenerationException : public exception
{
private:

	const char *msg;

public:

	KeyGenerationException (const char *m) : msg(m) {}

	virtual const char* what() const throw()
	{
		return msg;
	}
};


/*
**  bitwise rotation
*/


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
void output_line (OS& os, const C& block, bool ascii = false)
{
	for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
		output_hex(os, block[k]);
		os << " ";
	}
	if (ascii) {
		os << " (";
		for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
			if (block[k] >= 32 && block[k] <= 126) {
				os << (char)block[k];
			} else {
				os << ".";
			}
		}
		os << ")";
	}
	os << endl;
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

//#endif // AES_HEADER_INCLUDED
