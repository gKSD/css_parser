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
 * Parse background-size
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

css_error css__parse_background_size(css_language *c,
        const parserutils_vector *vector, int *ctx,
        css_style *result)
{
    int orig_ctx = *ctx;
    css_error error;
    const css_token *token;
    bool match;

    /* [length | percentage | IDENT(auto)]{1,2}
	 * | IDENT(inherit)
     * | IDENT(cover)
     * | IDENT(contain)
     */
    token = parserutils_vector_iterate(vector, ctx);
    if (token == NULL) {
        *ctx = orig_ctx;
        return CSS_INVALID;
    }

    if (token->type == CSS_TOKEN_IDENT &&
        lwc_string_caseless_isequal(
                 token->idata, c->strings[INHERIT],
                 &match) == lwc_error_ok && match) {
        error = css_stylesheet_style_inherit(result, CSS_PROP_BACKGROUND_SIZE);
    } else if (token->type == CSS_TOKEN_IDENT &&
               lwc_string_caseless_isequal(
                       token->idata, c->strings[COVER],
                       &match) == lwc_error_ok && match) {
        error = css__stylesheet_style_appendOPV(result, CSS_PROP_BACKGROUND_SIZE, 0,BACKGROUND_SIZE_COVER);
    } else if (token->type == CSS_TOKEN_IDENT &&
               lwc_string_caseless_isequal(
                       token->idata, c->strings[CONTAIN],
                       &match) == lwc_error_ok && match) {
        error = css__stylesheet_style_appendOPV(result, CSS_PROP_BACKGROUND_SIZE, 0,BACKGROUND_SIZE_CONTAIN);
    }
    else {
        int values_count = 0;
        css_code_t items[5] = {0};
        int items_count = 0;
        int i = 0;
        int prev_ctx = orig_ctx;

        while ((token != NULL) && (*ctx != prev_ctx)) {

            if (values_count >= 2) { // only 2 items are valid
                *ctx = orig_ctx;
                return CSS_INVALID;
            }

            if (token->type == CSS_TOKEN_IDENT &&
                lwc_string_caseless_isequal(
                        token->idata, c->strings[AUTO],
                        &match) == lwc_error_ok && match) {
                items[items_count++] = BACKGROUND_SIZE_AUTO;

            } else if (token->type == CSS_TOKEN_DIMENSION ||
                        token->type == CSS_TOKEN_PERCENTAGE) {
                css_fixed length = 0;
                uint32_t unit = 0;
                *ctx = prev_ctx;
                error = css__parse_unit_specifier(c, vector, ctx,
                                UNIT_PX, &length, &unit);
                if (error != CSS_OK) {
                    *ctx = orig_ctx;
                    return error;
                }

                if (unit&UNIT_ANGLE || unit&UNIT_TIME || unit&UNIT_FREQ) {
                    *ctx = orig_ctx;
                    return CSS_INVALID;
                }

                if (length < 0) {
                    *ctx = orig_ctx;
                    return CSS_INVALID;
                }


                items[items_count++] = BACKGROUND_SIZE_VALUE;
                items[items_count++] = length;
                items[items_count++] = unit;
            } else {
                *ctx = orig_ctx;
                return CSS_INVALID;
            }

            prev_ctx = *ctx;

            consumeWhitespace(vector, ctx);
            token = parserutils_vector_iterate(vector, ctx);

            ++values_count;
        }


        error = css__stylesheet_style_appendOPV(result, CSS_PROP_BACKGROUND_SIZE, 0, 0);

        for (; i < items_count; ++i) {
            error = css__stylesheet_style_append(result, items[i]);
            if (error != CSS_OK) {
                *ctx = orig_ctx;
                return error;
            }
        }

        error = css__stylesheet_style_append(result, BACKGROUND_SIZE_END);

    }

    if (error != CSS_OK)
        *ctx = orig_ctx;

    return error;
}
