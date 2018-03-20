/*
 * Copyright (C) 2010 Tobias Brunner
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "diffie_hellman.h"

ENUM_BEGIN(diffie_hellman_group_names, MODP_NONE, MODP_1024_BIT,
	"MODP_NONE",
	"MODP_768",
	"MODP_1024");
ENUM_NEXT(diffie_hellman_group_names, MODP_1536_BIT, MODP_1536_BIT, MODP_1024_BIT,
	"MODP_1536");
ENUM_NEXT(diffie_hellman_group_names, MODP_2048_BIT, ECP_521_BIT, MODP_1536_BIT,
	"MODP_2048",
	"MODP_3072",
	"MODP_4096",
	"MODP_6144",
	"MODP_8192",
	"ECP_256",
	"ECP_384",
	"ECP_521");
ENUM_NEXT(diffie_hellman_group_names, MODP_1024_160, CURVE_448, ECP_521_BIT,
	"MODP_1024_160",
	"MODP_2048_224",
	"MODP_2048_256",
	"ECP_192",
	"ECP_224",
	"ECP_224_BP",
	"ECP_256_BP",
	"ECP_384_BP",
	"ECP_512_BP",
	"CURVE_25519",
	"CURVE_448");
ENUM_NEXT(diffie_hellman_group_names, MODP_NULL, MODP_NULL, CURVE_448,
	"MODP_NULL");
ENUM_NEXT(diffie_hellman_group_names, NTRU_112_BIT, NTRU_256_BIT, MODP_NULL,
	"NTRU_112",
	"NTRU_128",
	"NTRU_192",
	"NTRU_256");
ENUM_NEXT(diffie_hellman_group_names, NH_128_BIT, NH_128_BIT, NTRU_256_BIT,
	"NEWHOPE_128");
ENUM_NEXT(diffie_hellman_group_names, NIST_PQC_NEWHOPE512CCA, NIST_PQC_LEDAKEM128SLN02, NH_128_BIT,
	"NIST_PQC_NEWHOPE512CCA",
	"NIST_PQC_KYBER512",
	"NIST_PQC_NTRULPR4591761",
	"NIST_PQC_NTRUKEM443",
	"NIST_PQC_SIKEP503",
	"NIST_PQC_LEDAKEM128SLN02");
ENUM_NEXT(diffie_hellman_group_names, MODP_CUSTOM, MODP_CUSTOM, NIST_PQC_LEDAKEM128SLN02,
	"MODP_CUSTOM");
ENUM_END(diffie_hellman_group_names, MODP_CUSTOM);


/**
 * List of known diffie hellman group parameters.
 */
