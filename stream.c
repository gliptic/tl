#include "stream.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

static void tl_bs_free_ud(tl_byte_source_pullable* self)
{
	free(self->ud);
}

//#define TL_BUFFER_SIZE (4096)
#define TL_BUFFER_SIZE (8192)

typedef struct tl_bs_file_data
{
	FILE* f;
	uint8_t buffer[TL_BUFFER_SIZE];
} tl_bs_file_data;

static int tl_bs_file_pull(tl_byte_source_pullable* self)
{
	tl_bs_file_data* data = (tl_bs_file_data*)self->ud;
	size_t len = fread(data->buffer, 1, TL_BUFFER_SIZE, data->f);
	self->buf = data->buffer;
	self->buf_end = data->buffer + len;
	return len > 0 ? 0 : 1;
}

static int tl_bs_file_push(tl_byte_sink_pushable* self)
{
	tl_bs_file_data* data = (tl_bs_file_data*)self->ud;
	size_t to_write = self->out - self->out_start;
	size_t len = fwrite(self->out_start, 1, to_write, data->f);
	self->out_start = self->out = data->buffer;
	self->out_end = data->buffer + TL_BUFFER_SIZE;
	return (to_write == 0 || len > 0) ? 0 : 1;
}

static void tl_bs_file_free(tl_byte_source_pullable* self)
{
	tl_bs_file_data* data = (tl_bs_file_data*)self->ud;
	if(data->f)
		fclose(data->f);
	free(data);
}

static void tl_bs_file_free_sink(tl_byte_sink_pushable* self)
{
	tl_bs_file_data* data = (tl_bs_file_data*)self->ud;
	tl_bs_file_push(self);
	if(data->f)
		fclose(data->f);
	free(data);
}

static int tl_bs_file_seek_sink(tl_byte_sink_pushable* self, uint64 p)
{
	tl_bs_file_data* data = (tl_bs_file_data*)self->ud;
	return fseek(data->f, (uint32_t)p, SEEK_SET);
}

static uint64 tl_bs_file_tell_sink(tl_byte_sink_pushable* self)
{
	tl_bs_file_data* data = (tl_bs_file_data*)self->ud;
	return ftell(data->f);
}

int tl_bs_file_source(tl_byte_source_pullable* src, char const* path)
{
	FILE* f = fopen(path, "rb");
	tl_bs_file_data* data;
	if(!f) return -1;

	tl_bs_init_source(src);
	
	data = malloc(sizeof(tl_bs_file_data));
	data->f = f;

	src->ud = data;
	src->buf = NULL;
	src->buf_end = NULL;
	src->pull = tl_bs_file_pull;
	src->free = tl_bs_file_free;
	return 0;
}

int tl_bs_file_sink(tl_byte_sink_pushable* sink, char const* path)
{
	FILE* f = fopen(path, "wb");
	tl_bs_file_data* data;
	if(!f) return -1;

	tl_bs_init_sink(sink);
	
	data = malloc(sizeof(tl_bs_file_data));
	data->f = f;

	sink->ud = data;
	sink->out = NULL;
	sink->out_end = NULL;
	sink->out_start = NULL;
	sink->push = tl_bs_file_push;
	sink->seek = tl_bs_file_seek_sink;
	sink->tell = tl_bs_file_tell_sink;
	sink->free = tl_bs_file_free_sink;
	return 0;
}

int dummy_pull(tl_byte_source_pullable* self)
{
	(void)self;
	return 1;
}

void dummy_free(tl_byte_source_pullable* self)
{
	(void)self;
	// Do nothing
}

int dummy_push(tl_byte_sink_pushable* self)
{
	(void)self;
	return 1;
}

void dummy_free_sink(tl_byte_sink_pushable* self)
{
	(void)self;
	// Do nothing
}

void tl_bs_init_source(tl_byte_source_pullable* self)
{
	self->pull = dummy_pull;
	self->free = dummy_free;
}

void tl_bs_init_sink(tl_byte_sink_pushable* self)
{
	self->push = dummy_push;
	self->free = dummy_free_sink;
}

void tl_bs_free_sink(tl_byte_sink_pushable* self)
{
	self->free(self);
}

void tl_bs_free(tl_byte_source_pullable* src)
{
	src->free(src);
}

int tl_bs_pushn(tl_byte_sink_pushable* self, uint8 const* data, int n)
{
	do
	{
		int bytes = self->out_end - self->out;

		if(bytes > n) bytes = n;
		memcpy(self->out, data, bytes);
		data += bytes;
		self->out += bytes;
		n -= bytes;
	}
	while(n > 0 && (self->push(self) == 0));

	return n == 0;
}

int tl_bs_pulln(tl_byte_source_pullable* src, uint8* data, int n)
{
	do
	{
		int bytes = src->buf_end - src->buf;

		if(bytes > n) bytes = n;
		memcpy(data, src->buf, bytes);
		data += bytes;
		src->buf += bytes;
		n -= bytes;
	}
	while(n > 0 && (src->pull(src) == 0));

	return n == 0;
}

int tl_bs_seek_sink(tl_byte_sink_pushable* self, uint64 p)
{
	tl_bs_flush(self);
	return self->seek(self, p);
}

uint64 tl_bs_tell_sink(tl_byte_sink_pushable* self)
{
	return self->tell(self) + (self->out - self->out_start);
}