/**
 * FreeRDP: A Remote Desktop Protocol Client
 * RemoteFX Codec Library Unit Tests
 *
 * Copyright 2011 Vic Lee
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * The sample data comes from [MS-RDPRFX] 4.2.3, which is decoded into three
 * vertical bands in red (21x64), green (23x64) and blue(20x64) color.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freerdp/types.h>
#include <freerdp/utils/print.h>
#include <freerdp/utils/memory.h>
#include <freerdp/utils/hexdump.h>
#include <freerdp/codec/rfx.h>
#include "rfx_types.h"
#include "rfx_bitstream.h"
#include "rfx_rlgr.h"
#include "rfx_differential.h"
#include "rfx_quantization.h"
#include "rfx_dwt.h"
#include "rfx_decode.h"
#include "rfx_encode.h"

#include "test_rfx.h"

static const uint8 y_data[] =
{
	                  0x19, 0x82, 0x1d, 0x10, 0x62, 0x9d, 0x28, 0x85, 0x2c, 0xa2, 0x14, 0xb2, 0x88,
	0x52, 0xca, 0x21, 0x4b, 0x28, 0x85, 0x2c, 0xa2, 0x14, 0xb2, 0x88, 0x52, 0xca, 0x21, 0x4b, 0x28,
	0x85, 0x2c, 0xa2, 0x14, 0xb2, 0x88, 0x52, 0xca, 0x21, 0x4b, 0x28, 0x85, 0x2c, 0xa2, 0x14, 0xb2,
	0x88, 0x52, 0xca, 0x21, 0x4b, 0x28, 0x85, 0x2c, 0xa2, 0x14, 0xb2, 0x88, 0x52, 0xca, 0x21, 0x4b,
	0x28, 0x85, 0x2c, 0xa2, 0x14, 0xb2, 0x88, 0x52, 0xca, 0x21, 0x4b, 0x28, 0x85, 0x2c, 0xa2, 0x14,
	0xb2, 0x88, 0x52, 0xca, 0x21, 0x4b, 0x28, 0x85, 0x2c, 0xa2, 0x14, 0xb0, 0x00, 0x20, 0xf4, 0x40,
	0x0c, 0xc1, 0x1e, 0x20, 0x26, 0x22, 0x20, 0x33, 0x23, 0xc4, 0x23, 0x88, 0x86, 0x50, 0xf1, 0x22,
	0x68, 0x4c, 0x91, 0x85, 0x10, 0x34, 0x4c, 0x84, 0x78, 0xa2, 0x0d, 0x13, 0x21, 0x1e, 0x29, 0x06,
	0x89, 0x90, 0x8f, 0x14, 0x83, 0x44, 0xf4, 0x23, 0xc5, 0x20, 0xd1, 0x3d, 0x08, 0xf1, 0x48, 0x34,
	0x4f, 0x42, 0x3c, 0x52, 0x0d, 0x13, 0xd0, 0x8f, 0x14, 0x83, 0x44, 0xf4, 0x23, 0xc5, 0x20, 0xd1,
	0x3d, 0x08, 0xf1, 0x48, 0x34, 0x4f, 0x42, 0x3c, 0x52, 0x0d, 0x13, 0xd0, 0x8f, 0x14, 0x83, 0x44,
	0xf4, 0x23, 0xc5, 0x20, 0x00, 0x08, 0x47, 0x70, 0x15, 0x02, 0xe0, 0x7f, 0xe4, 0x9d, 0xc2, 0x51,
	0x71, 0xf4, 0x99, 0xc9, 0x57, 0xff, 0x32, 0x87, 0x9d, 0x17, 0xd6, 0x50, 0x6e, 0x06, 0x2f, 0xac,
	0xa0, 0x9c, 0x0c, 0x5f, 0x59, 0x41, 0x38, 0x18, 0xbe, 0xb2, 0x82, 0x70, 0x31, 0x7d, 0x65, 0x00,
	0x00, 0x10, 0xff, 0x9c, 0x33, 0x41, 0xf1, 0xc4, 0xb0, 0x3c, 0xff, 0xa2, 0x15, 0xbd, 0x7b, 0xea,
	0x86, 0x9b, 0x5f, 0xfc, 0x78, 0x8c, 0xf5, 0xed, 0xa8, 0x68, 0xda, 0xfd, 0x3c, 0x45, 0x7a, 0xf4,
	0xd4, 0x34, 0x6d, 0x7e, 0x9e, 0x22, 0xbd, 0x7a, 0x6a, 0x1a, 0x36, 0xbf, 0x4f, 0x11, 0x5e, 0xbd,
	0x35, 0x0d, 0x1b, 0x5f, 0xa7, 0x88, 0xaf, 0x5e, 0x9a, 0x86, 0x8d, 0xaf, 0xd3, 0xc4, 0x57, 0xaf,
	0x4d, 0x43, 0x46, 0xd7, 0xe9, 0xe2, 0x20, 0x30, 0x00
};

static const uint8 cb_data[] =
{
	                                                      0x1b, 0x04, 0x7f, 0x04, 0x31, 0x5f, 0xc2,
	0x94, 0xaf, 0x05, 0x29, 0x5e, 0x0a, 0x52, 0xbc, 0x14, 0xa5, 0x78, 0x29, 0x25, 0x78, 0x29, 0x25,
	0x78, 0x29, 0x25, 0x68, 0x52, 0x4a, 0xf0, 0x52, 0x4a, 0xf0, 0x52, 0x4a, 0xd0, 0xa4, 0x95, 0xe0,
	0xa4, 0x95, 0xe0, 0xa4, 0x95, 0xa1, 0x49, 0x2b, 0xc1, 0x49, 0x2b, 0xc1, 0x49, 0x2b, 0x42, 0x92,
	0x57, 0x82, 0x92, 0x57, 0x82, 0x92, 0x56, 0x85, 0x24, 0xaf, 0x05, 0x24, 0xaf, 0x05, 0x24, 0xad,
	0x0a, 0x49, 0x5e, 0x0a, 0x49, 0x5e, 0x0a, 0x49, 0x5a, 0x14, 0x92, 0xbc, 0x14, 0x92, 0xbc, 0x14,
	0x92, 0xb4, 0x29, 0x25, 0x78, 0x29, 0x25, 0x78, 0x00, 0x02, 0x0f, 0x02, 0x00, 0xac, 0x13, 0xfc,
	0xc0, 0x0a, 0x20, 0x10, 0x2b, 0x27, 0xf9, 0x80, 0xb0, 0x08, 0xaa, 0x3d, 0x60, 0x8c, 0x0b, 0x24,
	0xff, 0x30, 0x80, 0xc0, 0xaa, 0x13, 0xfc, 0xc2, 0x03, 0x05, 0x90, 0x9f, 0xe6, 0x10, 0x18, 0x2c,
	0x84, 0xff, 0x30, 0x81, 0x82, 0xc8, 0x4f, 0xf3, 0x08, 0x18, 0x2c, 0x84, 0xff, 0x31, 0x03, 0x05,
	0x90, 0x9f, 0xff, 0xd8, 0x40, 0x60, 0x59, 0x09, 0xfe, 0x61, 0x01, 0x81, 0x64, 0x27, 0xf9, 0x84,
	0x06, 0x0b, 0x21, 0x3f, 0xcc, 0x20, 0x30, 0x59, 0x09, 0xfe, 0x61, 0x03, 0x05, 0x90, 0x9f, 0xe6,
	0x10, 0x30, 0x59, 0x09, 0xfe, 0x62, 0x00, 0x00, 0x42, 0x15, 0x00, 0x10, 0x15, 0x01, 0xfe, 0x20,
	0x84, 0xd5, 0x01, 0x0a, 0x8f, 0xf1, 0x40, 0x33, 0x78, 0x17, 0xf9, 0xc2, 0x03, 0x83, 0x01, 0x78,
	0xe1, 0x01, 0xc1, 0x00, 0xbc, 0x70, 0x80, 0xe0, 0x80, 0x5e, 0x38, 0x40, 0x70, 0x40, 0x2f, 0x1c,
	0x20, 0x38, 0x20, 0x17, 0x8e, 0x10, 0x00, 0x00, 0x87, 0xd5, 0x08, 0x70, 0xef, 0x81, 0xa2, 0xd8,
	0xff, 0xff, 0xff, 0xfb, 0xd1, 0x2d, 0x4e, 0xa6, 0xce, 0x20, 0xa4, 0xef, 0x05, 0x78, 0x35, 0x3a,
	0x9b, 0x38, 0x82, 0x93, 0xbc, 0x15, 0xe0, 0xd4, 0xea, 0x66, 0x71, 0x05, 0x27, 0x78, 0x2b, 0xc1,
	0x29, 0xd4, 0xcc, 0xe2, 0x0a, 0x4e, 0xf0, 0x57, 0x82, 0x53, 0xa9, 0x99, 0xc4, 0x14, 0x9d, 0xe0,
	0xaf, 0x04, 0xa7, 0x53, 0x33, 0x88, 0x29, 0x3b, 0xc1, 0x5e, 0x09, 0x4e, 0xa6, 0x67, 0x10, 0x52,
	0x77, 0x82, 0xbc, 0x00, 0x18, 0x00
};

static const uint8 cr_data[] =
{
	                                    0x1b, 0xfc, 0x11, 0xc1, 0x0f, 0x4a, 0xc1, 0x4f, 0x4a, 0xc1,
	0x4f, 0x4a, 0xa1, 0x4d, 0x95, 0x42, 0x9e, 0x95, 0x42, 0x9e, 0x95, 0x42, 0x9b, 0x2a, 0x85, 0x3d,
	0x2a, 0x85, 0x3d, 0x2a, 0x85, 0x36, 0x55, 0x0a, 0x7a, 0x55, 0x0a, 0x7a, 0x55, 0x0a, 0x6c, 0xaa,
	0x14, 0xf4, 0xaa, 0x14, 0xf4, 0xaa, 0x14, 0xd9, 0x54, 0x29, 0xe9, 0x54, 0x29, 0xe9, 0x54, 0x29,
	0xb2, 0xa8, 0x53, 0xd2, 0xa8, 0x53, 0xd2, 0xa8, 0x53, 0x65, 0x50, 0xa7, 0xa5, 0x50, 0xa7, 0xa5,
	0x50, 0xa6, 0xca, 0xa1, 0x4f, 0x4a, 0xa1, 0x4f, 0x4a, 0xa1, 0x4d, 0x95, 0x42, 0x9e, 0x95, 0x42,
	0x9e, 0x95, 0x42, 0x9b, 0x2a, 0x80, 0x00, 0x41, 0xe3, 0x80, 0x3f, 0xe2, 0x09, 0x9c, 0x00, 0x22,
	0x07, 0x03, 0xe1, 0x26, 0x70, 0x06, 0x07, 0x1f, 0x04, 0x67, 0x00, 0x61, 0xdf, 0x02, 0x67, 0x00,
	0x0c, 0x3b, 0xfe, 0x01, 0x33, 0x80, 0x06, 0x1d, 0xff, 0x00, 0x99, 0xc0, 0x03, 0x0e, 0xff, 0x80,
	0x4c, 0xe0, 0x01, 0x87, 0x7f, 0xc0, 0x26, 0x70, 0x00, 0xc3, 0xbf, 0xe0, 0x13, 0x38, 0x00, 0x61,
	0xdf, 0xf0, 0x09, 0x9c, 0x00, 0x30, 0xef, 0xf8, 0x04, 0xce, 0x00, 0x18, 0x77, 0xfc, 0x02, 0x67,
	0x00, 0x0c, 0x3b, 0xfe, 0x01, 0x33, 0x80, 0x06, 0x1d, 0xff, 0x00, 0x99, 0xc0, 0x03, 0x0e, 0xff,
	0x80, 0x4c, 0xe0, 0x01, 0x87, 0x7f, 0xc0, 0x26, 0x70, 0x00, 0x00, 0x08, 0x3c, 0x20, 0x1f, 0xf1,
	0x00, 0xf0, 0x05, 0x02, 0x93, 0x84, 0x3d, 0x20, 0xf0, 0x52, 0x81, 0xc7, 0xff, 0xff, 0xea, 0x54,
	0x01, 0x80, 0x05, 0xf5, 0x4a, 0x80, 0x30, 0x00, 0xb6, 0xa5, 0x40, 0x18, 0x00, 0x5f, 0x54, 0xa8,
	0x03, 0x00, 0x0b, 0xea, 0x95, 0x00, 0x60, 0x01, 0x6d, 0x4a, 0x80, 0x30, 0x00, 0x00, 0x22, 0x3f,
	0xba, 0x08, 0x10, 0x2b, 0x1f, 0xf2, 0x20, 0x3e, 0x49, 0x9c, 0x1f, 0x6e, 0x0f, 0x5a, 0x0f, 0xfb,
	0x18, 0x46, 0xae, 0x27, 0x9b, 0x83, 0xcb, 0x41, 0xf3, 0x18, 0x46, 0xae, 0x27, 0x9b, 0x83, 0xc5,
	0xa0, 0xf9, 0x8c, 0x22, 0xd7, 0x13, 0x8d, 0xc1, 0xe2, 0xd0, 0x7c, 0xc6, 0x11, 0x6b, 0x89, 0xc6,
	0xe0, 0xf1, 0x68, 0x3e, 0x63, 0x08, 0xb5, 0xc4, 0xe3, 0x70, 0x78, 0xb4, 0x1f, 0x31, 0x84, 0x5a,
	0xe2, 0x71, 0xb8, 0x3c, 0x5a, 0x0f, 0x98, 0xc2, 0x2d, 0x71, 0x30, 0x83, 0xc0, 0x00
};

/* HL1, LH1, HH1, HL2, LH2, HH2, HL3, LH3, HH3, LL3 */
static const unsigned int test_quantization_values[] =
{
	6, 6, 6, 6, 7, 7, 8, 8, 8, 9
};

