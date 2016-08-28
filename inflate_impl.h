#ifndef UUID_FF8E2E3BBF3946A7E57C25A214D47E20
#define UUID_FF8E2E3BBF3946A7E57C25A214D47E20
//#include <assert.h>
//#include <string.h>
//#include <stdlib.h>
#include "io/stream.h"
#include "inflate.h"
#include "coro.h"

#define ZFAST_BITS  9 // accelerate all cases in default tables
#define ZFAST_MASK  ((1 << ZFAST_BITS) - 1)

typedef struct zhuffman
{
	u16 fast[1 << ZFAST_BITS];
	u16 firstcode[16];
	int maxcode[17];
	u16 firstsymbol[16];
	u8  size[288];
	u16 value[288];
} zhuffman;

// States between yields

typedef struct tl_inflate_source {
	tl_inflate base;

	u8 window[32768];
	int window_pos; // = 0
	
	int num_bits;
	u32 code_buffer;

	zhuffman z_length, z_distance;

	YIELD_STATE()
	unsigned int final, type;

	// Compute huffman codes
	zhuffman z_codelength;
	u8 lencodes[286+32+137];//padding for maximum single op
	unsigned int i;
	unsigned int hlit, hdist, hclen;
	u8 codelength_sizes[19];

	// Uncompressed block
	u8 header[4];
	
	// Huffman block
	int dist;
	int z;

	// Used by uncompressed and huffman block
	int len;
} tl_inflate_source;

void tl_inf_init_(tl_inflate_source* self);

#endif // UUID_FF8E2E3BBF3946A7E57C25A214D47E20
