//
// Created by Софья on 15.05.17.
//
#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

css_error css__parse_hyphens(css_language *c,
        const parserutils_vector *vector, int *ctx,
        css_style *result)
{
    int orig_ctx = *ctx;
    css_error error;
    const css_token *token;
    bool match;

    token = parserutils_vector_iterate(vector, ctx);
    if ((token == NULL) || ((token->type != CSS_TOKEN_IDENT))) {
        *ctx = orig_ctx;
        return CSS_INVALID;
    }

    if ((lwc_string_caseless_isequal(token->idata, c->strings[INHERIT], &match) == lwc_error_ok && match)) {
            error = css_stylesheet_style_inherit(result, CSS_PROP_HYPHENS);
    } else if ((lwc_string_caseless_isequal(token->idata, c->strings[AUTO], &match) == lwc_error_ok && match)) {
            error = css__stylesheet_style_appendOPV(result, CSS_PROP_HYPHENS, 0,HYPHENS_AUTO);
    } else if ((lwc_string_caseless_isequal(token->idata, c->strings[MANUAL], &match) == lwc_error_ok && match)) {
            error = css__stylesheet_style_appendOPV(result, CSS_PROP_HYPHENS, 0,HYPHENS_MANUAL);
    } else if ((lwc_string_caseless_isequal(token->idata, c->strings[NONE], &match) == lwc_error_ok && match)) {
            error = css__stylesheet_style_appendOPV(result, CSS_PROP_HYPHENS, 0,HYPHENS_NONE);
    } else if ((lwc_string_caseless_isequal(token->idata, c->strings[UNSET], &match) == lwc_error_ok && match)) {
            error = css__stylesheet_style_appendOPV(result, CSS_PROP_HYPHENS, 0,HYPHENS_UNSET);
    } else if ((lwc_string_caseless_isequal(token->idata, c->strings[INITIAL], &match) == lwc_error_ok && match)) {
            error = css__stylesheet_style_appendOPV(result, CSS_PROP_HYPHENS, 0,HYPHENS_INITIAL);
    } else {
        error = CSS_INVALID;
    }

    if (error != CSS_OK)
        *ctx = orig_ctx;
    
    return error;
}
