#include <iostream>
#include <vector>

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include "aes.h"

using namespace std;


/*
**  AES S-Box
*/

static const uint8_t SBOX[256] = {
	0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
	0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
	0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
	0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
	0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
	0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
	0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
	0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
	0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
	0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
	0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
	0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
	0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
	0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
	0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
	0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};
static const uint8_t *inverseLookupTable (const uint8_t table[256])
{
	static uint8_t reverse[256];
	for (int k = 0; k < 256; ++k)
		reverse[table[k]] = k;
	return reverse;
}
static const uint8_t *SBOX_INV = inverseLookupTable(SBOX);

static const uint8_t RCON[256] = {
	0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A,
	0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39,
	0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A,
	0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8,
	0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF,
	0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC,
	0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B,
	0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3,
	0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94,
	0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
	0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35,
	0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F,
	0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D, 0x01, 0x02, 0x04,
	0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F, 0x5E, 0xBC, 0x63,
	0xC6, 0x97, 0x35, 0x6A, 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39, 0x72, 0xE4, 0xD3, 0xBD,
	0x61, 0xC2, 0x9F, 0x25, 0x4A, 0x94, 0x33, 0x66, 0xCC, 0x83, 0x1D, 0x3A, 0x74, 0xE8, 0xCB, 0x8D
};


/*
**  Precomputed multiplication in Galois field
*/

static const uint8_t GMUL_2[] = {
	0X00, 0X02, 0X04, 0X06, 0X08, 0X0A, 0X0C, 0X0E, 0X10, 0X12, 0X14, 0X16, 0X18, 0X1A, 0X1C, 0X1E,
	0X20, 0X22, 0X24, 0X26, 0X28, 0X2A, 0X2C, 0X2E, 0X30, 0X32, 0X34, 0X36, 0X38, 0X3A, 0X3C, 0X3E,
	0X40, 0X42, 0X44, 0X46, 0X48, 0X4A, 0X4C, 0X4E, 0X50, 0X52, 0X54, 0X56, 0X58, 0X5A, 0X5C, 0X5E,
	0X60, 0X62, 0X64, 0X66, 0X68, 0X6A, 0X6C, 0X6E, 0X70, 0X72, 0X74, 0X76, 0X78, 0X7A, 0X7C, 0X7E,
	0X80, 0X82, 0X84, 0X86, 0X88, 0X8A, 0X8C, 0X8E, 0X90, 0X92, 0X94, 0X96, 0X98, 0X9A, 0X9C, 0X9E,
	0XA0, 0XA2, 0XA4, 0XA6, 0XA8, 0XAA, 0XAC, 0XAE, 0XB0, 0XB2, 0XB4, 0XB6, 0XB8, 0XBA, 0XBC, 0XBE,
	0XC0, 0XC2, 0XC4, 0XC6, 0XC8, 0XCA, 0XCC, 0XCE, 0XD0, 0XD2, 0XD4, 0XD6, 0XD8, 0XDA, 0XDC, 0XDE,
	0XE0, 0XE2, 0XE4, 0XE6, 0XE8, 0XEA, 0XEC, 0XEE, 0XF0, 0XF2, 0XF4, 0XF6, 0XF8, 0XFA, 0XFC, 0XFE,
	0X1B, 0X19, 0X1F, 0X1D, 0X13, 0X11, 0X17, 0X15, 0X0B, 0X09, 0X0F, 0X0D, 0X03, 0X01, 0X07, 0X05,
	0X3B, 0X39, 0X3F, 0X3D, 0X33, 0X31, 0X37, 0X35, 0X2B, 0X29, 0X2F, 0X2D, 0X23, 0X21, 0X27, 0X25,
	0X5B, 0X59, 0X5F, 0X5D, 0X53, 0X51, 0X57, 0X55, 0X4B, 0X49, 0X4F, 0X4D, 0X43, 0X41, 0X47, 0X45,
	0X7B, 0X79, 0X7F, 0X7D, 0X73, 0X71, 0X77, 0X75, 0X6B, 0X69, 0X6F, 0X6D, 0X63, 0X61, 0X67, 0X65,
	0X9B, 0X99, 0X9F, 0X9D, 0X93, 0X91, 0X97, 0X95, 0X8B, 0X89, 0X8F, 0X8D, 0X83, 0X81, 0X87, 0X85,
	0XBB, 0XB9, 0XBF, 0XBD, 0XB3, 0XB1, 0XB7, 0XB5, 0XAB, 0XA9, 0XAF, 0XAD, 0XA3, 0XA1, 0XA7, 0XA5,
	0XDB, 0XD9, 0XDF, 0XDD, 0XD3, 0XD1, 0XD7, 0XD5, 0XCB, 0XC9, 0XCF, 0XCD, 0XC3, 0XC1, 0XC7, 0XC5,
	0XFB, 0XF9, 0XFF, 0XFD, 0XF3, 0XF1, 0XF7, 0XF5, 0XEB, 0XE9, 0XEF, 0XED, 0XE3, 0XE1, 0XE7, 0XE5
};

