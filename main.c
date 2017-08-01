/*
 * LibCSS - example1.c
 *
 * Compile this using a command such as:
 *  gcc -g -W -Wall -o example1 example1.c `pkg-config --cflags --libs libcss`
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

// for memory leaks checking memory
//#include <sys/resource.h>

/* The entire API is available through this header. */
#include "libcss/include/libcss/libcss.h"
#include "libcss/include/libcss/errors.h"
#include "libcss/include/libcss/stylesheet.h"

#include "libcss/include/testutils.h"
#include "libcss/include/dump.h"
#include "sys/times.h"
#include <stdlib.h>

/* This macro is used to silence compiler warnings about unused function
 * arguments. */
#define UNUSED(x) ((x) = (x))

static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;

void init()
{
    FILE* file;
    struct tms timeSample;
    char line[128];

    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    file = fopen("/proc/cpuinfo", "r");
    numProcessors = 0;
    while (fgets(line, 128, file) != NULL)
    {
        if (strncmp(line, "processor", 9) == 0) numProcessors++;
    }
    fclose(file);
}


double getCurrentValue()
{
    struct tms timeSample;
    clock_t now;
    double percent;

    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
            timeSample.tms_utime < lastUserCPU)
    {
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else
    {
        percent = (timeSample.tms_stime - lastSysCPU) +
            (timeSample.tms_utime - lastUserCPU);
        percent /= (now - lastCPU);
        percent /= numProcessors;
        percent *= 100;
    }
    lastCPU = now;
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    return percent;
}

/* Function declarations. */
static css_error resolve_url(void *pw,
        const char *base, lwc_string *rel, lwc_string **abs);
static void die(const char *text, css_error code);

static css_error node_name(void *pw, void *node,
        css_qname *qname);
static css_error node_classes(void *pw, void *node,
        lwc_string ***classes, uint32_t *n_classes);
static css_error node_id(void *pw, void *node,
        lwc_string **id);
static css_error named_ancestor_node(void *pw, void *node,
        const css_qname *qname,
        void **ancestor);
static css_error named_parent_node(void *pw, void *node,
        const css_qname *qname,
        void **parent);
static css_error named_sibling_node(void *pw, void *node,
        const css_qname *qname,
        void **sibling);
static css_error named_generic_sibling_node(void *pw, void *node,
        const css_qname *qname,
        void **sibling);
static css_error parent_node(void *pw, void *node, void **parent);
static css_error sibling_node(void *pw, void *node, void **sibling);
static css_error node_has_name(void *pw, void *node, 
        const css_qname *qname, 
        bool *match);
static css_error node_has_class(void *pw, void *node,
        lwc_string *name,
        bool *match);
static css_error node_has_id(void *pw, void *node,
        lwc_string *name,
        bool *match);
static css_error node_has_attribute(void *pw, void *node,
        const css_qname *qname,
        bool *match);
static css_error node_has_attribute_equal(void *pw, void *node,
        const css_qname *qname,
        lwc_string *value,
        bool *match);
static css_error node_has_attribute_dashmatch(void *pw, void *node,
        const css_qname *qname,
        lwc_string *value,
        bool *match);
static css_error node_has_attribute_includes(void *pw, void *node,
        const css_qname *qname,
        lwc_string *value,
        bool *match);
static css_error node_has_attribute_prefix(void *pw, void *node,
        const css_qname *qname,
        lwc_string *value,
        bool *match);
static css_error node_has_attribute_suffix(void *pw, void *node,
        const css_qname *qname,
        lwc_string *value,
        bool *match);
static css_error node_has_attribute_substring(void *pw, void *node,
        const css_qname *qname,
        lwc_string *value,
        bool *match);
static css_error node_is_root(void *pw, void *node, bool *match);
static css_error node_count_siblings(void *pw, void *node,
        bool same_name, bool after, int32_t *count);
static css_error node_is_empty(void *pw, void *node, bool *match);
static css_error node_is_link(void *pw, void *node, bool *match);
static css_error node_is_visited(void *pw, void *node, bool *match);
static css_error node_is_hover(void *pw, void *node, bool *match);
static css_error node_is_active(void *pw, void *node, bool *match);
static css_error node_is_focus(void *pw, void *node, bool *match);
static css_error node_is_enabled(void *pw, void *node, bool *match);
static css_error node_is_disabled(void *pw, void *node, bool *match);
static css_error node_is_checked(void *pw, void *node, bool *match);
static css_error node_is_target(void *pw, void *node, bool *match);
static css_error node_is_lang(void *pw, void *node,
        lwc_string *lang, bool *match);
static css_error node_presentational_hint(void *pw, void *node,
        uint32_t property, css_hint *hint);
