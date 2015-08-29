#include "model.h"

#include "../std.h"

void tl_model_init_(tl_model* self, int size, int n)
{
	int i;
	mset(self, 0, size);
	for(i = 0; i < n; ++i)
	{
		self->symbols[i].freq = 1;
	}

	self->sum = n;
	self->symbols[n].freq = 0xffffffff;
	self->n = n;
}