static const uint8_t GMUL_3[] = {
	0X00, 0X03, 0X06, 0X05, 0X0C, 0X0F, 0X0A, 0X09, 0X18, 0X1B, 0X1E, 0X1D, 0X14, 0X17, 0X12, 0X11,
	0X30, 0X33, 0X36, 0X35, 0X3C, 0X3F, 0X3A, 0X39, 0X28, 0X2B, 0X2E, 0X2D, 0X24, 0X27, 0X22, 0X21,
	0X60, 0X63, 0X66, 0X65, 0X6C, 0X6F, 0X6A, 0X69, 0X78, 0X7B, 0X7E, 0X7D, 0X74, 0X77, 0X72, 0X71,
	0X50, 0X53, 0X56, 0X55, 0X5C, 0X5F, 0X5A, 0X59, 0X48, 0X4B, 0X4E, 0X4D, 0X44, 0X47, 0X42, 0X41,
	0XC0, 0XC3, 0XC6, 0XC5, 0XCC, 0XCF, 0XCA, 0XC9, 0XD8, 0XDB, 0XDE, 0XDD, 0XD4, 0XD7, 0XD2, 0XD1,
	0XF0, 0XF3, 0XF6, 0XF5, 0XFC, 0XFF, 0XFA, 0XF9, 0XE8, 0XEB, 0XEE, 0XED, 0XE4, 0XE7, 0XE2, 0XE1,
	0XA0, 0XA3, 0XA6, 0XA5, 0XAC, 0XAF, 0XAA, 0XA9, 0XB8, 0XBB, 0XBE, 0XBD, 0XB4, 0XB7, 0XB2, 0XB1,
	0X90, 0X93, 0X96, 0X95, 0X9C, 0X9F, 0X9A, 0X99, 0X88, 0X8B, 0X8E, 0X8D, 0X84, 0X87, 0X82, 0X81,
	0X9B, 0X98, 0X9D, 0X9E, 0X97, 0X94, 0X91, 0X92, 0X83, 0X80, 0X85, 0X86, 0X8F, 0X8C, 0X89, 0X8A,
	0XAB, 0XA8, 0XAD, 0XAE, 0XA7, 0XA4, 0XA1, 0XA2, 0XB3, 0XB0, 0XB5, 0XB6, 0XBF, 0XBC, 0XB9, 0XBA,
	0XFB, 0XF8, 0XFD, 0XFE, 0XF7, 0XF4, 0XF1, 0XF2, 0XE3, 0XE0, 0XE5, 0XE6, 0XEF, 0XEC, 0XE9, 0XEA,
	0XCB, 0XC8, 0XCD, 0XCE, 0XC7, 0XC4, 0XC1, 0XC2, 0XD3, 0XD0, 0XD5, 0XD6, 0XDF, 0XDC, 0XD9, 0XDA,
	0X5B, 0X58, 0X5D, 0X5E, 0X57, 0X54, 0X51, 0X52, 0X43, 0X40, 0X45, 0X46, 0X4F, 0X4C, 0X49, 0X4A,
	0X6B, 0X68, 0X6D, 0X6E, 0X67, 0X64, 0X61, 0X62, 0X73, 0X70, 0X75, 0X76, 0X7F, 0X7C, 0X79, 0X7A,
	0X3B, 0X38, 0X3D, 0X3E, 0X37, 0X34, 0X31, 0X32, 0X23, 0X20, 0X25, 0X26, 0X2F, 0X2C, 0X29, 0X2A,
	0X0B, 0X08, 0X0D, 0X0E, 0X07, 0X04, 0X01, 0X02, 0X13, 0X10, 0X15, 0X16, 0X1F, 0X1C, 0X19, 0X1A
};