static css_error ua_default_for_property(void *pw, uint32_t property,
        css_hint *hint);
static css_error compute_font_size(void *pw, const css_hint *parent,
        css_hint *size);
static css_error set_libcss_node_data(void *pw, void *n,
        void *libcss_node_data);
static css_error get_libcss_node_data(void *pw, void *n,
        void **libcss_node_data);
static css_error node_is_first_child(void *pw, void *n, bool *match);

/* Table of function pointers for the LibCSS Select API. */
static css_select_handler select_handler = {
    CSS_SELECT_HANDLER_VERSION_1,

    node_name,
    node_classes,
    node_id,
    named_ancestor_node,
    named_parent_node,
    named_sibling_node,
    named_generic_sibling_node,
    parent_node,
    sibling_node,
    node_has_name,
    node_has_class,
    node_has_id,
    node_has_attribute,
    node_has_attribute_equal,
    node_has_attribute_dashmatch,
    node_has_attribute_includes,
    node_has_attribute_prefix,
    node_has_attribute_suffix,
    node_has_attribute_substring,
    node_is_root,
    node_count_siblings,
    node_is_empty,
    node_is_link,
    node_is_visited,
    node_is_hover,
    node_is_active,
    node_is_focus,
    node_is_enabled,
    node_is_disabled,
    node_is_checked,
    node_is_target,
    node_is_lang,
    node_presentational_hint,
    ua_default_for_property,
    compute_font_size,
    set_libcss_node_data,
    get_libcss_node_data
};


int main(int argc, char **argv)
{
    char *data = NULL;

    if (argc >= 2)
    {
        for (int i = 1; i < argc; i++)
        {
            if (strncmp(argv[i], "file=", 5) == 0)
            {
                char *file_name = argv[i] + 5;
                printf("File: %s\n", file_name);

                FILE *fp;
                fp = fopen( file_name , "rt" );
                if (!fp) perror(file_name), exit(1);

                fseek( fp , 0L , SEEK_END);
                long lSize = ftell( fp ) - 1;
                rewind( fp );
                data = calloc( 1, lSize+1 );
                assert(data != NULL);
                memset(data, 0, lSize+1);
                if (!data) fclose(fp), fputs("memory alloc fails",stderr), exit(1);
                if (1 != fread( data , lSize, 1 , fp))
                    fclose(fp), free(data), fputs("entire read fails",stderr), exit(1);

                fclose(fp);

                break;
            }
            else if (strncmp(argv[i], "data=", 5) == 0)
            {
                data = strdup(argv[i] + 5);
                break;
            }
        }
    }
    else
    {
        printf("No input data set!\n");
        return 0;
    }

    if (data == NULL)
    {
        printf("Bad input data!\n");
        return 0;
    }

    size_t data_size = strlen(data);
    printf("INPUT DATA: %s\n", data);

    css_stylesheet_params params;
    params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
    params.level = CSS_LEVEL_3;
    params.charset = "UTF-8";
    params.url = "foo";
    params.title = "foo";
    params.allow_quirks = false;
    params.inline_style = false;
    params.resolve = resolve_url;
    params.resolve_pw = NULL;
    params.import = NULL;
    params.import_pw = NULL;
    params.color = NULL;
    params.color_pw = NULL;
    params.font = NULL;
    params.font_pw = NULL;

    css_stylesheet *sheet;
    size_t size;
    /* create a stylesheet */
    css_error code = css_stylesheet_create(&params, &sheet);
    if (code != CSS_OK)
        die("css_stylesheet_create", code);
    code = css_stylesheet_size(sheet, &size);
    if (code != CSS_OK)
        die("css_stylesheet_size", code);
    printf("created stylesheet, size %zu\n", size);

    /* parse some CSS source */
    code = css_stylesheet_append_data(sheet, (const uint8_t *) data,
            data_size);
    free(data);
    if (code != CSS_OK && code != CSS_NEEDDATA)
        die("css_stylesheet_append_data", code);
    printf("[DEBUG] data successfully appended\n");
    code = css_stylesheet_data_done(sheet);
    if (code != CSS_OK)
       die("css_stylesheet_data_done", code);
    printf("[DEBUG] data successfully done\n");
    code = css_stylesheet_size(sheet, &size);
    if (code != CSS_OK)
        die("css_stylesheet_size", code);
    printf("appended data, size now %zu\n", size);

    /*{ 
        printf("\n\nPARSED STYLESHEET: \n"); 
        size_t outlen = data_size * 8;
        char *out = malloc(outlen + 1);
        memset(out, 0, outlen+1);
        assert(out != NULL);
        dump_sheet(sheet, out, &outlen);
        printf("%s\n", out);
        free(out);
    }*/

    code = css_stylesheet_destroy(sheet);
    if (code != CSS_OK)
        die("css_stylesheet_destroy", code);
    return 0;
}