static const uint8 rgb_scanline_data[] =
{
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,

	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF
};

static uint8* rgb_data;

int init_rfx_suite(void)
{
	return 0;
}

int clean_rfx_suite(void)
{
	return 0;
}

int add_rfx_suite(void)
{
	add_test_suite(rfx);

	add_test_function(bitstream);
	add_test_function(bitstream_enc);
	add_test_function(rlgr);
	add_test_function(differential);
	add_test_function(quantization);
	add_test_function(dwt);
	add_test_function(decode);
	add_test_function(encode);
	add_test_function(message);

	return 0;
}

void test_bitstream(void)
{
	uint16 b;
	RFX_BITSTREAM* bs;

	bs = xnew(RFX_BITSTREAM);
	rfx_bitstream_attach(bs, (uint8*) y_data, sizeof(y_data));
	while (!rfx_bitstream_eos(bs))
	{
		rfx_bitstream_get_bits(bs, 3, b);
		(void) b;
		//printf("%u ", b);
	}
	xfree(bs);

	//printf("\n");
}

void test_bitstream_enc(void)
{
	uint8 buffer[10];
	RFX_BITSTREAM* bs;
	int i;

	bs = xnew(RFX_BITSTREAM);
	memset(buffer, 0, sizeof(buffer));
	rfx_bitstream_attach(bs, buffer, sizeof(buffer));
	for (i = 0; i < 16; i++)
	{
		rfx_bitstream_put_bits(bs, i, 5);
	}
	/*for (i = 0; i < sizeof(buffer); i++)
	{
		printf("%X ", buffer[i]);
	}*/
	xfree(bs);

	//printf("\n");
}