static const uint8_t GMUL_9[] = {
	0X00, 0X09, 0X12, 0X1B, 0X24, 0X2D, 0X36, 0X3F, 0X48, 0X41, 0X5A, 0X53, 0X6C, 0X65, 0X7E, 0X77,
	0X90, 0X99, 0X82, 0X8B, 0XB4, 0XBD, 0XA6, 0XAF, 0XD8, 0XD1, 0XCA, 0XC3, 0XFC, 0XF5, 0XEE, 0XE7,
	0X3B, 0X32, 0X29, 0X20, 0X1F, 0X16, 0X0D, 0X04, 0X73, 0X7A, 0X61, 0X68, 0X57, 0X5E, 0X45, 0X4C,
	0XAB, 0XA2, 0XB9, 0XB0, 0X8F, 0X86, 0X9D, 0X94, 0XE3, 0XEA, 0XF1, 0XF8, 0XC7, 0XCE, 0XD5, 0XDC,
	0X76, 0X7F, 0X64, 0X6D, 0X52, 0X5B, 0X40, 0X49, 0X3E, 0X37, 0X2C, 0X25, 0X1A, 0X13, 0X08, 0X01,
	0XE6, 0XEF, 0XF4, 0XFD, 0XC2, 0XCB, 0XD0, 0XD9, 0XAE, 0XA7, 0XBC, 0XB5, 0X8A, 0X83, 0X98, 0X91,
	0X4D, 0X44, 0X5F, 0X56, 0X69, 0X60, 0X7B, 0X72, 0X05, 0X0C, 0X17, 0X1E, 0X21, 0X28, 0X33, 0X3A,
	0XDD, 0XD4, 0XCF, 0XC6, 0XF9, 0XF0, 0XEB, 0XE2, 0X95, 0X9C, 0X87, 0X8E, 0XB1, 0XB8, 0XA3, 0XAA,
	0XEC, 0XE5, 0XFE, 0XF7, 0XC8, 0XC1, 0XDA, 0XD3, 0XA4, 0XAD, 0XB6, 0XBF, 0X80, 0X89, 0X92, 0X9B,
	0X7C, 0X75, 0X6E, 0X67, 0X58, 0X51, 0X4A, 0X43, 0X34, 0X3D, 0X26, 0X2F, 0X10, 0X19, 0X02, 0X0B,
	0XD7, 0XDE, 0XC5, 0XCC, 0XF3, 0XFA, 0XE1, 0XE8, 0X9F, 0X96, 0X8D, 0X84, 0XBB, 0XB2, 0XA9, 0XA0,
	0X47, 0X4E, 0X55, 0X5C, 0X63, 0X6A, 0X71, 0X78, 0X0F, 0X06, 0X1D, 0X14, 0X2B, 0X22, 0X39, 0X30,
	0X9A, 0X93, 0X88, 0X81, 0XBE, 0XB7, 0XAC, 0XA5, 0XD2, 0XDB, 0XC0, 0XC9, 0XF6, 0XFF, 0XE4, 0XED,
	0X0A, 0X03, 0X18, 0X11, 0X2E, 0X27, 0X3C, 0X35, 0X42, 0X4B, 0X50, 0X59, 0X66, 0X6F, 0X74, 0X7D,
	0XA1, 0XA8, 0XB3, 0XBA, 0X85, 0X8C, 0X97, 0X9E, 0XE9, 0XE0, 0XFB, 0XF2, 0XCD, 0XC4, 0XDF, 0XD6,
	0X31, 0X38, 0X23, 0X2A, 0X15, 0X1C, 0X07, 0X0E, 0X79, 0X70, 0X6B, 0X62, 0X5D, 0X54, 0X4F, 0X46
};

