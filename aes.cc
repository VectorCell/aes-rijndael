#include <iostream>
#include <vector>

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include "aes.h"

using namespace std;

AESEngine::AESEngine (const AES_Mode m, const vector<uint8_t>& k)
	: mode(m)
{
	key = vector<uint8_t>(k.size());
	copy(k.begin(), k.end(), key.begin());

	nrounds = key.size() / 4 + 6;
	schedule = keyExpansion();

	prev = vector<uint8_t>(AES_BLOCK_SIZE);
}


AESEngine::~AESEngine ()
{
	fill(key.begin(), key.end(), 0);
	for (unsigned int b = 0; b < schedule.size(); ++b) {
		for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
			schedule[b][k] = 0;
		}
	}
	nrounds = 0;
}


vector<vector<uint8_t>> AESEngine::keyExpansion ()
{
	int nk = key.size() / 4;
	int ns = 4 * (nrounds + 1);
	vector<vector<uint8_t>> w(ns, vector<uint8_t>(4));
	for (unsigned int k = 0; k < key.size(); ++k) {
		w[k / 4][k % 4] = key[k];
	}
	for (int c = nk; c < ns; ++c) {
		if ((c % nk) == 0) {
			w[c][0] = w[c - nk][0] ^ SBOX[w[c - 1][1]] ^ RCON[c / nk];
			for (int r = 1; r < 4; ++r) {
				w[c][r] = w[c - nk][r] ^ SBOX[w[c - 1][(r + 1) % 4]];
			}
		} else if (nk > 6 && (c % nk) == 4) {
			for (int r = 0; r < 4; ++r) {
				w[c][r] = w[c][r - nk] ^ SBOX[w[c - 1][r]];
			}
		} else {
			for (int r = 0; r < 4; ++r) {
				w[c][r] = w[c][r - nk] ^ w[c - 1][r];
			}
		}
	}
	vector<vector<uint8_t>> sched = 
		vector<vector<uint8_t>>(nrounds + 1, vector<uint8_t>(AES_BLOCK_SIZE));
	for (int b = 0; b < ns / 4; ++b) {
		for (int c = 0; c < 4; ++c) {
			for (int r = 0; r < 4; ++r) {
				sched[b][c * 4 + r] = w[b * 4 + c][r];
			}
		}
	}
	return sched;
}


void AESEngine::encryptBlock (uint8_t *block)
{
	encryptAddRoundKey(block, &schedule[0][0]);
	for (int r = 0; r < nrounds; ++r) {
		encryptSubBytes(block);
		encryptShiftRows(block);
		encryptMixColumns(block);
		encryptAddRoundKey(block, &schedule[r][0]);
	}
	encryptSubBytes(block);
	encryptShiftRows(block);
	encryptAddRoundKey(block, &schedule[nrounds][0]);
	if (isModeCBC())
		encryptCBC(block, &prev[0]);
}


void AESEngine::decryptBlock (uint8_t *block)
{
	if (isModeCBC())
		decryptCBC(block, &prev[0]);
	decryptAddRoundKey(block, &schedule[nrounds][0]);
	decryptShiftRows(block);
	decryptSubBytes(block);
	for (int r = nrounds - 1; r >= 0; --r) {
		decryptAddRoundKey(block, &schedule[r][0]);
		decryptMixColumns(block);
		decryptShiftRows(block);
		decryptSubBytes(block);
	}
	decryptAddRoundKey(block, &schedule[0][0]);
}


void AESEngine::encryptFile (FILE *infile, FILE *outfile)
{
	vector<uint8_t> buffer(AES_BLOCK_SIZE);
	uint8_t *buf = (uint8_t *)&buffer[0];
	uint64_t count = 0;
	while ((count = fread(buf, 1, buffer.size(), infile)) > 0) {
		encryptBlock(buf);
		fwrite(buf, 1, count, outfile);
		memset(buf, 0, buffer.size());
	}
}


void AESEngine::decryptFile (FILE *infile, FILE *outfile)
{
	vector<uint8_t> buffer(AES_BLOCK_SIZE);
	uint8_t *buf = (uint8_t *)&buffer[0];
	uint64_t count = 0;
	while ((count = fread(buf, 1, buffer.size(), infile)) > 0) {
		decryptBlock(buf);
		fwrite(buf, 1, count, outfile);
		memset(buf, 0, buffer.size());
	}
}

//                 #      mmmmm           m                 
//    mmm   m   m  #mmm   #    # m   m  mm#mm   mmm    mmm  
//   #   "  #   #  #" "#  #mmmm" "m m"    #    #"  #  #   " 
//    """m  #   #  #   #  #    #  #m#     #    #""""   """m 
//   "mmm"  "mm"#  ##m#"  #mmmm"  "#      "mm  "#mm"  "mmm" 
//                                m"                        
//                               ""                         


void AESEngine::encryptSubBytes (uint8_t *block)
{
	for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
		block[k] = SBOX[block[k]];
	}
}


void AESEngine::decryptSubBytes (uint8_t *block)
{
	for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
		block[k] = SBOX_INV[block[k]];
	}
}


//          #        "      m""    m    mmmmm                      
//    mmm   # mm   mmm    mm#mm  mm#mm  #   "#  mmm  m     m  mmm  
//   #   "  #"  #    #      #      #    #mmmm" #" "# "m m m" #   " 
//    """m  #   #    #      #      #    #   "m #   #  #m#m#   """m 
//   "mmm"  #   #  mm#mm    #      "mm  #    " "#m#"   # #   "mmm" 
//