static sint16 buffer[4096];

void dump_buffer(sint16* buf, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		if (i % 16 == 0)
			printf("\n%04d ", i);
		printf("% 4d ", buf[i]);
	}
	printf("\n");
}

void test_rlgr(void)
{
	int n;

	n = rfx_rlgr_decode(RLGR3, y_data, sizeof(y_data), buffer, sizeof(buffer) / sizeof(unsigned int));

	//printf("RLGR decode %d bytes to %d values.", sizeof(y_data), n);
	//dump_buffer(buffer, n);
}

void test_differential(void)
{
	rfx_differential_decode(buffer + 4032, 64);
	//dump_buffer(buffer + 4032, 64);
}

void test_quantization(void)
{
	rfx_quantization_decode(buffer, test_quantization_values);
	//dump_buffer(buffer, 4096);
}

void test_dwt(void)
{
	RFX_CONTEXT* context;

	context = rfx_context_new();
	rfx_dwt_2d_decode(buffer, context->priv->dwt_buffer);
	//dump_buffer(buffer, 4096);
	rfx_context_free(context);
}

/* Dump a .ppm image. */
static void dump_ppm_image(uint8* image_buf)
{
	static int frame_id = 0;
	char buf[100];
	FILE* fp;

	snprintf(buf, sizeof(buf), "/tmp/FreeRDP_Frame_%d.ppm", frame_id);
	fp = fopen(buf, "wb");
	fwrite("P6\n", 1, 3, fp);
	fwrite("# Created by FreeRDP\n", 1, 21, fp);
	fwrite("64 64\n", 1, 6, fp);
	fwrite("255\n", 1, 4, fp);
	fwrite(image_buf, 1, 4096 * 3, fp);
	fflush(fp);
	fclose(fp);
	frame_id++;
}