css_error resolve_url(void *pw,
        const char *base, lwc_string *rel, lwc_string **abs)
{
    UNUSED(pw);
    UNUSED(base);

    /* About as useless as possible */
    *abs = lwc_string_ref(rel);

    return CSS_OK;
}


void die(const char *text, css_error code)
{
    fprintf(stderr, "ERROR: %s: %i: %s\n",
            text, code, css_error_to_string(code));
    exit(EXIT_FAILURE);
}



/* Select handlers. Our "document tree" is actually just a single node, which is
 * a libwapcaplet string containing the element name. Therefore all the
 * functions below except those getting or testing the element name return empty
 * data or false attributes. */
css_error node_name(void *pw, void *n, css_qname *qname)
{
    lwc_string *node = n;

    UNUSED(pw);
    
    qname->name = lwc_string_ref(node);
    
    return CSS_OK;
}

css_error node_classes(void *pw, void *n,
        lwc_string ***classes, uint32_t *n_classes)
{
    UNUSED(pw);
    UNUSED(n);
    *classes = NULL;
    *n_classes = 0;
    return CSS_OK;
}

css_error node_id(void *pw, void *n, lwc_string **id)
{
    UNUSED(pw);
    UNUSED(n);
    *id = NULL;
    return CSS_OK;
}

css_error named_ancestor_node(void *pw, void *n,
        const css_qname *qname,
        void **ancestor)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    *ancestor = NULL;
    return CSS_OK;
}

css_error named_parent_node(void *pw, void *n,
        const css_qname *qname,
        void **parent)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    *parent = NULL;
    return CSS_OK;
}

css_error named_generic_sibling_node(void *pw, void *n,
        const css_qname *qname,
        void **sibling)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    *sibling = NULL;
    return CSS_OK;
}

css_error named_sibling_node(void *pw, void *n,
        const css_qname *qname,
        void **sibling)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    *sibling = NULL;
    return CSS_OK;
}

css_error parent_node(void *pw, void *n, void **parent)
{
    UNUSED(pw);
    UNUSED(n);
    *parent = NULL;
    return CSS_OK;
}

css_error sibling_node(void *pw, void *n, void **sibling)
{
    UNUSED(pw);
    UNUSED(n);
    *sibling = NULL;
    return CSS_OK;
}

css_error node_has_name(void *pw, void *n,
        const css_qname *qname,
        bool *match)
{
    lwc_string *node = n;
    UNUSED(pw);
    assert(lwc_string_caseless_isequal(node, qname->name, match) ==
            lwc_error_ok);
    return CSS_OK;
}

css_error node_has_class(void *pw, void *n,
        lwc_string *name,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(name);
    *match = false;
    return CSS_OK;
}

css_error node_has_id(void *pw, void *n,
        lwc_string *name,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(name);
    *match = false;
    return CSS_OK;
}

css_error node_has_attribute(void *pw, void *n,
        const css_qname *qname,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    *match = false;
    return CSS_OK;
}

css_error node_has_attribute_equal(void *pw, void *n,
        const css_qname *qname,
        lwc_string *value,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    UNUSED(value);
    *match = false;
    return CSS_OK;
}

css_error node_has_attribute_dashmatch(void *pw, void *n,
        const css_qname *qname,
        lwc_string *value,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    UNUSED(value);
    *match = false;
    return CSS_OK;
}

css_error node_has_attribute_includes(void *pw, void *n,
        const css_qname *qname,
        lwc_string *value,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    UNUSED(value);
    *match = false;
    return CSS_OK;
}

css_error node_has_attribute_prefix(void *pw, void *n,
        const css_qname *qname,
        lwc_string *value,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    UNUSED(value);
    *match = false;
    return CSS_OK;
}

css_error node_has_attribute_suffix(void *pw, void *n,
        const css_qname *qname,
        lwc_string *value,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    UNUSED(value);
    *match = false;
    return CSS_OK;
}

css_error node_has_attribute_substring(void *pw, void *n,
        const css_qname *qname,
        lwc_string *value,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(qname);
    UNUSED(value);
    *match = false;
    return CSS_OK;
}