void AESEngine::encryptShiftRows (uint8_t *block)
{
	transpose(block);
	for (int col = 1; col < 4; ++col) {
		uint32_t *t = (uint32_t *)(block + (4 * col));
		*t = rotl(*t, col << 3);
	}
	transpose(block);
}


void AESEngine::decryptShiftRows (uint8_t *block)
{
	transpose(block);
	for (int col = 1; col < 4; ++col) {
		uint32_t *t = (uint32_t *)(block + (4 * col));
		*t = rotr(*t, col << 3);
	}
	transpose(block);
}

//            "             mmm         ""#                               
//   mmmmm  mmm    m   m  m"   "  mmm     #    m   m  mmmmm  m mm    mmm  
//   # # #    #     #m#   #      #" "#    #    #   #  # # #  #"  #  #   " 
//   # # #    #     m#m   #      #   #    #    #   #  # # #  #   #   """m 
//   # # #  mm#mm  m" "m   "mmm" "#m#"    "mm  "mm"#  # # #  #   #  "mmm" 
//


void AESEngine::encryptMixColumns (uint8_t *block)
{
	uint32_t *col = (uint32_t *)block;
	while ((uint8_t *)col < (block + AES_BLOCK_SIZE))
		mixColumn(col++);
}


void AESEngine::decryptMixColumns (uint8_t *block)
{
	uint32_t *col = (uint32_t *)block;
	while ((uint8_t *)col < (block + AES_BLOCK_SIZE))
		mixColumnInv(col++);
}


void AESEngine::mixColumn (uint32_t *col)
{
	uint32_t temp = *col;
	uint8_t *a = (uint8_t *)&temp;
	uint8_t *s = (uint8_t *)col;
	s[0] = GMUL_2[a[0]] ^ GMUL_3[a[1]] ^        a[2]  ^        a[3];
	s[1] =        a[0]  ^ GMUL_2[a[1]] ^ GMUL_3[a[2]] ^        a[3];
	s[2] =        a[0]  ^        a[1]  ^ GMUL_2[a[2]] ^ GMUL_3[a[3]];
	s[3] = GMUL_3[a[0]] ^        a[1]  ^        a[2]  ^ GMUL_2[a[3]];
}

void AESEngine::mixColumnInv (uint32_t *col)
{
	uint32_t temp = *col;
	uint8_t *a = (uint8_t *)&temp;
	uint8_t *s = (uint8_t *)col;
	s[0] = GMUL_E[a[0]] ^ GMUL_B[a[1]] ^ GMUL_D[a[2]] ^ GMUL_9[a[3]];
	s[1] = GMUL_9[a[0]] ^ GMUL_E[a[1]] ^ GMUL_B[a[2]] ^ GMUL_D[a[3]];
	s[2] = GMUL_D[a[0]] ^ GMUL_9[a[1]] ^ GMUL_E[a[2]] ^ GMUL_B[a[3]];
	s[3] = GMUL_B[a[0]] ^ GMUL_D[a[1]] ^ GMUL_9[a[2]] ^ GMUL_E[a[3]];
}


//              #      #  mmmmm                           #  m    m              
//    mmm    mmm#   mmm#  #   "#  mmm   m   m  m mm    mmm#  #  m"   mmm   m   m 
//   "   #  #" "#  #" "#  #mmmm" #" "#  #   #  #"  #  #" "#  #m#    #"  #  "m m" 
//   m"""#  #   #  #   #  #   "m #   #  #   #  #   #  #   #  #  #m  #""""   #m#  
//   "mm"#  "#m##  "#m##  #    " "#m#"  "mm"#  #   #  "#m##  #   "m "#mm"   "#   
//                                                                          m"   
//                                                                         ""    


void AESEngine::encryptAddRoundKey (uint8_t *block, uint8_t *roundkey)
{
	uint_fast32_t *blk = (uint_fast32_t *)block;
	uint_fast32_t *rk  = (uint_fast32_t *)roundkey;
	while ((int *)blk < (int *)(block + AES_BLOCK_SIZE)) {
		*(blk++) ^= *(rk++);
	}
}


void AESEngine::decryptAddRoundKey (uint8_t *block, uint8_t *roundkey)
{
	encryptAddRoundKey(block, roundkey);
}


//     mmm  mmmmm    mmm 
//   m"   " #    # m"   "
//   #      #mmmm" #     
//   #      #    # #     
//    "mmm" #mmmm"  "mmm"
//


void AESEngine::encryptCBC (uint8_t *block, uint8_t *prev)
{
	for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
		block[k] ^= prev[k];
		prev[k] = block[k];
	}
}


void AESEngine::decryptCBC (uint8_t *block, uint8_t *prev)
{
	for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
		uint8_t temp = block[k];
		block[k] ^= prev[k];
		prev[k] = temp;
	}
}


//            m      "    ""#   
//   m   m  mm#mm  mmm      #   
//   #   #    #      #      #   
//   #   #    #      #      #   
//   "mm"#    "mm  mm#mm    "mm 
//


void AESEngine::transpose (uint8_t *block)
{
	for (int c = 0; c < 4; ++c) {
		for (int r = c + 1; r < 4; ++r) {
			swap(block[c * 4 + r], block[r * 4 + c]);
		}
	}
}

bool AESEngine::isModeECB ()
{
	return mode <= AES_Mode::AES_256_ECB;
}

bool AESEngine::isModeCBC ()
{
	return !isModeECB() && (mode <= AES_Mode::AES_256_CBC);
}


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

	cout << endl << "PASS" << endl << endl;

	return 0;
}


int main (int argc, char *argv[])
{
	vector<uint8_t> key(16, 0);
	AESEngine engine(AESEngine::AES_Mode::AES_128_CBC, key);
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