void test_decode(void)
{
	RFX_CONTEXT* context;
	uint8 decode_buffer[4096 * 3];
	STREAM* s;

	s = stream_new(sizeof(y_data) + sizeof(cb_data) + sizeof(cr_data));
	stream_write(s, y_data, sizeof(y_data));
	stream_write(s, cb_data, sizeof(cb_data));
	stream_write(s, cr_data, sizeof(cr_data));
	stream_set_pos(s, 0);

	context = rfx_context_new();
	context->mode = RLGR3;
	rfx_context_set_pixel_format(context, RFX_PIXEL_FORMAT_RGB);
	rfx_decode_rgb(context, s,
		sizeof(y_data), test_quantization_values,
		sizeof(cb_data), test_quantization_values,
		sizeof(cr_data), test_quantization_values,
		decode_buffer);
	rfx_context_free(context);
	stream_free(s);

	dump_ppm_image(decode_buffer);
}

void test_encode(void)
{
	RFX_CONTEXT* context;
	STREAM* enc_stream;
	int y_size, cb_size, cr_size;
	int i;
	uint8 decode_buffer[4096 * 3];

	rgb_data = (uint8 *) malloc(64 * 64 * 3);
	for (i = 0; i < 64; i++)
		memcpy(rgb_data + i * 64 * 3, rgb_scanline_data, 64 * 3);
	//freerdp_hexdump(rgb_data, 64 * 64 * 3);

	enc_stream = stream_new(65536);
	stream_clear(enc_stream);

	context = rfx_context_new();
	context->mode = RLGR3;
	rfx_context_set_pixel_format(context, RFX_PIXEL_FORMAT_RGB);

	rfx_encode_rgb(context, rgb_data, 64, 64, 64 * 3,
		test_quantization_values, test_quantization_values, test_quantization_values,
		enc_stream, &y_size, &cb_size, &cr_size);
	//dump_buffer(context->priv->cb_g_buffer, 4096);

	/*printf("*** Y ***\n");
	freerdp_hexdump(stream_get_head(enc_stream), y_size);
	printf("*** Cb ***\n");
	freerdp_hexdump(stream_get_head(enc_stream) + y_size, cb_size);
	printf("*** Cr ***\n");
	freerdp_hexdump(stream_get_head(enc_stream) + y_size + cb_size, cr_size);*/

	stream_set_pos(enc_stream, 0);
	rfx_decode_rgb(context, enc_stream,
		y_size, test_quantization_values,
		cb_size, test_quantization_values,
		cr_size, test_quantization_values,
		decode_buffer);
	dump_ppm_image(decode_buffer);

	rfx_context_free(context);
	stream_free(enc_stream);
	free(rgb_data);
}