static struct {
	/* Public part of the struct */
	diffie_hellman_params_t public;
	/* The group identifier as specified in IKEv2 */
	diffie_hellman_group_t group;
	/* Optimal length of the exponent (in bytes), as specified in RFC 3526. */
	size_t opt_exp;
} dh_params[] = {
	{
		.group = MODP_768_BIT, .opt_exp = 32, .public = {
			.exp_len = 32,
			.generator = chunk_from_chars(0x02),
			.prime = chunk_from_chars(
				0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
				0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
				0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
				0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
				0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
				0xF4,0x4C,0x42,0xE9,0xA6,0x3A,0x36,0x20,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
		},
	},{
		.group = MODP_1024_BIT, .opt_exp = 32, .public = {
			.exp_len = 32,
			.generator = chunk_from_chars(0x02),
			.prime = chunk_from_chars(
				0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
				0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
				0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
				0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
				0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
				0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
				0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
				0x49,0x28,0x66,0x51,0xEC,0xE6,0x53,0x81,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
		},
	},{
		.group = MODP_1536_BIT, .opt_exp = 32, .public = {
			.exp_len = 32,
			.generator = chunk_from_chars(0x02),
			.prime = chunk_from_chars(
				0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
				0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
				0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
				0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
				0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
				0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
				0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
				0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
				0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
				0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
				0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
				0xF1,0x74,0x6C,0x08,0xCA,0x23,0x73,0x27,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
		},
	},{
		.group = MODP_2048_BIT, .opt_exp = 48, .public = {
			.exp_len = 48,
			.generator = chunk_from_chars(0x02),
			.prime = chunk_from_chars(
				0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
				0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
				0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
				0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
				0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
				0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
				0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
				0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
				0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
				0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
				0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
				0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
				0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
				0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
				0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
				0x15,0x72,0x8E,0x5A,0x8A,0xAC,0xAA,0x68,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
		},
	},{
		.group = MODP_3072_BIT, .opt_exp = 48, .public = {
			.exp_len = 48,
			.generator = chunk_from_chars(0x02),
			.prime = chunk_from_chars(
				0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
				0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
				0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
				0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
				0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
				0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
				0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
				0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
				0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
				0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
				0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
				0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
				0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
				0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
				0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
				0x15,0x72,0x8E,0x5A,0x8A,0xAA,0xC4,0x2D,0xAD,0x33,0x17,0x0D,0x04,0x50,0x7A,0x33,
				0xA8,0x55,0x21,0xAB,0xDF,0x1C,0xBA,0x64,0xEC,0xFB,0x85,0x04,0x58,0xDB,0xEF,0x0A,
				0x8A,0xEA,0x71,0x57,0x5D,0x06,0x0C,0x7D,0xB3,0x97,0x0F,0x85,0xA6,0xE1,0xE4,0xC7,
				0xAB,0xF5,0xAE,0x8C,0xDB,0x09,0x33,0xD7,0x1E,0x8C,0x94,0xE0,0x4A,0x25,0x61,0x9D,
				0xCE,0xE3,0xD2,0x26,0x1A,0xD2,0xEE,0x6B,0xF1,0x2F,0xFA,0x06,0xD9,0x8A,0x08,0x64,
				0xD8,0x76,0x02,0x73,0x3E,0xC8,0x6A,0x64,0x52,0x1F,0x2B,0x18,0x17,0x7B,0x20,0x0C,
				0xBB,0xE1,0x17,0x57,0x7A,0x61,0x5D,0x6C,0x77,0x09,0x88,0xC0,0xBA,0xD9,0x46,0xE2,
				0x08,0xE2,0x4F,0xA0,0x74,0xE5,0xAB,0x31,0x43,0xDB,0x5B,0xFC,0xE0,0xFD,0x10,0x8E,
				0x4B,0x82,0xD1,0x20,0xA9,0x3A,0xD2,0xCA,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
		},
	},{
		.group = MODP_4096_BIT, .opt_exp = 64, .public = {
			.exp_len = 64,
			.generator = chunk_from_chars(0x02),
			.prime = chunk_from_chars(
				0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
				0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
				0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
				0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
				0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
				0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
				0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
				0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
				0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
				0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
				0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
				0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
				0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
				0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
				0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
				0x15,0x72,0x8E,0x5A,0x8A,0xAA,0xC4,0x2D,0xAD,0x33,0x17,0x0D,0x04,0x50,0x7A,0x33,
				0xA8,0x55,0x21,0xAB,0xDF,0x1C,0xBA,0x64,0xEC,0xFB,0x85,0x04,0x58,0xDB,0xEF,0x0A,
				0x8A,0xEA,0x71,0x57,0x5D,0x06,0x0C,0x7D,0xB3,0x97,0x0F,0x85,0xA6,0xE1,0xE4,0xC7,
				0xAB,0xF5,0xAE,0x8C,0xDB,0x09,0x33,0xD7,0x1E,0x8C,0x94,0xE0,0x4A,0x25,0x61,0x9D,
				0xCE,0xE3,0xD2,0x26,0x1A,0xD2,0xEE,0x6B,0xF1,0x2F,0xFA,0x06,0xD9,0x8A,0x08,0x64,
				0xD8,0x76,0x02,0x73,0x3E,0xC8,0x6A,0x64,0x52,0x1F,0x2B,0x18,0x17,0x7B,0x20,0x0C,
				0xBB,0xE1,0x17,0x57,0x7A,0x61,0x5D,0x6C,0x77,0x09,0x88,0xC0,0xBA,0xD9,0x46,0xE2,
				0x08,0xE2,0x4F,0xA0,0x74,0xE5,0xAB,0x31,0x43,0xDB,0x5B,0xFC,0xE0,0xFD,0x10,0x8E,
				0x4B,0x82,0xD1,0x20,0xA9,0x21,0x08,0x01,0x1A,0x72,0x3C,0x12,0xA7,0x87,0xE6,0xD7,
				0x88,0x71,0x9A,0x10,0xBD,0xBA,0x5B,0x26,0x99,0xC3,0x27,0x18,0x6A,0xF4,0xE2,0x3C,
				0x1A,0x94,0x68,0x34,0xB6,0x15,0x0B,0xDA,0x25,0x83,0xE9,0xCA,0x2A,0xD4,0x4C,0xE8,
				0xDB,0xBB,0xC2,0xDB,0x04,0xDE,0x8E,0xF9,0x2E,0x8E,0xFC,0x14,0x1F,0xBE,0xCA,0xA6,
				0x28,0x7C,0x59,0x47,0x4E,0x6B,0xC0,0x5D,0x99,0xB2,0x96,0x4F,0xA0,0x90,0xC3,0xA2,
				0x23,0x3B,0xA1,0x86,0x51,0x5B,0xE7,0xED,0x1F,0x61,0x29,0x70,0xCE,0xE2,0xD7,0xAF,
				0xB8,0x1B,0xDD,0x76,0x21,0x70,0x48,0x1C,0xD0,0x06,0x91,0x27,0xD5,0xB0,0x5A,0xA9,
				0x93,0xB4,0xEA,0x98,0x8D,0x8F,0xDD,0xC1,0x86,0xFF,0xB7,0xDC,0x90,0xA6,0xC0,0x8F,
				0x4D,0xF4,0x35,0xC9,0x34,0x06,0x31,0x99,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
		},
	},{
		.group = MODP_6144_BIT, .opt_exp = 64, .public = {
			.exp_len = 64,
			.generator = chunk_from_chars(0x02),
			.prime = chunk_from_chars(
				0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
				0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
				0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
				0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
				0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
				0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
				0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
				0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
				0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
				0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
				0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
				0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
				0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
				0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
				0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
				0x15,0x72,0x8E,0x5A,0x8A,0xAA,0xC4,0x2D,0xAD,0x33,0x17,0x0D,0x04,0x50,0x7A,0x33,
				0xA8,0x55,0x21,0xAB,0xDF,0x1C,0xBA,0x64,0xEC,0xFB,0x85,0x04,0x58,0xDB,0xEF,0x0A,
				0x8A,0xEA,0x71,0x57,0x5D,0x06,0x0C,0x7D,0xB3,0x97,0x0F,0x85,0xA6,0xE1,0xE4,0xC7,
				0xAB,0xF5,0xAE,0x8C,0xDB,0x09,0x33,0xD7,0x1E,0x8C,0x94,0xE0,0x4A,0x25,0x61,0x9D,
				0xCE,0xE3,0xD2,0x26,0x1A,0xD2,0xEE,0x6B,0xF1,0x2F,0xFA,0x06,0xD9,0x8A,0x08,0x64,
				0xD8,0x76,0x02,0x73,0x3E,0xC8,0x6A,0x64,0x52,0x1F,0x2B,0x18,0x17,0x7B,0x20,0x0C,
				0xBB,0xE1,0x17,0x57,0x7A,0x61,0x5D,0x6C,0x77,0x09,0x88,0xC0,0xBA,0xD9,0x46,0xE2,
				0x08,0xE2,0x4F,0xA0,0x74,0xE5,0xAB,0x31,0x43,0xDB,0x5B,0xFC,0xE0,0xFD,0x10,0x8E,
				0x4B,0x82,0xD1,0x20,0xA9,0x21,0x08,0x01,0x1A,0x72,0x3C,0x12,0xA7,0x87,0xE6,0xD7,
				0x88,0x71,0x9A,0x10,0xBD,0xBA,0x5B,0x26,0x99,0xC3,0x27,0x18,0x6A,0xF4,0xE2,0x3C,
				0x1A,0x94,0x68,0x34,0xB6,0x15,0x0B,0xDA,0x25,0x83,0xE9,0xCA,0x2A,0xD4,0x4C,0xE8,
				0xDB,0xBB,0xC2,0xDB,0x04,0xDE,0x8E,0xF9,0x2E,0x8E,0xFC,0x14,0x1F,0xBE,0xCA,0xA6,
				0x28,0x7C,0x59,0x47,0x4E,0x6B,0xC0,0x5D,0x99,0xB2,0x96,0x4F,0xA0,0x90,0xC3,0xA2,
				0x23,0x3B,0xA1,0x86,0x51,0x5B,0xE7,0xED,0x1F,0x61,0x29,0x70,0xCE,0xE2,0xD7,0xAF,
				0xB8,0x1B,0xDD,0x76,0x21,0x70,0x48,0x1C,0xD0,0x06,0x91,0x27,0xD5,0xB0,0x5A,0xA9,
				0x93,0xB4,0xEA,0x98,0x8D,0x8F,0xDD,0xC1,0x86,0xFF,0xB7,0xDC,0x90,0xA6,0xC0,0x8F,
				0x4D,0xF4,0x35,0xC9,0x34,0x02,0x84,0x92,0x36,0xC3,0xFA,0xB4,0xD2,0x7C,0x70,0x26,
				0xC1,0xD4,0xDC,0xB2,0x60,0x26,0x46,0xDE,0xC9,0x75,0x1E,0x76,0x3D,0xBA,0x37,0xBD,
				0xF8,0xFF,0x94,0x06,0xAD,0x9E,0x53,0x0E,0xE5,0xDB,0x38,0x2F,0x41,0x30,0x01,0xAE,
				0xB0,0x6A,0x53,0xED,0x90,0x27,0xD8,0x31,0x17,0x97,0x27,0xB0,0x86,0x5A,0x89,0x18,
				0xDA,0x3E,0xDB,0xEB,0xCF,0x9B,0x14,0xED,0x44,0xCE,0x6C,0xBA,0xCE,0xD4,0xBB,0x1B,
				0xDB,0x7F,0x14,0x47,0xE6,0xCC,0x25,0x4B,0x33,0x20,0x51,0x51,0x2B,0xD7,0xAF,0x42,
				0x6F,0xB8,0xF4,0x01,0x37,0x8C,0xD2,0xBF,0x59,0x83,0xCA,0x01,0xC6,0x4B,0x92,0xEC,
				0xF0,0x32,0xEA,0x15,0xD1,0x72,0x1D,0x03,0xF4,0x82,0xD7,0xCE,0x6E,0x74,0xFE,0xF6,
				0xD5,0x5E,0x70,0x2F,0x46,0x98,0x0C,0x82,0xB5,0xA8,0x40,0x31,0x90,0x0B,0x1C,0x9E,
				0x59,0xE7,0xC9,0x7F,0xBE,0xC7,0xE8,0xF3,0x23,0xA9,0x7A,0x7E,0x36,0xCC,0x88,0xBE,
				0x0F,0x1D,0x45,0xB7,0xFF,0x58,0x5A,0xC5,0x4B,0xD4,0x07,0xB2,0x2B,0x41,0x54,0xAA,
				0xCC,0x8F,0x6D,0x7E,0xBF,0x48,0xE1,0xD8,0x14,0xCC,0x5E,0xD2,0x0F,0x80,0x37,0xE0,
				0xA7,0x97,0x15,0xEE,0xF2,0x9B,0xE3,0x28,0x06,0xA1,0xD5,0x8B,0xB7,0xC5,0xDA,0x76,
				0xF5,0x50,0xAA,0x3D,0x8A,0x1F,0xBF,0xF0,0xEB,0x19,0xCC,0xB1,0xA3,0x13,0xD5,0x5C,
				0xDA,0x56,0xC9,0xEC,0x2E,0xF2,0x96,0x32,0x38,0x7F,0xE8,0xD7,0x6E,0x3C,0x04,0x68,
				0x04,0x3E,0x8F,0x66,0x3F,0x48,0x60,0xEE,0x12,0xBF,0x2D,0x5B,0x0B,0x74,0x74,0xD6,
				0xE6,0x94,0xF9,0x1E,0x6D,0xCC,0x40,0x24,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
		},
	},{
		.group = MODP_8192_BIT, .opt_exp = 64, .public = {
			.exp_len = 64,
			.generator = chunk_from_chars(0x02),
			.prime = chunk_from_chars(
				0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
				0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
				0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
				0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
				0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
				0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
				0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
				0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
				0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
				0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
				0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
				0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
				0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
				0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
				0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
				0x15,0x72,0x8E,0x5A,0x8A,0xAA,0xC4,0x2D,0xAD,0x33,0x17,0x0D,0x04,0x50,0x7A,0x33,
				0xA8,0x55,0x21,0xAB,0xDF,0x1C,0xBA,0x64,0xEC,0xFB,0x85,0x04,0x58,0xDB,0xEF,0x0A,
				0x8A,0xEA,0x71,0x57,0x5D,0x06,0x0C,0x7D,0xB3,0x97,0x0F,0x85,0xA6,0xE1,0xE4,0xC7,
				0xAB,0xF5,0xAE,0x8C,0xDB,0x09,0x33,0xD7,0x1E,0x8C,0x94,0xE0,0x4A,0x25,0x61,0x9D,
				0xCE,0xE3,0xD2,0x26,0x1A,0xD2,0xEE,0x6B,0xF1,0x2F,0xFA,0x06,0xD9,0x8A,0x08,0x64,
				0xD8,0x76,0x02,0x73,0x3E,0xC8,0x6A,0x64,0x52,0x1F,0x2B,0x18,0x17,0x7B,0x20,0x0C,
				0xBB,0xE1,0x17,0x57,0x7A,0x61,0x5D,0x6C,0x77,0x09,0x88,0xC0,0xBA,0xD9,0x46,0xE2,
				0x08,0xE2,0x4F,0xA0,0x74,0xE5,0xAB,0x31,0x43,0xDB,0x5B,0xFC,0xE0,0xFD,0x10,0x8E,
				0x4B,0x82,0xD1,0x20,0xA9,0x21,0x08,0x01,0x1A,0x72,0x3C,0x12,0xA7,0x87,0xE6,0xD7,
				0x88,0x71,0x9A,0x10,0xBD,0xBA,0x5B,0x26,0x99,0xC3,0x27,0x18,0x6A,0xF4,0xE2,0x3C,
				0x1A,0x94,0x68,0x34,0xB6,0x15,0x0B,0xDA,0x25,0x83,0xE9,0xCA,0x2A,0xD4,0x4C,0xE8,
				0xDB,0xBB,0xC2,0xDB,0x04,0xDE,0x8E,0xF9,0x2E,0x8E,0xFC,0x14,0x1F,0xBE,0xCA,0xA6,
				0x28,0x7C,0x59,0x47,0x4E,0x6B,0xC0,0x5D,0x99,0xB2,0x96,0x4F,0xA0,0x90,0xC3,0xA2,
				0x23,0x3B,0xA1,0x86,0x51,0x5B,0xE7,0xED,0x1F,0x61,0x29,0x70,0xCE,0xE2,0xD7,0xAF,
				0xB8,0x1B,0xDD,0x76,0x21,0x70,0x48,0x1C,0xD0,0x06,0x91,0x27,0xD5,0xB0,0x5A,0xA9,
				0x93,0xB4,0xEA,0x98,0x8D,0x8F,0xDD,0xC1,0x86,0xFF,0xB7,0xDC,0x90,0xA6,0xC0,0x8F,
				0x4D,0xF4,0x35,0xC9,0x34,0x02,0x84,0x92,0x36,0xC3,0xFA,0xB4,0xD2,0x7C,0x70,0x26,
				0xC1,0xD4,0xDC,0xB2,0x60,0x26,0x46,0xDE,0xC9,0x75,0x1E,0x76,0x3D,0xBA,0x37,0xBD,
				0xF8,0xFF,0x94,0x06,0xAD,0x9E,0x53,0x0E,0xE5,0xDB,0x38,0x2F,0x41,0x30,0x01,0xAE,
				0xB0,0x6A,0x53,0xED,0x90,0x27,0xD8,0x31,0x17,0x97,0x27,0xB0,0x86,0x5A,0x89,0x18,
				0xDA,0x3E,0xDB,0xEB,0xCF,0x9B,0x14,0xED,0x44,0xCE,0x6C,0xBA,0xCE,0xD4,0xBB,0x1B,
				0xDB,0x7F,0x14,0x47,0xE6,0xCC,0x25,0x4B,0x33,0x20,0x51,0x51,0x2B,0xD7,0xAF,0x42,
				0x6F,0xB8,0xF4,0x01,0x37,0x8C,0xD2,0xBF,0x59,0x83,0xCA,0x01,0xC6,0x4B,0x92,0xEC,
				0xF0,0x32,0xEA,0x15,0xD1,0x72,0x1D,0x03,0xF4,0x82,0xD7,0xCE,0x6E,0x74,0xFE,0xF6,
				0xD5,0x5E,0x70,0x2F,0x46,0x98,0x0C,0x82,0xB5,0xA8,0x40,0x31,0x90,0x0B,0x1C,0x9E,
				0x59,0xE7,0xC9,0x7F,0xBE,0xC7,0xE8,0xF3,0x23,0xA9,0x7A,0x7E,0x36,0xCC,0x88,0xBE,
				0x0F,0x1D,0x45,0xB7,0xFF,0x58,0x5A,0xC5,0x4B,0xD4,0x07,0xB2,0x2B,0x41,0x54,0xAA,
				0xCC,0x8F,0x6D,0x7E,0xBF,0x48,0xE1,0xD8,0x14,0xCC,0x5E,0xD2,0x0F,0x80,0x37,0xE0,
				0xA7,0x97,0x15,0xEE,0xF2,0x9B,0xE3,0x28,0x06,0xA1,0xD5,0x8B,0xB7,0xC5,0xDA,0x76,
				0xF5,0x50,0xAA,0x3D,0x8A,0x1F,0xBF,0xF0,0xEB,0x19,0xCC,0xB1,0xA3,0x13,0xD5,0x5C,
				0xDA,0x56,0xC9,0xEC,0x2E,0xF2,0x96,0x32,0x38,0x7F,0xE8,0xD7,0x6E,0x3C,0x04,0x68,
				0x04,0x3E,0x8F,0x66,0x3F,0x48,0x60,0xEE,0x12,0xBF,0x2D,0x5B,0x0B,0x74,0x74,0xD6,
				0xE6,0x94,0xF9,0x1E,0x6D,0xBE,0x11,0x59,0x74,0xA3,0x92,0x6F,0x12,0xFE,0xE5,0xE4,
				0x38,0x77,0x7C,0xB6,0xA9,0x32,0xDF,0x8C,0xD8,0xBE,0xC4,0xD0,0x73,0xB9,0x31,0xBA,
				0x3B,0xC8,0x32,0xB6,0x8D,0x9D,0xD3,0x00,0x74,0x1F,0xA7,0xBF,0x8A,0xFC,0x47,0xED,
				0x25,0x76,0xF6,0x93,0x6B,0xA4,0x24,0x66,0x3A,0xAB,0x63,0x9C,0x5A,0xE4,0xF5,0x68,
				0x34,0x23,0xB4,0x74,0x2B,0xF1,0xC9,0x78,0x23,0x8F,0x16,0xCB,0xE3,0x9D,0x65,0x2D,
				0xE3,0xFD,0xB8,0xBE,0xFC,0x84,0x8A,0xD9,0x22,0x22,0x2E,0x04,0xA4,0x03,0x7C,0x07,
				0x13,0xEB,0x57,0xA8,0x1A,0x23,0xF0,0xC7,0x34,0x73,0xFC,0x64,0x6C,0xEA,0x30,0x6B,
				0x4B,0xCB,0xC8,0x86,0x2F,0x83,0x85,0xDD,0xFA,0x9D,0x4B,0x7F,0xA2,0xC0,0x87,0xE8,
				0x79,0x68,0x33,0x03,0xED,0x5B,0xDD,0x3A,0x06,0x2B,0x3C,0xF5,0xB3,0xA2,0x78,0xA6,
				0x6D,0x2A,0x13,0xF8,0x3F,0x44,0xF8,0x2D,0xDF,0x31,0x0E,0xE0,0x74,0xAB,0x6A,0x36,
				0x45,0x97,0xE8,0x99,0xA0,0x25,0x5D,0xC1,0x64,0xF3,0x1C,0xC5,0x08,0x46,0x85,0x1D,
				0xF9,0xAB,0x48,0x19,0x5D,0xED,0x7E,0xA1,0xB1,0xD5,0x10,0xBD,0x7E,0xE7,0x4D,0x73,
				0xFA,0xF3,0x6B,0xC3,0x1E,0xCF,0xA2,0x68,0x35,0x90,0x46,0xF4,0xEB,0x87,0x9F,0x92,
				0x40,0x09,0x43,0x8B,0x48,0x1C,0x6C,0xD7,0x88,0x9A,0x00,0x2E,0xD5,0xEE,0x38,0x2B,
				0xC9,0x19,0x0D,0xA6,0xFC,0x02,0x6E,0x47,0x95,0x58,0xE4,0x47,0x56,0x77,0xE9,0xAA,
				0x9E,0x30,0x50,0xE2,0x76,0x56,0x94,0xDF,0xC8,0x1F,0x56,0xE8,0x80,0xB9,0x6E,0x71,
				0x60,0xC9,0x80,0xDD,0x98,0xED,0xD3,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF),
		},
	},{
		.group = MODP_1024_160, .opt_exp = 20, .public = {
			.exp_len = 20,
			.subgroup = chunk_from_chars(
				0xF5,0x18,0xAA,0x87,0x81,0xA8,0xDF,0x27,0x8A,0xBA,0x4E,0x7D,0x64,0xB7,0xCB,0x9D,
				0x49,0x46,0x23,0x53),
			.generator = chunk_from_chars(
				0xA4,0xD1,0xCB,0xD5,0xC3,0xFD,0x34,0x12,0x67,0x65,0xA4,0x42,0xEF,0xB9,0x99,0x05,
				0xF8,0x10,0x4D,0xD2,0x58,0xAC,0x50,0x7F,0xD6,0x40,0x6C,0xFF,0x14,0x26,0x6D,0x31,
				0x26,0x6F,0xEA,0x1E,0x5C,0x41,0x56,0x4B,0x77,0x7E,0x69,0x0F,0x55,0x04,0xF2,0x13,
				0x16,0x02,0x17,0xB4,0xB0,0x1B,0x88,0x6A,0x5E,0x91,0x54,0x7F,0x9E,0x27,0x49,0xF4,
				0xD7,0xFB,0xD7,0xD3,0xB9,0xA9,0x2E,0xE1,0x90,0x9D,0x0D,0x22,0x63,0xF8,0x0A,0x76,
				0xA6,0xA2,0x4C,0x08,0x7A,0x09,0x1F,0x53,0x1D,0xBF,0x0A,0x01,0x69,0xB6,0xA2,0x8A,
				0xD6,0x62,0xA4,0xD1,0x8E,0x73,0xAF,0xA3,0x2D,0x77,0x9D,0x59,0x18,0xD0,0x8B,0xC8,
				0x85,0x8F,0x4D,0xCE,0xF9,0x7C,0x2A,0x24,0x85,0x5E,0x6E,0xEB,0x22,0xB3,0xB2,0xE5),
			.prime = chunk_from_chars(
				0xB1,0x0B,0x8F,0x96,0xA0,0x80,0xE0,0x1D,0xDE,0x92,0xDE,0x5E,0xAE,0x5D,0x54,0xEC,
				0x52,0xC9,0x9F,0xBC,0xFB,0x06,0xA3,0xC6,0x9A,0x6A,0x9D,0xCA,0x52,0xD2,0x3B,0x61,
				0x60,0x73,0xE2,0x86,0x75,0xA2,0x3D,0x18,0x98,0x38,0xEF,0x1E,0x2E,0xE6,0x52,0xC0,
				0x13,0xEC,0xB4,0xAE,0xA9,0x06,0x11,0x23,0x24,0x97,0x5C,0x3C,0xD4,0x9B,0x83,0xBF,
				0xAC,0xCB,0xDD,0x7D,0x90,0xC4,0xBD,0x70,0x98,0x48,0x8E,0x9C,0x21,0x9A,0x73,0x72,
				0x4E,0xFF,0xD6,0xFA,0xE5,0x64,0x47,0x38,0xFA,0xA3,0x1A,0x4F,0xF5,0x5B,0xCC,0xC0,
				0xA1,0x51,0xAF,0x5F,0x0D,0xC8,0xB4,0xBD,0x45,0xBF,0x37,0xDF,0x36,0x5C,0x1A,0x65,
				0xE6,0x8C,0xFD,0xA7,0x6D,0x4D,0xA7,0x08,0xDF,0x1F,0xB2,0xBC,0x2E,0x4A,0x43,0x71),
		},
	}, {
		.group = MODP_2048_224, .opt_exp = 28, .public = {
			.exp_len = 28,
			.subgroup = chunk_from_chars(
				0x80,0x1C,0x0D,0x34,0xC5,0x8D,0x93,0xFE,0x99,0x71,0x77,0x10,0x1F,0x80,0x53,0x5A,
				0x47,0x38,0xCE,0xBC,0xBF,0x38,0x9A,0x99,0xB3,0x63,0x71,0xEB),
			.generator = chunk_from_chars(
				0xAC,0x40,0x32,0xEF,0x4F,0x2D,0x9A,0xE3,0x9D,0xF3,0x0B,0x5C,0x8F,0xFD,0xAC,0x50,
				0x6C,0xDE,0xBE,0x7B,0x89,0x99,0x8C,0xAF,0x74,0x86,0x6A,0x08,0xCF,0xE4,0xFF,0xE3,
				0xA6,0x82,0x4A,0x4E,0x10,0xB9,0xA6,0xF0,0xDD,0x92,0x1F,0x01,0xA7,0x0C,0x4A,0xFA,
				0xAB,0x73,0x9D,0x77,0x00,0xC2,0x9F,0x52,0xC5,0x7D,0xB1,0x7C,0x62,0x0A,0x86,0x52,
				0xBE,0x5E,0x90,0x01,0xA8,0xD6,0x6A,0xD7,0xC1,0x76,0x69,0x10,0x19,0x99,0x02,0x4A,
				0xF4,0xD0,0x27,0x27,0x5A,0xC1,0x34,0x8B,0xB8,0xA7,0x62,0xD0,0x52,0x1B,0xC9,0x8A,
				0xE2,0x47,0x15,0x04,0x22,0xEA,0x1E,0xD4,0x09,0x93,0x9D,0x54,0xDA,0x74,0x60,0xCD,
				0xB5,0xF6,0xC6,0xB2,0x50,0x71,0x7C,0xBE,0xF1,0x80,0xEB,0x34,0x11,0x8E,0x98,0xD1,
				0x19,0x52,0x9A,0x45,0xD6,0xF8,0x34,0x56,0x6E,0x30,0x25,0xE3,0x16,0xA3,0x30,0xEF,
				0xBB,0x77,0xA8,0x6F,0x0C,0x1A,0xB1,0x5B,0x05,0x1A,0xE3,0xD4,0x28,0xC8,0xF8,0xAC,
				0xB7,0x0A,0x81,0x37,0x15,0x0B,0x8E,0xEB,0x10,0xE1,0x83,0xED,0xD1,0x99,0x63,0xDD,
				0xD9,0xE2,0x63,0xE4,0x77,0x05,0x89,0xEF,0x6A,0xA2,0x1E,0x7F,0x5F,0x2F,0xF3,0x81,
				0xB5,0x39,0xCC,0xE3,0x40,0x9D,0x13,0xCD,0x56,0x6A,0xFB,0xB4,0x8D,0x6C,0x01,0x91,
				0x81,0xE1,0xBC,0xFE,0x94,0xB3,0x02,0x69,0xED,0xFE,0x72,0xFE,0x9B,0x6A,0xA4,0xBD,
				0x7B,0x5A,0x0F,0x1C,0x71,0xCF,0xFF,0x4C,0x19,0xC4,0x18,0xE1,0xF6,0xEC,0x01,0x79,
				0x81,0xBC,0x08,0x7F,0x2A,0x70,0x65,0xB3,0x84,0xB8,0x90,0xD3,0x19,0x1F,0x2B,0xFA),
			.prime = chunk_from_chars(
				0xAD,0x10,0x7E,0x1E,0x91,0x23,0xA9,0xD0,0xD6,0x60,0xFA,0xA7,0x95,0x59,0xC5,0x1F,
				0xA2,0x0D,0x64,0xE5,0x68,0x3B,0x9F,0xD1,0xB5,0x4B,0x15,0x97,0xB6,0x1D,0x0A,0x75,
				0xE6,0xFA,0x14,0x1D,0xF9,0x5A,0x56,0xDB,0xAF,0x9A,0x3C,0x40,0x7B,0xA1,0xDF,0x15,
				0xEB,0x3D,0x68,0x8A,0x30,0x9C,0x18,0x0E,0x1D,0xE6,0xB8,0x5A,0x12,0x74,0xA0,0xA6,
				0x6D,0x3F,0x81,0x52,0xAD,0x6A,0xC2,0x12,0x90,0x37,0xC9,0xED,0xEF,0xDA,0x4D,0xF8,
				0xD9,0x1E,0x8F,0xEF,0x55,0xB7,0x39,0x4B,0x7A,0xD5,0xB7,0xD0,0xB6,0xC1,0x22,0x07,
				0xC9,0xF9,0x8D,0x11,0xED,0x34,0xDB,0xF6,0xC6,0xBA,0x0B,0x2C,0x8B,0xBC,0x27,0xBE,
				0x6A,0x00,0xE0,0xA0,0xB9,0xC4,0x97,0x08,0xB3,0xBF,0x8A,0x31,0x70,0x91,0x88,0x36,
				0x81,0x28,0x61,0x30,0xBC,0x89,0x85,0xDB,0x16,0x02,0xE7,0x14,0x41,0x5D,0x93,0x30,
				0x27,0x82,0x73,0xC7,0xDE,0x31,0xEF,0xDC,0x73,0x10,0xF7,0x12,0x1F,0xD5,0xA0,0x74,
				0x15,0x98,0x7D,0x9A,0xDC,0x0A,0x48,0x6D,0xCD,0xF9,0x3A,0xCC,0x44,0x32,0x83,0x87,
				0x31,0x5D,0x75,0xE1,0x98,0xC6,0x41,0xA4,0x80,0xCD,0x86,0xA1,0xB9,0xE5,0x87,0xE8,
				0xBE,0x60,0xE6,0x9C,0xC9,0x28,0xB2,0xB9,0xC5,0x21,0x72,0xE4,0x13,0x04,0x2E,0x9B,
				0x23,0xF1,0x0B,0x0E,0x16,0xE7,0x97,0x63,0xC9,0xB5,0x3D,0xCF,0x4B,0xA8,0x0A,0x29,
				0xE3,0xFB,0x73,0xC1,0x6B,0x8E,0x75,0xB9,0x7E,0xF3,0x63,0xE2,0xFF,0xA3,0x1F,0x71,
				0xCF,0x9D,0xE5,0x38,0x4E,0x71,0xB8,0x1C,0x0A,0xC4,0xDF,0xFE,0x0C,0x10,0xE6,0x4F)
		},
	},{
		.group = MODP_2048_256, .opt_exp = 32, .public = {
			.exp_len = 32,
			.subgroup = chunk_from_chars(
				0x8C,0xF8,0x36,0x42,0xA7,0x09,0xA0,0x97,0xB4,0x47,0x99,0x76,0x40,0x12,0x9D,0xA2,
				0x99,0xB1,0xA4,0x7D,0x1E,0xB3,0x75,0x0B,0xA3,0x08,0xB0,0xFE,0x64,0xF5,0xFB,0xD3),
			.generator = chunk_from_chars(
				0x3F,0xB3,0x2C,0x9B,0x73,0x13,0x4D,0x0B,0x2E,0x77,0x50,0x66,0x60,0xED,0xBD,0x48,
				0x4C,0xA7,0xB1,0x8F,0x21,0xEF,0x20,0x54,0x07,0xF4,0x79,0x3A,0x1A,0x0B,0xA1,0x25,
				0x10,0xDB,0xC1,0x50,0x77,0xBE,0x46,0x3F,0xFF,0x4F,0xED,0x4A,0xAC,0x0B,0xB5,0x55,
				0xBE,0x3A,0x6C,0x1B,0x0C,0x6B,0x47,0xB1,0xBC,0x37,0x73,0xBF,0x7E,0x8C,0x6F,0x62,
				0x90,0x12,0x28,0xF8,0xC2,0x8C,0xBB,0x18,0xA5,0x5A,0xE3,0x13,0x41,0x00,0x0A,0x65,
				0x01,0x96,0xF9,0x31,0xC7,0x7A,0x57,0xF2,0xDD,0xF4,0x63,0xE5,0xE9,0xEC,0x14,0x4B,
				0x77,0x7D,0xE6,0x2A,0xAA,0xB8,0xA8,0x62,0x8A,0xC3,0x76,0xD2,0x82,0xD6,0xED,0x38,
				0x64,0xE6,0x79,0x82,0x42,0x8E,0xBC,0x83,0x1D,0x14,0x34,0x8F,0x6F,0x2F,0x91,0x93,
				0xB5,0x04,0x5A,0xF2,0x76,0x71,0x64,0xE1,0xDF,0xC9,0x67,0xC1,0xFB,0x3F,0x2E,0x55,
				0xA4,0xBD,0x1B,0xFF,0xE8,0x3B,0x9C,0x80,0xD0,0x52,0xB9,0x85,0xD1,0x82,0xEA,0x0A,
				0xDB,0x2A,0x3B,0x73,0x13,0xD3,0xFE,0x14,0xC8,0x48,0x4B,0x1E,0x05,0x25,0x88,0xB9,
				0xB7,0xD2,0xBB,0xD2,0xDF,0x01,0x61,0x99,0xEC,0xD0,0x6E,0x15,0x57,0xCD,0x09,0x15,
				0xB3,0x35,0x3B,0xBB,0x64,0xE0,0xEC,0x37,0x7F,0xD0,0x28,0x37,0x0D,0xF9,0x2B,0x52,
				0xC7,0x89,0x14,0x28,0xCD,0xC6,0x7E,0xB6,0x18,0x4B,0x52,0x3D,0x1D,0xB2,0x46,0xC3,
				0x2F,0x63,0x07,0x84,0x90,0xF0,0x0E,0xF8,0xD6,0x47,0xD1,0x48,0xD4,0x79,0x54,0x51,
				0x5E,0x23,0x27,0xCF,0xEF,0x98,0xC5,0x82,0x66,0x4B,0x4C,0x0F,0x6C,0xC4,0x16,0x59),
			.prime = chunk_from_chars(
				0x87,0xA8,0xE6,0x1D,0xB4,0xB6,0x66,0x3C,0xFF,0xBB,0xD1,0x9C,0x65,0x19,0x59,0x99,
				0x8C,0xEE,0xF6,0x08,0x66,0x0D,0xD0,0xF2,0x5D,0x2C,0xEE,0xD4,0x43,0x5E,0x3B,0x00,
				0xE0,0x0D,0xF8,0xF1,0xD6,0x19,0x57,0xD4,0xFA,0xF7,0xDF,0x45,0x61,0xB2,0xAA,0x30,
				0x16,0xC3,0xD9,0x11,0x34,0x09,0x6F,0xAA,0x3B,0xF4,0x29,0x6D,0x83,0x0E,0x9A,0x7C,
				0x20,0x9E,0x0C,0x64,0x97,0x51,0x7A,0xBD,0x5A,0x8A,0x9D,0x30,0x6B,0xCF,0x67,0xED,
				0x91,0xF9,0xE6,0x72,0x5B,0x47,0x58,0xC0,0x22,0xE0,0xB1,0xEF,0x42,0x75,0xBF,0x7B,
				0x6C,0x5B,0xFC,0x11,0xD4,0x5F,0x90,0x88,0xB9,0x41,0xF5,0x4E,0xB1,0xE5,0x9B,0xB8,
				0xBC,0x39,0xA0,0xBF,0x12,0x30,0x7F,0x5C,0x4F,0xDB,0x70,0xC5,0x81,0xB2,0x3F,0x76,
				0xB6,0x3A,0xCA,0xE1,0xCA,0xA6,0xB7,0x90,0x2D,0x52,0x52,0x67,0x35,0x48,0x8A,0x0E,
				0xF1,0x3C,0x6D,0x9A,0x51,0xBF,0xA4,0xAB,0x3A,0xD8,0x34,0x77,0x96,0x52,0x4D,0x8E,
				0xF6,0xA1,0x67,0xB5,0xA4,0x18,0x25,0xD9,0x67,0xE1,0x44,0xE5,0x14,0x05,0x64,0x25,
				0x1C,0xCA,0xCB,0x83,0xE6,0xB4,0x86,0xF6,0xB3,0xCA,0x3F,0x79,0x71,0x50,0x60,0x26,
				0xC0,0xB8,0x57,0xF6,0x89,0x96,0x28,0x56,0xDE,0xD4,0x01,0x0A,0xBD,0x0B,0xE6,0x21,
				0xC3,0xA3,0x96,0x0A,0x54,0xE7,0x10,0xC3,0x75,0xF2,0x63,0x75,0xD7,0x01,0x41,0x03,
				0xA4,0xB5,0x43,0x30,0xC1,0x98,0xAF,0x12,0x61,0x16,0xD2,0x27,0x6E,0x11,0x71,0x5F,
				0x69,0x38,0x77,0xFA,0xD7,0xEF,0x09,0xCA,0xDB,0x09,0x4A,0xE9,0x1E,0x1A,0x15,0x97)
		},
	},
};

/**
 * See header.
 */
void diffie_hellman_init()
{
	int i;

	if (lib->settings->get_bool(lib->settings,
					"%s.dh_exponent_ansi_x9_42", TRUE, lib->ns))
	{
		for (i = 0; i < countof(dh_params); i++)
		{
			dh_params[i].public.exp_len = dh_params[i].public.prime.len;
		}
	}
}

/**
 * Described in header.
 */
diffie_hellman_params_t *diffie_hellman_get_params(diffie_hellman_group_t group)
{
	int i;

	for (i = 0; i < countof(dh_params); i++)
	{
		if (dh_params[i].group == group)
		{
			if (!dh_params[i].public.exp_len)
			{
				if (!dh_params[i].public.subgroup.len &&
					lib->settings->get_bool(lib->settings,
									"%s.dh_exponent_ansi_x9_42", TRUE, lib->ns))
				{
					dh_params[i].public.exp_len = dh_params[i].public.prime.len;
				}
				else
				{
					dh_params[i].public.exp_len = dh_params[i].opt_exp;
				}
			}
			return &dh_params[i].public;
		}
	}
	return NULL;
}

/**
 * See header.
 */
bool diffie_hellman_group_is_ec(diffie_hellman_group_t group)
{
	switch (group)
	{
		case ECP_256_BIT:
		case ECP_384_BIT:
		case ECP_521_BIT:
		case ECP_192_BIT:
		case ECP_224_BIT:
		case ECP_224_BP:
		case ECP_256_BP:
		case ECP_384_BP:
		case ECP_512_BP:
			return TRUE;
		default:
			return FALSE;
	}
}

/**
 * See header.
 */
bool diffie_hellman_verify_value(diffie_hellman_group_t group, chunk_t value)
{
	diffie_hellman_params_t *params;
	bool valid = FALSE;

	switch (group)
	{
		case MODP_768_BIT:
		case MODP_1024_BIT:
		case MODP_1536_BIT:
		case MODP_2048_BIT:
		case MODP_3072_BIT:
		case MODP_4096_BIT:
		case MODP_6144_BIT:
		case MODP_8192_BIT:
		case MODP_1024_160:
		case MODP_2048_224:
		case MODP_2048_256:
			params = diffie_hellman_get_params(group);
			if (params)
			{
				valid = value.len == params->prime.len;
			}
			break;
		case ECP_192_BIT:
			valid = value.len == 48;
			break;
		case ECP_224_BIT:
		case ECP_224_BP:
			valid = value.len == 56;
			break;
		case ECP_256_BIT:
		case ECP_256_BP:
			valid = value.len == 64;
			break;
		case ECP_384_BIT:
		case ECP_384_BP:
			valid = value.len == 96;
			break;
		case ECP_512_BP:
			valid = value.len == 128;
			break;
		case ECP_521_BIT:
			valid = value.len == 132;
			break;
		case CURVE_25519:
			valid = value.len == 32;
			break;
		case CURVE_448:
			valid = value.len == 56;
			break;
		case NTRU_112_BIT:
		case NTRU_128_BIT:
		case NTRU_192_BIT:
		case NTRU_256_BIT:
		case NH_128_BIT:
			/* verification currently not supported, do in plugin */
			valid = FALSE;
			break;
		case NIST_PQC_NEWHOPE512CCA:
		case NIST_PQC_KYBER512:
		case NIST_PQC_NTRULPR4591761:
		case NIST_PQC_NTRUKEM443:
		case NIST_PQC_SIKEP503:
		case NIST_PQC_LEDAKEM128SLN02:
			/**
			 * TODO: 
			 * - We know the public-key size of these PQ ciphers,
			 *   some verifications could be done here.
			 **/
			valid = FALSE;
			break;
		case MODP_NULL:
		case MODP_CUSTOM:
			valid = TRUE;
			break;
		case MODP_NONE:
			/* fail */
			break;
		/* compile-warn unhandled groups, fail verification */
	}
	if (!valid)
	{
		DBG1(DBG_ENC, "invalid DH public value size (%zu bytes) for %N",
			 value.len, diffie_hellman_group_names, group);
	}
	return valid;
}