static const uint8_t GMUL_B[] = {
	0X00, 0X0B, 0X16, 0X1D, 0X2C, 0X27, 0X3A, 0X31, 0X58, 0X53, 0X4E, 0X45, 0X74, 0X7F, 0X62, 0X69,
	0XB0, 0XBB, 0XA6, 0XAD, 0X9C, 0X97, 0X8A, 0X81, 0XE8, 0XE3, 0XFE, 0XF5, 0XC4, 0XCF, 0XD2, 0XD9,
	0X7B, 0X70, 0X6D, 0X66, 0X57, 0X5C, 0X41, 0X4A, 0X23, 0X28, 0X35, 0X3E, 0X0F, 0X04, 0X19, 0X12,
	0XCB, 0XC0, 0XDD, 0XD6, 0XE7, 0XEC, 0XF1, 0XFA, 0X93, 0X98, 0X85, 0X8E, 0XBF, 0XB4, 0XA9, 0XA2,
	0XF6, 0XFD, 0XE0, 0XEB, 0XDA, 0XD1, 0XCC, 0XC7, 0XAE, 0XA5, 0XB8, 0XB3, 0X82, 0X89, 0X94, 0X9F,
	0X46, 0X4D, 0X50, 0X5B, 0X6A, 0X61, 0X7C, 0X77, 0X1E, 0X15, 0X08, 0X03, 0X32, 0X39, 0X24, 0X2F,
	0X8D, 0X86, 0X9B, 0X90, 0XA1, 0XAA, 0XB7, 0XBC, 0XD5, 0XDE, 0XC3, 0XC8, 0XF9, 0XF2, 0XEF, 0XE4,
	0X3D, 0X36, 0X2B, 0X20, 0X11, 0X1A, 0X07, 0X0C, 0X65, 0X6E, 0X73, 0X78, 0X49, 0X42, 0X5F, 0X54,
	0XF7, 0XFC, 0XE1, 0XEA, 0XDB, 0XD0, 0XCD, 0XC6, 0XAF, 0XA4, 0XB9, 0XB2, 0X83, 0X88, 0X95, 0X9E,
	0X47, 0X4C, 0X51, 0X5A, 0X6B, 0X60, 0X7D, 0X76, 0X1F, 0X14, 0X09, 0X02, 0X33, 0X38, 0X25, 0X2E,
	0X8C, 0X87, 0X9A, 0X91, 0XA0, 0XAB, 0XB6, 0XBD, 0XD4, 0XDF, 0XC2, 0XC9, 0XF8, 0XF3, 0XEE, 0XE5,
	0X3C, 0X37, 0X2A, 0X21, 0X10, 0X1B, 0X06, 0X0D, 0X64, 0X6F, 0X72, 0X79, 0X48, 0X43, 0X5E, 0X55,
	0X01, 0X0A, 0X17, 0X1C, 0X2D, 0X26, 0X3B, 0X30, 0X59, 0X52, 0X4F, 0X44, 0X75, 0X7E, 0X63, 0X68,
	0XB1, 0XBA, 0XA7, 0XAC, 0X9D, 0X96, 0X8B, 0X80, 0XE9, 0XE2, 0XFF, 0XF4, 0XC5, 0XCE, 0XD3, 0XD8,
	0X7A, 0X71, 0X6C, 0X67, 0X56, 0X5D, 0X40, 0X4B, 0X22, 0X29, 0X34, 0X3F, 0X0E, 0X05, 0X18, 0X13,
	0XCA, 0XC1, 0XDC, 0XD7, 0XE6, 0XED, 0XF0, 0XFB, 0X92, 0X99, 0X84, 0X8F, 0XBE, 0XB5, 0XA8, 0XA3
};