void test_message(void)
{
	RFX_CONTEXT* context;
	STREAM* s;
	int i, j;
	RFX_RECT rect = {0, 0, 100, 80};
	RFX_MESSAGE * message;

	rgb_data = (uint8 *) malloc(100 * 80 * 3);
	for (i = 0; i < 80; i++)
		memcpy(rgb_data + i * 100 * 3, rgb_scanline_data, 100 * 3);

	s = stream_new(65536);
	stream_clear(s);

	context = rfx_context_new();
	context->mode = RLGR3;
	context->width = 800;
	context->height = 600;
	rfx_context_set_pixel_format(context, RFX_PIXEL_FORMAT_RGB);

	for (i = 0; i < 1000; i++)
	{
		s = stream_new(65536);
		stream_clear(s);
		rfx_compose_message(context, s,
			&rect, 1, rgb_data, 100, 80, 100 * 3);
		stream_seal(s);
		/*hexdump(buffer, size);*/
		stream_set_pos(s, 0);
		message = rfx_process_message(context, s->p, s->size);
		if (i == 0)
		{
			for (j = 0; j < message->num_tiles; j++)
			{
				dump_ppm_image(message->tiles[j]->data);
			}
		}
		rfx_message_free(context, message);
		stream_free(s);
	}

	rfx_context_free(context);
	free(rgb_data);
}
