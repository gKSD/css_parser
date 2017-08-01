/*
 * This file is part of LibCSS.
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

/**
 * Parse border-radius
 *
 * \param c	  Parsing context
 * \param vector  Vector of tokens to process
 * \param ctx	  Pointer to vector iteration context
 * \param result  Pointer to location to receive resulting style
 * \return CSS_OK on success,
 *	   CSS_NOMEM on memory exhaustion,
 *	   CSS_INVALID if the input is not valid
 *
 * Post condition: \a *ctx is updated with the next token to process
 *		   If the input is invalid, then \a *ctx remains unchanged.
 */

static
void _set_indexes(int side_count, int pos,
                         int *top_left,
                         int *top_right,
                         int *bottom_left,
                         int *bottom_right)
{
    switch (side_count) {
        case 1:
            top_left[pos] = 0;
            top_right[pos] = 0;
            bottom_left[pos] = 0;
            bottom_right[pos] = 0;
            break;
        case 2:
            top_left[pos] = 0;
            top_right[pos] = 1;
            bottom_left[pos] = 1;
            bottom_right[pos] = 0;
            break;
        case 3:
            top_left[pos] = 0;
            top_right[pos] = 1;
            bottom_left[pos] = 1;
            bottom_right[pos] = 2;
            break;
        case 4:
            top_left[pos] = 0;
            top_right[pos] = 1;
            bottom_left[pos] = 3;
            bottom_right[pos] = 2;
            break;
    }
}

css_error css__parse_border_radius(css_language *c,
                                   const parserutils_vector *vector, int *ctx,
                                   css_style *result)
{
    int orig_ctx = *ctx;
    int prev_ctx = orig_ctx;

    css_error error = CSS_OK;
    const css_token *token;
    bool match;

    typedef struct side_value {
        bool has_dimension;
        css_fixed length;
        uint32_t unit; // valid if has_dimension == true;
    } side_value;

    side_value side_value_first[4];
    uint32_t side_count_first = 0;
    side_value side_value_second[4];
    uint32_t side_count_second = 0;

    bool has_slash = false;
    uint32_t *side_count = &side_count_first;
    side_value *side_value_current = side_value_first;

    // store indexes in original arrays
    int top_left[2];
    int top_right[2];
    int bottom_left[2];
    int bottom_right[2];
    int side_items_count = 0;

    token = parserutils_vector_iterate(vector, ctx);
    if (token == NULL)
        return CSS_INVALID;

    if (token->type == CSS_TOKEN_IDENT &&
             lwc_string_caseless_isequal(
                     token->idata, c->strings[INHERIT],
                     &match) == lwc_error_ok && match) {
        error = css_stylesheet_style_inherit(result, CSS_PROP_BORDER_TOP_LEFT_RADIUS);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }

        error = css_stylesheet_style_inherit(result, CSS_PROP_BORDER_TOP_RIGHT_RADIUS);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }

        error = css_stylesheet_style_inherit(result, CSS_PROP_BORDER_BOTTOM_LEFT_RADIUS);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }

        error = css_stylesheet_style_inherit(result, CSS_PROP_BORDER_BOTTOM_RIGHT_RADIUS);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }

        return CSS_OK;
    }

    /* Attempt to parse up to 4 values */
    do {
        if ((token != NULL) && is_css_inherit(c, token)) {
            *ctx = orig_ctx;
            return CSS_INVALID;
        }

        if (token->type == CSS_TOKEN_PERCENTAGE || token->type == CSS_TOKEN_DIMENSION) {
            side_value_current[*side_count].has_dimension = true;

            *ctx = prev_ctx;

            error = css__parse_unit_specifier(c, vector, ctx, UNIT_PX, &side_value_current[*side_count].length,
                                              &side_value_current[*side_count].unit);
            if (error == CSS_OK) {
                if (side_value_current[*side_count].unit & UNIT_ANGLE ||
                        side_value_current[*side_count].unit & UNIT_TIME ||
                        side_value_current[*side_count].unit & UNIT_FREQ) {
                    *ctx = orig_ctx;
                    return CSS_INVALID;
                }

                if (side_value_current[*side_count].length < 0) {
                    *ctx = orig_ctx;
                    return CSS_INVALID;
                }
            }
        } else if (token->type == CSS_TOKEN_NUMBER) {
            side_value_current[*side_count].has_dimension = false;

            size_t consumed = 0;
            side_value_current[*side_count].length = css__number_from_lwc_string(token->idata, true, &consumed);
            /* Invalid if there are trailing characters */
            if (consumed != lwc_string_length(token->idata)) {
                *ctx = orig_ctx;
                return CSS_INVALID;
            }
            if (side_value_current[*side_count].length < 0) {
                *ctx = orig_ctx;
                return CSS_INVALID;
            }
        }
        else {
            *ctx = orig_ctx;
            return CSS_INVALID;
        }

        if (error != CSS_OK)
            break;

        (*side_count)++;

        consumeWhitespace(vector, ctx);
        prev_ctx = *ctx;
        token = parserutils_vector_iterate(vector, ctx);

        if (!has_slash && token != NULL && tokenIsChar(token, '/')) {
            has_slash = true;
            side_count = &side_count_second;
            side_value_current = side_value_second;

            consumeWhitespace(vector, ctx);
            prev_ctx = *ctx;
            token = parserutils_vector_iterate(vector, ctx);
        }

    } while ((token != NULL) && ((*side_count) < 4));

    if (error != CSS_OK) {
        *ctx = orig_ctx;
        return error;
    }

    if (side_count_first > 4 || side_count_second > 4) {
        *ctx = orig_ctx;
        return CSS_INVALID;
    }

    _set_indexes(side_count_first,
                side_items_count++,
                top_left,
                top_right,
                bottom_left,
                bottom_right);


    if (has_slash) {
        _set_indexes(side_count_second,
                    side_items_count++,
                    top_left,
                    top_right,
                    bottom_left,
                    bottom_right);
    }

