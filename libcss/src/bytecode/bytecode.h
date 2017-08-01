/*
 * This file is part of LibCSS.
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2008 John-Mark Bell <jmb@netsurf-browser.org>
 */

#ifndef css_bytecode_bytecode_h_
#define css_bytecode_bytecode_h_

#include <inttypes.h>
#include <stdio.h>

#include "../../include/libcss/types.h"
#include "../../include/libcss/properties.h"

typedef uint32_t css_code_t; 

typedef enum css_properties_e opcode_t;

enum flag {
	FLAG_IMPORTANT			= (1<<0),
	FLAG_INHERIT			= (1<<1)
};

enum media_features_flags {
	FLAG_DELIMITER_COMMA = (1<<0),
	FLAG_DELIMITER_AND   = (1<<1),
	FLAG_DELIMITER_OR    = (1<<2),
	FLAG_ONLY            = (1<<3),
	FLAG_NOT             = (1<<4),
};

typedef enum unit {
	UNIT_PX   = 0,
	UNIT_EX   = 1,
	UNIT_EM   = 2,
	UNIT_IN   = 3,
	UNIT_CM   = 4,
	UNIT_MM   = 5,
	UNIT_PT   = 6,
	UNIT_PC   = 7,

	UNIT_PCT  = (1 << 8),

	UNIT_ANGLE = (1 << 9),
	UNIT_DEG  = (1 << 9) + 1,
	UNIT_GRAD = (1 << 9) + 2,
	UNIT_RAD  = (1 << 9) + 3,

	UNIT_TIME = (1 << 10),
	UNIT_MS   = (1 << 10) + 1,
	UNIT_S    = (1 << 10) + 2,

	UNIT_FREQ = (1 << 11),
	UNIT_HZ   = (1 << 11) + 1,
	UNIT_KHZ  = (1 << 11) + 2,

	UNIT_DPI  = (1 << 12),
	UNIT_DPCM  = (1 << 12) + 1,
	UNIT_DPPX  = (1 << 12) + 2,
} unit;

typedef uint32_t colour;

typedef enum shape {
	SHAPE_RECT = 0
} shape;

static inline css_code_t buildOPV(uint16_t opcode, uint8_t flags, uint16_t value)
{
	return (opcode & 0x3ff) | (flags << 10) | ((value & 0x3fff) << 18);
}

static inline uint16_t getOpcode(css_code_t OPV)
{
	return (OPV & 0x3ff);
}

static inline uint8_t getFlags(css_code_t OPV)
{
	return ((OPV >> 10) & 0xff);
}

static inline uint16_t getValue(css_code_t OPV)
{
	return (OPV >> 18);
}

static inline bool isImportant(css_code_t OPV)
{
	return getFlags(OPV) & 0x1;
}

static inline bool isInherit(css_code_t OPV)
{
	return getFlags(OPV) & 0x2;
}

static inline void appendOpcode(css_code_t *OPV, uint16_t opcode)
{
	*OPV |= (opcode & 0x3ff);
}

static inline void appendValue(css_code_t *OPV, uint16_t value)
{
	*OPV |= ((value & 0x3fff) << 18);
}

static inline void appendFlags(css_code_t *OPV, uint8_t flags)
{
	*OPV |= (flags << 10);
}

#endif



