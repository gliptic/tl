#include "test_codec.h"
#include "bitstream.h"
#include "polar_model.h"
#include "../rand.h"

//#include <stdlib.h>

void test_codec()
{
	int i;
	tl_bitsink bs;

	tl_def_polar_model(256, model);
	tl_model_init(model, 256);

	tl_bitsink_init(&bs);
	tl_bs_file_sink(&bs.sink, "test.bin");

	tl_xorshift xs = { 1 };
	
	for(i = 0; i < 8192; ++i)
	{
		//tl_bitsink_putbits(&bs, i, 7);
		int sym = (tl_xs_gen(&xs) & 0xf);
		//sym = (sym * sym) >> 16;

		tl_polar_model_update(model);
		tl_model_write(model, &bs, sym);
		tl_model_incr(model, sym);
	}

	tl_bitsink_flushbytes(&bs);
	tl_bitsink_deinit(&bs);
}