static const uint8_t GMUL_D[] = {
	0X00, 0X0D, 0X1A, 0X17, 0X34, 0X39, 0X2E, 0X23, 0X68, 0X65, 0X72, 0X7F, 0X5C, 0X51, 0X46, 0X4B,
	0XD0, 0XDD, 0XCA, 0XC7, 0XE4, 0XE9, 0XFE, 0XF3, 0XB8, 0XB5, 0XA2, 0XAF, 0X8C, 0X81, 0X96, 0X9B,
	0XBB, 0XB6, 0XA1, 0XAC, 0X8F, 0X82, 0X95, 0X98, 0XD3, 0XDE, 0XC9, 0XC4, 0XE7, 0XEA, 0XFD, 0XF0,
	0X6B, 0X66, 0X71, 0X7C, 0X5F, 0X52, 0X45, 0X48, 0X03, 0X0E, 0X19, 0X14, 0X37, 0X3A, 0X2D, 0X20,
	0X6D, 0X60, 0X77, 0X7A, 0X59, 0X54, 0X43, 0X4E, 0X05, 0X08, 0X1F, 0X12, 0X31, 0X3C, 0X2B, 0X26,
	0XBD, 0XB0, 0XA7, 0XAA, 0X89, 0X84, 0X93, 0X9E, 0XD5, 0XD8, 0XCF, 0XC2, 0XE1, 0XEC, 0XFB, 0XF6,
	0XD6, 0XDB, 0XCC, 0XC1, 0XE2, 0XEF, 0XF8, 0XF5, 0XBE, 0XB3, 0XA4, 0XA9, 0X8A, 0X87, 0X90, 0X9D,
	0X06, 0X0B, 0X1C, 0X11, 0X32, 0X3F, 0X28, 0X25, 0X6E, 0X63, 0X74, 0X79, 0X5A, 0X57, 0X40, 0X4D,
	0XDA, 0XD7, 0XC0, 0XCD, 0XEE, 0XE3, 0XF4, 0XF9, 0XB2, 0XBF, 0XA8, 0XA5, 0X86, 0X8B, 0X9C, 0X91,
	0X0A, 0X07, 0X10, 0X1D, 0X3E, 0X33, 0X24, 0X29, 0X62, 0X6F, 0X78, 0X75, 0X56, 0X5B, 0X4C, 0X41,
	0X61, 0X6C, 0X7B, 0X76, 0X55, 0X58, 0X4F, 0X42, 0X09, 0X04, 0X13, 0X1E, 0X3D, 0X30, 0X27, 0X2A,
	0XB1, 0XBC, 0XAB, 0XA6, 0X85, 0X88, 0X9F, 0X92, 0XD9, 0XD4, 0XC3, 0XCE, 0XED, 0XE0, 0XF7, 0XFA,
	0XB7, 0XBA, 0XAD, 0XA0, 0X83, 0X8E, 0X99, 0X94, 0XDF, 0XD2, 0XC5, 0XC8, 0XEB, 0XE6, 0XF1, 0XFC,
	0X67, 0X6A, 0X7D, 0X70, 0X53, 0X5E, 0X49, 0X44, 0X0F, 0X02, 0X15, 0X18, 0X3B, 0X36, 0X21, 0X2C,
	0X0C, 0X01, 0X16, 0X1B, 0X38, 0X35, 0X22, 0X2F, 0X64, 0X69, 0X7E, 0X73, 0X50, 0X5D, 0X4A, 0X47,
	0XDC, 0XD1, 0XC6, 0XCB, 0XE8, 0XE5, 0XF2, 0XFF, 0XB4, 0XB9, 0XAE, 0XA3, 0X80, 0X8D, 0X9A, 0X97
};

