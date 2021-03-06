#include <iostream>
#include <vector>

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include <unistd.h>

#include "aes.h"


typedef struct args_struct {

	bool verbose;
	char opmode;
	AESEngine::AESMode mode;
	vector<uint8_t> key;

	FILE *infile;
	FILE *outfile;

	args_struct ()
	{
		verbose = false;
		opmode = '-';
		mode = AESEngine::AESMode::AES_128_ECB;
		key = vector<uint8_t>(AESEngine::keySize(mode));

		infile = stdin;
		outfile = stdout;
	}

} args_type;


bool parse_args (int argc, char *argv[], args_type& args)
{
	args.mode = AESEngine::AESMode::AES_128_ECB;
	args.key = vector<uint8_t>(16, 0);

	string mode = "ecb";
	int size = 128;
	string keyfilename;

	int c;
	while ((c = getopt(argc, argv, "m:s:k:i:o:v")) != -1) {
		switch (c) {
			case 'm':
				mode = optarg;
				break;
			case 's':
				size = atoi(optarg);
				break;
			case 'v':
				args.verbose = true;
				break;
			case 'i':
				fprintf(stderr, "not yet implemented\n");
				return false;
			case 'o':
				fprintf(stderr, "not yet implemented\n");
				return false;
			default:
				fprintf(stderr, "unknown arg: %c\n", c);
				return false;
		}
	}

	mode = tolowercase(mode);
	if (size == 128) {
		if (mode == "ecb") {
			args.mode = AESEngine::AESMode::AES_128_ECB;
		} else if (mode == "cbc") {
			args.mode = AESEngine::AESMode::AES_128_CBC;
		} else {
			fprintf(stderr, "invalid mode: %s\n", mode.c_str());
			return false;
		}
	} else if (size == 192) {
		if (mode == "ecb") {
			args.mode = AESEngine::AESMode::AES_192_ECB;
		} else if (mode == "cbc") {
			args.mode = AESEngine::AESMode::AES_192_CBC;
		} else {
			fprintf(stderr, "invalid mode: %s\n", mode.c_str());
			return false;
		}
	} else if (size == 256) {
		if (mode == "ecb") {
			args.mode = AESEngine::AESMode::AES_256_ECB;
		} else if (mode == "cbc") {
			args.mode = AESEngine::AESMode::AES_256_CBC;
		} else {
			fprintf(stderr, "invalid mode: %s\n", mode.c_str());
			return false;
		}
	} else {
		fprintf(stderr, "unknown size: %d\n", size);
		return false;
	}

	for (int k = optind; k < argc; ++k) {
		if (k == optind) {
			args.opmode = argv[k][0];
		} else if (k == optind + 1) {
			keyfilename = argv[k];
		} else {
			fprintf(stderr, "passed unnamed arg %s\n", argv[k]);
		}
	}

	return true;
}


vector<uint8_t> random_block ()
{
	vector<uint8_t> block(AES_BLOCK_SIZE);
	for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
		block[k] = (uint8_t)(0xff & rand());
	}
	return block;
}


int runtests ()
{
	cout << "Running unit tests ..." << endl;

	vector<uint8_t> key = AESEngine::generateKey(
		AESEngine::AESMode::AES_128_ECB);
	AESEngine engine(AESEngine::AESMode::AES_128_ECB, key);

	vector<uint8_t> block(16);
	vector<uint8_t> copy = block;

	cout << "\ttesting subBytes ... ";
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
	cout << "PASS" << endl;

	cout << "\ttesting transpose ... ";
	block = random_block();
	copy = block;
	engine.transpose(&block[0]);
	engine.transpose(&block[0]);
	for (int k = 0; k < 16; ++k)
		if (block[k] != copy[k])
			return 1;
	cout << "PASS" << endl;

	cout << "\ttesting shiftRows ... ";
	block = random_block();
	copy = block;
	engine.encryptShiftRows(&block[0]);
	engine.decryptShiftRows(&block[0]);
	for (int k = 0; k < 16; ++k)
		if (block[k] != copy[k])
			return 1;
	cout << "PASS" << endl;

	cout << "\ttesting mixColumn ... ";
	for (int k = 0; k < 8; ++k) {
		uint32_t n = rand();
		uint32_t copy = n;
		engine.mixColumn(&n);
		engine.mixColumnInv(&n);
		if (n != copy)
			return 1;
	}
	cout << "PASS" << endl;

	cout << "\ttesting mixColumns ... ";
	block = random_block();
	copy = block;
	engine.encryptMixColumns(&block[0]);
	engine.decryptMixColumns(&block[0]);
	for (int k = 0; k < 16; ++k)
		if (block[k] != copy[k])
			return 1;
	cout << "PASS" << endl;

	return 0;
}


void print_help ()
{
	printf("USAGE: aes MODE [OPTIONS] [-i inputfile] [-o outputfile]\n");
	printf("\n");
	printf("MODE\n");
	printf("\n");
	printf("\te    encryption\n");
	printf("\n");
	printf("\td    decryption\n");
	printf("\n");
	printf("\tg    generates a key (sized according to the -s option), writes it to output\n");
	printf("\n");
	printf("\th    shows this help information\n");
	printf("\n");
	printf("\n");
	printf("OPTIONS\n");
	printf("\n");
	printf("\t-s SIZE\n");
	printf("\t\tThe key size, which can be 128, 192, or 256.\n");
	printf("\t\tThe default value is 128.\n");
	printf("\n");
	printf("\t-m MODE\n");
	printf("\t\tThe AES block cipher mode. ECB or CBC.\n");
	printf("\t\tThe default value is ECB.\n");
	printf("\n");
	printf("\t-v\n");
	printf("\t\tSets verbose mode\n");
	printf("\n");
}


int main (int argc, char *argv[])
{
	args_type args;
	if (!parse_args(argc, argv, args)) {
		return EXIT_FAILURE;
	}

	AESEngine engine(args.mode, args.key);

	if (args.opmode == 'e') {
		engine.encryptFile(stdin, stdout);
	} else if (args.opmode == 'd') {
		engine.decryptFile(stdin, stdout);
	} else if (args.opmode == 'g') {
		vector<uint8_t> key = engine.generateKey();
		for (unsigned int k = 0; k < key.size(); ++k) {
			printf("%c", (char)key[k]);
		}
	} else if (args.opmode == 'h' || args.opmode == '-') {
		print_help();
	} else if (args.opmode == 't') {
		int ret = runtests();
		if (ret != 0) {
			cout << "FAIL" << endl;
			return ret;
		}
	} else {
		fprintf(stderr, "unknown mode: %c\n", args.opmode);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