#define SIDE_APPEND(OP, SIDE_INDEXES)							\
	error = css__stylesheet_style_appendOPV(result, (OP), 0, BORDER_RADIUS_SET);	\
	if (error != CSS_OK) {                         \
        *ctx = orig_ctx;                         \
        return error;                         \
    }								\
    if (side_value_first[SIDE_INDEXES[0]].has_dimension) {								\
        error = css__stylesheet_style_append(result, BORDER_RADIUS_DIMENSION_VALUE);	\
        if (error != CSS_OK) {                         \
            *ctx = orig_ctx;                         \
            return error;                         \
        }                         \
        error = css__stylesheet_style_append(result, side_value_first[SIDE_INDEXES[0]].length);	\
        if (error != CSS_OK) {                         \
            *ctx = orig_ctx;                         \
            return error;                         \
        }								\
        error = css__stylesheet_style_append(result, side_value_first[SIDE_INDEXES[0]].unit);		\
        if (error != CSS_OK) {                         \
            *ctx = orig_ctx;                         \
            return error;                         \
        }								\
    } else {								\
        error = css__stylesheet_style_append(result, BORDER_RADIUS_NUMBER_VALUE);	\
        if (error != CSS_OK) {                         \
            *ctx = orig_ctx;                         \
            return error;                         \
        }                         \
        error = css__stylesheet_style_append(result, side_value_first[SIDE_INDEXES[0]].length);	\
        if (error != CSS_OK) {                         \
            *ctx = orig_ctx;                         \
            return error;                         \
        }								\
    }								\
    if (has_slash) {                                      \
        if (side_value_second[SIDE_INDEXES[1]].has_dimension) {								\
            error = css__stylesheet_style_append(result, BORDER_RADIUS_DIMENSION_VALUE);	\
            if (error != CSS_OK) {                         \
                *ctx = orig_ctx;                         \
                return error;                         \
            }                         \
            error = css__stylesheet_style_append(result, side_value_second[SIDE_INDEXES[1]].length);	\
            if (error != CSS_OK) {                         \
                *ctx = orig_ctx;                         \
                return error;                         \
            }								\
            error = css__stylesheet_style_append(result, side_value_second[SIDE_INDEXES[1]].unit);		\
            if (error != CSS_OK) {                         \
                *ctx = orig_ctx;                         \
                return error;                         \
            }								\
        } else {								\
            error = css__stylesheet_style_append(result, BORDER_RADIUS_NUMBER_VALUE);	\
            if (error != CSS_OK) {                         \
                *ctx = orig_ctx;                         \
                return error;                         \
            }                         \
            error = css__stylesheet_style_append(result, side_value_second[SIDE_INDEXES[1]].length);	\
            if (error != CSS_OK) {                         \
                *ctx = orig_ctx;                         \
                return error;                         \
            }								\
        }								\
    }                                      \
    error = css__stylesheet_style_append(result, BORDER_RADIUS_END);		\
	if (error != CSS_OK) {                         \
        *ctx = orig_ctx;                         \
        return error;                         \
    }

    SIDE_APPEND(CSS_PROP_BORDER_TOP_LEFT_RADIUS, top_left);
    SIDE_APPEND(CSS_PROP_BORDER_TOP_RIGHT_RADIUS, top_right);
    SIDE_APPEND(CSS_PROP_BORDER_BOTTOM_LEFT_RADIUS, bottom_left);
    SIDE_APPEND(CSS_PROP_BORDER_BOTTOM_RIGHT_RADIUS, bottom_right);

#undef SIDE_APPEND

    return CSS_OK;
}