static const uint8_t GMUL_E[] = {
	0X00, 0X0E, 0X1C, 0X12, 0X38, 0X36, 0X24, 0X2A, 0X70, 0X7E, 0X6C, 0X62, 0X48, 0X46, 0X54, 0X5A,
	0XE0, 0XEE, 0XFC, 0XF2, 0XD8, 0XD6, 0XC4, 0XCA, 0X90, 0X9E, 0X8C, 0X82, 0XA8, 0XA6, 0XB4, 0XBA,
	0XDB, 0XD5, 0XC7, 0XC9, 0XE3, 0XED, 0XFF, 0XF1, 0XAB, 0XA5, 0XB7, 0XB9, 0X93, 0X9D, 0X8F, 0X81,
	0X3B, 0X35, 0X27, 0X29, 0X03, 0X0D, 0X1F, 0X11, 0X4B, 0X45, 0X57, 0X59, 0X73, 0X7D, 0X6F, 0X61,
	0XAD, 0XA3, 0XB1, 0XBF, 0X95, 0X9B, 0X89, 0X87, 0XDD, 0XD3, 0XC1, 0XCF, 0XE5, 0XEB, 0XF9, 0XF7,
	0X4D, 0X43, 0X51, 0X5F, 0X75, 0X7B, 0X69, 0X67, 0X3D, 0X33, 0X21, 0X2F, 0X05, 0X0B, 0X19, 0X17,
	0X76, 0X78, 0X6A, 0X64, 0X4E, 0X40, 0X52, 0X5C, 0X06, 0X08, 0X1A, 0X14, 0X3E, 0X30, 0X22, 0X2C,
	0X96, 0X98, 0X8A, 0X84, 0XAE, 0XA0, 0XB2, 0XBC, 0XE6, 0XE8, 0XFA, 0XF4, 0XDE, 0XD0, 0XC2, 0XCC,
	0X41, 0X4F, 0X5D, 0X53, 0X79, 0X77, 0X65, 0X6B, 0X31, 0X3F, 0X2D, 0X23, 0X09, 0X07, 0X15, 0X1B,
	0XA1, 0XAF, 0XBD, 0XB3, 0X99, 0X97, 0X85, 0X8B, 0XD1, 0XDF, 0XCD, 0XC3, 0XE9, 0XE7, 0XF5, 0XFB,
	0X9A, 0X94, 0X86, 0X88, 0XA2, 0XAC, 0XBE, 0XB0, 0XEA, 0XE4, 0XF6, 0XF8, 0XD2, 0XDC, 0XCE, 0XC0,
	0X7A, 0X74, 0X66, 0X68, 0X42, 0X4C, 0X5E, 0X50, 0X0A, 0X04, 0X16, 0X18, 0X32, 0X3C, 0X2E, 0X20,
	0XEC, 0XE2, 0XF0, 0XFE, 0XD4, 0XDA, 0XC8, 0XC6, 0X9C, 0X92, 0X80, 0X8E, 0XA4, 0XAA, 0XB8, 0XB6,
	0X0C, 0X02, 0X10, 0X1E, 0X34, 0X3A, 0X28, 0X26, 0X7C, 0X72, 0X60, 0X6E, 0X44, 0X4A, 0X58, 0X56,
	0X37, 0X39, 0X2B, 0X25, 0X0F, 0X01, 0X13, 0X1D, 0X47, 0X49, 0X5B, 0X55, 0X7F, 0X71, 0X63, 0X6D,
	0XD7, 0XD9, 0XCB, 0XC5, 0XEF, 0XE1, 0XF3, 0XFD, 0XA7, 0XA9, 0XBB, 0XB5, 0X9F, 0X91, 0X83, 0X8D
};


AESEngine::AESEngine (const AESMode m, const vector<uint8_t>& k)
	: mode(m), key(k)
{
	while (key.size() < keySize()) {
		key.push_back(0);
	}
	if (key.size() > keySize()) {
		key.resize(keySize());
	}

	key = vector<uint8_t>(k.size());
	copy(k.begin(), k.end(), key.begin());

	nrounds = key.size() / 4 + 6;
	schedule = keyExpansion();

	prev = vector<uint8_t>(AES_BLOCK_SIZE);
}


