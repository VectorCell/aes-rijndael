# aes-rijndael

An implementation of AES (Rijndael) in C++

Usage:
```
aes MODE [OPTIONS] [-i INPUTFILE] [-o OUTPUTFILE]
```

If no input file is specified, input is read from stdin.
If no output file is specified, output is written to stdout.

```
MODE

	e    encrytion

	d    decryption

	g    generates a key (sized according to the -s option), writes it to output


OPTIONS

	-s SIZE
		The key size, which can be 128, 192, or 256.
		The default value is 128.

	-m MODE
		The AES block cipher mode. ECB or CBC.
		The default value is ECB.

	-v
		Sets verbose mode
```


Travis CI builds:

|Branch | Status |
|-------|--------|
|master | [![Build Status](https://travis-ci.org/VectorCell/aes-rijndael.svg?branch=master)](https://travis-ci.org/VectorCell/aes-rijndael?branch=master) |
|dev | [![Build Status](https://travis-ci.org/VectorCell/aes-rijndael.svg?branch=dev)](https://travis-ci.org/VectorCell/aes-rijndael?branch=dev) |
