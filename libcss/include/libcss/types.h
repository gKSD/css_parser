/*
 * This file is part of LibCSS.
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2007 John-Mark Bell <jmb@netsurf-browser.org>
 */

#ifndef libcss_types_h_
#define libcss_types_h_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "../../../libwapcaplet/include/libwapcaplet/libwapcaplet.h"

#include "../libcss/fpmath.h"

/**
 * Source of charset information, in order of importance.
 * A client-dictated charset will override all others.
 * A document-specified charset will override autodetection or the default.
 */
typedef enum css_charset_source {
	CSS_CHARSET_DEFAULT          = 0,	/**< Default setting */
	CSS_CHARSET_REFERRED         = 1,	/**< From referring document */
	CSS_CHARSET_METADATA         = 2,	/**< From linking metadata */
	CSS_CHARSET_DOCUMENT         = 3,	/**< Defined in document */
	CSS_CHARSET_DICTATED         = 4	/**< Dictated by client */
} css_charset_source;

/**
 * Stylesheet language level -- defines parsing rules and supported properties
 */
typedef enum css_language_level {
	CSS_LEVEL_1                 = 0,	/**< CSS 1 */
	CSS_LEVEL_2                 = 1,	/**< CSS 2 */
	CSS_LEVEL_21                = 2,	/**< CSS 2.1 */
	CSS_LEVEL_3                 = 3,	/**< CSS 3 */
	CSS_LEVEL_DEFAULT           = CSS_LEVEL_21	/**< Default level */
} css_language_level;

/**
 * Stylesheet media types
 */
typedef enum css_media_type {
	CSS_MEDIA_AURAL                   = 0x0000,
	CSS_MEDIA_BRAILLE                 = 0x0001,
	CSS_MEDIA_EMBOSSED                = 0x0002,
	CSS_MEDIA_HANDHELD                = 0x0003,
	CSS_MEDIA_PRINT                   = 0x0004,
	CSS_MEDIA_PROJECTION              = 0x0005,
	CSS_MEDIA_SCREEN                  = 0x0006,
	CSS_MEDIA_SPEECH                  = 0x0007,
	CSS_MEDIA_TTY                     = 0x0008,
	CSS_MEDIA_TV                      = 0x0009,
	CSS_MEDIA_ALL                     = 0x000a,
	CSS_MEDIA_ASPECT_RATIO            = 0x000b,
	CSS_MEDIA_COLOR                   = 0x000c,
	CSS_MEDIA_COLOR_INDEX             = 0x000d,
	CSS_MEDIA_DEVICE_ASPECT_RATIO     = 0x000e,
	CSS_MEDIA_DEVICE_HEIGHT           = 0x000f,
	CSS_MEDIA_DEVICE_WIDTH            = 0x0010,
	CSS_MEDIA_GRID                    = 0x0011,
	CSS_MEDIA_HEIGHT                  = 0x0012,
	CSS_MEDIA_MAX_ASPECT_RATIO        = 0x0013,
	CSS_MEDIA_MAX_COLOR               = 0x0014,
	CSS_MEDIA_MAX_COLOR_INDEX         = 0x0015,
	CSS_MEDIA_MAX_DEVICE_ASPECT_RATIO = 0x0016,
	CSS_MEDIA_MAX_DEVICE_HEIGHT       = 0x0017,
	CSS_MEDIA_MAX_DEVICE_WIDTH        = 0x0018,
	CSS_MEDIA_MAX_HEIGHT              = 0x0019,
	CSS_MEDIA_MAX_MONOCHROME          = 0x001a,
	CSS_MEDIA_MAX_RESOLUTION          = 0x001b,
	CSS_MEDIA_MAX_WIDTH               = 0x001c,
	CSS_MEDIA_MIN_ASPECT_RATIO        = 0x001d,
	CSS_MEDIA_MIN_COLOR               = 0x001e,
	CSS_MEDIA_MIN_COLOR_INDEX         = 0x001f,
	CSS_MEDIA_MIN_DEVICE_ASPECT_RATIO = 0x0020,
	CSS_MEDIA_MIN_DEVICE_WIDTH        = 0x0021,
	CSS_MEDIA_MIN_DEVICE_HEIGHT       = 0x0022,
	CSS_MEDIA_MIN_HEIGHT              = 0x0023,
	CSS_MEDIA_MIN_MONOCHROME          = 0x0024,
	CSS_MEDIA_MIN_RESOLUTION          = 0x0025,
	CSS_MEDIA_MIN_WIDTH               = 0x0026,
	CSS_MEDIA_MONOCHROME              = 0x0027,
	CSS_MEDIA_ORIENTATION             = 0x0028,
	CSS_MEDIA_OVERFLOW_BLOCK          = 0x0029,
	CSS_MEDIA_OVERFLOW_INLINE         = 0x002a,
	CSS_MEDIA_RESOLUTION              = 0x002b,
	CSS_MEDIA_SCAN                    = 0x002c,
	CSS_MEDIA_UPDATE_FREQUENCY        = 0x002d,
	CSS_MEDIA_WIDTH                   = 0x002e,
	CSS_MEDIA_TOTAL                   = 0x002f
} css_media_type;

/**
 * Stylesheet origin
 */
typedef enum css_origin {
	CSS_ORIGIN_UA               = 0,	/**< User agent stylesheet */
	CSS_ORIGIN_USER             = 1,	/**< User stylesheet */
	CSS_ORIGIN_AUTHOR           = 2		/**< Author stylesheet */
} css_origin;

/** CSS colour -- AARRGGBB */
typedef uint32_t css_color;

/* CSS unit */
typedef enum css_unit {
	CSS_UNIT_PX                 = 0x0,
	CSS_UNIT_EX                 = 0x1,
	CSS_UNIT_EM                 = 0x2,
	CSS_UNIT_IN                 = 0x3,
	CSS_UNIT_CM                 = 0x4,
	CSS_UNIT_MM                 = 0x5,
	CSS_UNIT_PT                 = 0x6,
	CSS_UNIT_PC                 = 0x7,

	CSS_UNIT_PCT                = 0x8,	/* Percentage */

	CSS_UNIT_DEG                = 0x9,
	CSS_UNIT_GRAD               = 0xa,
	CSS_UNIT_RAD                = 0xb,

	CSS_UNIT_MS                 = 0xc,
	CSS_UNIT_S                  = 0xd,

	CSS_UNIT_HZ                 = 0xe,
	CSS_UNIT_KHZ                = 0xf,

	CSS_UNIT_DPI                = 0x10,
	CSS_UNIT_DPCM               = 0x11,
	CSS_UNIT_DPPX               = 0x12
} css_unit;

/**
 * Type of a qualified name
 */
typedef struct css_qname {
	/**
	 * Namespace URI:
	 *
	 * NULL for no namespace
	 * '*' for any namespace (including none)
	 * URI for a specific namespace
	 */
	lwc_string *ns;

	/**
	 * Local part of qualified name
	 */
	lwc_string *name;
} css_qname;

typedef struct css_stylesheet css_stylesheet;

typedef struct css_select_ctx css_select_ctx;

typedef struct css_computed_style css_computed_style;

typedef struct css_font_face css_font_face;

typedef struct css_font_face_src css_font_face_src;

typedef struct css_style css_media_query;

#ifdef __cplusplus
}
#endif

#endif