css_error node_is_first_child(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_root(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_count_siblings(void *pw, void *n,
        bool same_name, bool after, int32_t *count)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(same_name);
    UNUSED(after);
    *count = 1;
    return CSS_OK;
}

css_error node_is_empty(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_link(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_visited(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_hover(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_active(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_focus(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_enabled(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_disabled(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_checked(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}

css_error node_is_target(void *pw, void *n, bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    *match = false;
    return CSS_OK;
}


css_error node_is_lang(void *pw, void *n,
        lwc_string *lang,
        bool *match)
{
    UNUSED(pw);
    UNUSED(n);
    UNUSED(lang);
    *match = false;
    return CSS_OK;
}

css_error node_presentational_hint(void *pw, void *node,
        uint32_t property, css_hint *hint)
{
    UNUSED(pw);
    UNUSED(node);
    UNUSED(property);
    UNUSED(hint);
    //return CSS_PROPERTY_NOT_SET;
    return CSS_OK;
}

css_error ua_default_for_property(void *pw, uint32_t property, css_hint *hint)
{
    UNUSED(pw);

    if (property == CSS_PROP_COLOR) {
        hint->data.color = 0x00000000;
        hint->status = CSS_COLOR_COLOR;
    } else if (property == CSS_PROP_FONT_FAMILY) {
        hint->data.strings = NULL;
        hint->status = CSS_FONT_FAMILY_SANS_SERIF;
    } else if (property == CSS_PROP_QUOTES) {
        /* Not exactly useful :) */
        hint->data.strings = NULL;
        hint->status = CSS_QUOTES_NONE;
    } else if (property == CSS_PROP_VOICE_FAMILY) {
        /** \todo Fix this when we have voice-family done */
        hint->data.strings = NULL;
        hint->status = 0;
    } else {
        return CSS_INVALID;
    }

    return CSS_OK;
}

css_error compute_font_size(void *pw, const css_hint *parent, css_hint *size)
{
    static css_hint_length sizes[] = {
        { FLTTOFIX(6.75), CSS_UNIT_PT },
        { FLTTOFIX(7.50), CSS_UNIT_PT },
        { FLTTOFIX(9.75), CSS_UNIT_PT },
        { FLTTOFIX(12.0), CSS_UNIT_PT },
        { FLTTOFIX(13.5), CSS_UNIT_PT },
        { FLTTOFIX(18.0), CSS_UNIT_PT },
        { FLTTOFIX(24.0), CSS_UNIT_PT }
    };
    const css_hint_length *parent_size;

    UNUSED(pw);

    /* Grab parent size, defaulting to medium if none */
    if (parent == NULL) {
        parent_size = &sizes[CSS_FONT_SIZE_MEDIUM - 1];
    } else {
        assert(parent->status == CSS_FONT_SIZE_DIMENSION);
        assert(parent->data.length.unit != CSS_UNIT_EM);
        assert(parent->data.length.unit != CSS_UNIT_EX);
        parent_size = &parent->data.length;
    }

    assert(size->status != CSS_FONT_SIZE_INHERIT);

    if (size->status < CSS_FONT_SIZE_LARGER) {
        /* Keyword -- simple */
        size->data.length = sizes[size->status - 1];
    } else if (size->status == CSS_FONT_SIZE_LARGER) {
        /** \todo Step within table, if appropriate */
        size->data.length.value = 
                FMUL(parent_size->value, FLTTOFIX(1.2));
        size->data.length.unit = parent_size->unit;
    } else if (size->status == CSS_FONT_SIZE_SMALLER) {
        /** \todo Step within table, if appropriate */
        size->data.length.value = 
                FMUL(parent_size->value, FLTTOFIX(1.2));
        size->data.length.unit = parent_size->unit;
    } else if (size->data.length.unit == CSS_UNIT_EM ||
            size->data.length.unit == CSS_UNIT_EX) {
        size->data.length.value = 
            FMUL(size->data.length.value, parent_size->value);

        if (size->data.length.unit == CSS_UNIT_EX) {
            size->data.length.value = FMUL(size->data.length.value,
                    FLTTOFIX(0.6));
        }

        size->data.length.unit = parent_size->unit;
    } else if (size->data.length.unit == CSS_UNIT_PCT) {
        size->data.length.value = FDIV(FMUL(size->data.length.value,
                parent_size->value), FLTTOFIX(100));
        size->data.length.unit = parent_size->unit;
    }

    size->status = CSS_FONT_SIZE_DIMENSION;

    return CSS_OK;
}

static css_error set_libcss_node_data(void *pw, void *n,
        void *libcss_node_data)
{
    UNUSED(pw);
    UNUSED(n);

    /* Since we're not storing it, ensure node data gets deleted */
    css_libcss_node_data_handler(&select_handler, CSS_NODE_DELETED,
            pw, n, NULL, libcss_node_data);

    return CSS_OK;
}

static css_error get_libcss_node_data(void *pw, void *n,
        void **libcss_node_data)
{
    UNUSED(pw);
    UNUSED(n);
    *libcss_node_data = NULL;

    return CSS_OK;
}


