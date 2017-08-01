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
 * Parse border_radius_side
 *
 * \param c       Parsing context
 * \param vector  Vector of tokens to process
 * \param ctx     Pointer to vector iteration context
 * \param op      Opcode to parse for
 * \param result  Pointer to location to receive resulting style
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion,
 *         CSS_INVALID if the input is not valid
 *
 * Post condition: \a *ctx is updated with the next token to process
 *                 If the input is invalid, then \a *ctx remains unchanged.
 */

css_error css__parse_border_radius_side(css_language *c,
                                        const parserutils_vector *vector, int *ctx,
                                        css_style *result, enum css_properties_e op)
{
    int orig_ctx = *ctx;
    int prev_ctx = orig_ctx;

    css_error error = CSS_OK;
    const css_token *token;
    bool match;

    token = parserutils_vector_iterate(vector, ctx);
    if (token == NULL)
        return CSS_INVALID;

    if (token->type == CSS_TOKEN_IDENT &&
        lwc_string_caseless_isequal(
                token->idata, c->strings[INHERIT],
                &match) == lwc_error_ok && match) {
        error = css_stylesheet_style_inherit(result, op);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }
    } else {
        typedef struct side_value {
            bool has_dimension;
            css_fixed length;
            uint32_t unit; // valid if has_dimension == true;
        } side_value;

        side_value side_values[2];
        uint32_t side_count = 0;

        /* Attempt to parse up to 2 values */
        do {
            if ((token != NULL) && is_css_inherit(c, token)) {
                *ctx = orig_ctx;
                return CSS_INVALID;
            }

            if (token->type == CSS_TOKEN_PERCENTAGE || token->type == CSS_TOKEN_DIMENSION) {
                side_values[side_count].has_dimension = true;
                *ctx = prev_ctx;

                error = css__parse_unit_specifier(c, vector, ctx, UNIT_PX, &side_values[side_count].length,
                                                  &side_values[side_count].unit);
                if (error == CSS_OK) {
                    if (side_values[side_count].unit & UNIT_ANGLE ||
                            side_values[side_count].unit & UNIT_TIME ||
                            side_values[side_count].unit & UNIT_FREQ) {
                        *ctx = orig_ctx;
                        return CSS_INVALID;
                    }

                    if (side_values[side_count].length < 0) {
                        *ctx = orig_ctx;
                        return CSS_INVALID;
                    }
                }
            } else if (token->type == CSS_TOKEN_NUMBER) {
                side_values[side_count].has_dimension = false;
                size_t consumed = 0;
                side_values[side_count].length = css__number_from_lwc_string(token->idata, true, &consumed);
                /* Invalid if there are trailing characters */
                if (consumed != lwc_string_length(token->idata)) {
                    *ctx = orig_ctx;
                    return CSS_INVALID;
                }
                if (side_values[side_count].length < 0) {
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

            side_count++;

            consumeWhitespace(vector, ctx);
            prev_ctx = *ctx;
            token = parserutils_vector_iterate(vector, ctx);

        } while ((token != NULL) && (side_count < 2));

        if (side_count > 2) {
            *ctx = orig_ctx;
            return CSS_INVALID;
        }

        // append result
        error = css__stylesheet_style_appendOPV(result, op, 0, BORDER_RADIUS_SET);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }

#define SIDE_VALUE_APPEND(INDEX)     \
        if (side_values[INDEX].has_dimension) {     \
            error = css__stylesheet_style_vappend(result, 3, BORDER_RADIUS_DIMENSION_VALUE, side_values[INDEX].length, side_values[INDEX].unit);     \
            if (error != CSS_OK) {     \
                *ctx = orig_ctx;     \
                return error;     \
            }     \
        } else {     \
            error = css__stylesheet_style_vappend(result, 2, BORDER_RADIUS_NUMBER_VALUE, side_values[INDEX].length);     \
            if (error != CSS_OK) {     \
                *ctx = orig_ctx;     \
                return error;     \
            }     \
        }

        SIDE_VALUE_APPEND(0);

        if (side_count == 2) {
            SIDE_VALUE_APPEND(1);
        }

#undef SIDE_VALUE_APPEND

        error = css__stylesheet_style_append(result, BORDER_RADIUS_END);
        if (error != CSS_OK) {
            *ctx = orig_ctx;
            return error;
        }
    }
    return CSS_OK;
}