AESEngine::~AESEngine ()
{
	fill(key.begin(), key.end(), 0);
	fill(prev.begin(), prev.end(), 0);
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


//                                               m   
//    mmm   m mm    mmm    m mm  m   m  mmmm   mm#mm 
//   #"  #  #"  #  #"  "   #"  " "m m"  #" "#    #   
//   #""""  #   #  #       #      #m#   #   #    #   
//   "#mm"  #   #  "#mm"   #      "#    ##m#"    "mm 
//                                m"    #            
//                               ""     "            


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


void AESEngine::encryptFile (FILE *infile, FILE *outfile)
{
	vector<uint8_t> bufferA(AES_BLOCK_SIZE);
	uint8_t *buf = (uint8_t *)&bufferA[0];
	size_t count = 0;
	bool stopped_at_bounds = true;
	while ((count = fread(buf, 1, AES_BLOCK_SIZE, infile)) > 0) {

		if (count < AES_BLOCK_SIZE) {
			stopped_at_bounds = false;
			uint8_t val = (uint8_t)(AES_BLOCK_SIZE - count);
			while (count < AES_BLOCK_SIZE) {
				bufferA[count] = val;
				++count;
			}
		}

		encryptBlock(buf);
		fwrite(buf, 1, AES_BLOCK_SIZE, outfile);
	}
	if (stopped_at_bounds) {
		for (int k = 0; k < AES_BLOCK_SIZE; ++k) {
			buf[k] = 0x10;
		}
		encryptBlock(buf);
		fwrite(buf, 1, AES_BLOCK_SIZE, outfile);
	}
}


//       #                                       m   
//    mmm#   mmm    mmm    m mm  m   m  mmmm   mm#mm 
//   #" "#  #"  #  #"  "   #"  " "m m"  #" "#    #   
//   #   #  #""""  #       #      #m#   #   #    #   
//   "#m##  "#mm"  "#mm"   #      "#    ##m#"    "mm 
//                                m"    #            
//                               ""     "            


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


void AESEngine::decryptFile (FILE *infile, FILE *outfile)
{
	vector<uint8_t> bufferA(AES_BLOCK_SIZE);
	vector<uint8_t> bufferB(AES_BLOCK_SIZE);
	vector<uint8_t> bufferC(AES_BLOCK_SIZE);
	uint8_t *newest = (uint8_t *)&bufferA[0];
	uint8_t *next   = (uint8_t *)&bufferB[0];
	uint8_t *oldest = (uint8_t *)&bufferC[0];
	size_t nblocks = 0;
	size_t count = 0;
	while ((count = fread(newest, 1, AES_BLOCK_SIZE, infile)) > 0) {
		if (count != AES_BLOCK_SIZE) {
			throw IllegalAESBlockSize();
		}

		++nblocks;

		if (nblocks >= 2) {
			decryptBlock(oldest);
			fwrite(oldest, 1, AES_BLOCK_SIZE, outfile);
		}

		uint8_t *temp = next;
		next = newest;
		oldest = next;
		newest = temp;
	}

	decryptBlock(next);
	uint8_t padding = next[AES_BLOCK_SIZE - 1];
	if (padding < 0x10) {
		fwrite(next, 1, AES_BLOCK_SIZE - padding, outfile);
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
	while ((uint8_t *)blk < (block + AES_BLOCK_SIZE)) {
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
	uint_fast32_t *blk = (uint_fast32_t *)block;
	uint_fast32_t *pr  = (uint_fast32_t *)prev;
	while ((uint8_t *)blk < (block + AES_BLOCK_SIZE)) {
		*blk ^= *pr;
		*pr = *blk;
		++blk;
		++pr;
	}
}


void AESEngine::decryptCBC (uint8_t *block, uint8_t *prev)
{
	uint_fast32_t temp;
	uint_fast32_t *blk = (uint_fast32_t *)block;
	uint_fast32_t *pr = (uint_fast32_t *)prev;
	while ((uint8_t *)blk < (block + AES_BLOCK_SIZE)) {
		temp = *blk;
		*blk ^= *pr;
		*pr = temp;
		++blk;
		++pr;
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


vector<uint8_t> AESEngine::loadKey (const char* filename, AESMode m)
{
	FILE *kf = fopen(filename, "rb");
	vector<uint8_t> key(keySize(m));
	if (kf != NULL) {
		size_t count = fread(&key[0], 1, key.size(), kf);
		if (count < key.size()) {

		}
		fclose(kf);
	}
	return key;
}


vector<uint8_t> AESEngine::generateKey ()
{
	return generateKey(mode);
}


vector<uint8_t> AESEngine::generateKey (AESMode m)
{
	vector<uint8_t> key(keySize(m));
	FILE *random = fopen("/dev/urandom", "rb");
	if (random == NULL)
		throw KeyGenerationException("unable to open /dev/urandom");
	if (random != NULL) {
		size_t count = fread(&key[0], 1, key.size(), random);
		if (count != key.size())
			throw KeyGenerationException("unable to create enough entropy");
	}
	return key;
}


bool AESEngine::isModeECB ()
{
	return isModeECB(mode);
}


bool AESEngine::isModeECB (AESMode mode)
{
	return mode <= AESMode::AES_256_ECB;
}


bool AESEngine::isModeCBC ()
{
	return isModeCBC(mode);
}


bool AESEngine::isModeCBC (AESMode mode)
{
	return !isModeECB(mode) && (mode <= AESMode::AES_256_CBC);
}


size_t AESEngine::keySize ()
{
	return keySize(mode);
}


size_t AESEngine::keySize (AESMode mode)
{
	int m = mode & 0x3;
	switch (m) {
		case 0:
			return 16;
		case 1:
			return 24;
		case 2:
			return 32;
		default:
			throw IllegalAESMode();
	}
}
