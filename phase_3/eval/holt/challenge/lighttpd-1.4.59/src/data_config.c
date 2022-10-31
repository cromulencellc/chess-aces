#include "first.h"

#include "array.h"
#include "configfile.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_PCRE_H
#include <pcre.h>
#endif

__attribute_cold__
static data_unset *data_config_copy(const data_unset *s) {
	data_config *src = (data_config *)s;
	data_config *ds = data_config_init();

	ds->comp = src->comp;
	if (!buffer_is_empty(&src->key)) {
		buffer_copy_buffer(&ds->key, &src->key);
		ds->comp_key = ds->key.ptr + (src->comp_key - src->key.ptr);
	}
	buffer_copy_buffer(&ds->comp_tag, &src->comp_tag);
	array_copy_array(ds->value, src->value);
	return (data_unset *)ds;
}

__attribute_cold__
static void data_config_free(data_unset *d) {
	data_config *ds = (data_config *)d;

	free(ds->key.ptr);
	free(ds->comp_tag.ptr);

	array_free(ds->value);
	vector_config_weak_clear(&ds->children);

	free(ds->string.ptr);
#ifdef HAVE_PCRE_H
	if (ds->regex) pcre_free(ds->regex);
	if (ds->regex_study) pcre_free(ds->regex_study);
#endif

	free(d);
}

__attribute_cold__
static int data_config_insert_dup(data_unset *dst, data_unset *src) {
	UNUSED(dst);

	src->fn->free(src);

	return 0;
}

__attribute_cold__
static void data_config_print(const data_unset *d, int depth) {
	data_config *ds = (data_config *)d;
	array *a = (array *)ds->value;
	size_t i;
	size_t maxlen;

	if (0 == ds->context_ndx) {
		fprintf(stdout, "config {\n");
	}
	else {
		if (ds->cond != CONFIG_COND_ELSE) {
			fprintf(stdout, "%s {\n", ds->comp_key);
		} else {
			fprintf(stdout, "{\n");
		}
		array_print_indent(depth + 1);
		fprintf(stdout, "# block %d\n", ds->context_ndx);
	}
	depth ++;

	maxlen = array_get_max_key_length(a);
	for (i = 0; i < a->used; i ++) {
		data_unset *du = a->data[i];
		size_t len = buffer_string_length(&du->key);
		size_t j;

		array_print_indent(depth);
		fprintf(stdout, "%s", du->key.ptr);
		for (j = maxlen - len; j > 0; j --) {
			fprintf(stdout, " ");
		}
		fprintf(stdout, " = ");
		du->fn->print(du, depth);
		fprintf(stdout, "\n");
	}

	fprintf(stdout, "\n");
	for (i = 0; i < ds->children.used; i ++) {
		data_config *dc = ds->children.data[i];

		/* only the 1st block of chaining */
		if (NULL == dc->prev) {
			fprintf(stdout, "\n");
			array_print_indent(depth);
			dc->fn->print((data_unset *) dc, depth);
			fprintf(stdout, "\n");
		}
	}

	depth --;
	array_print_indent(depth);
	fprintf(stdout, "}");
	if (0 != ds->context_ndx) {
		if (ds->cond != CONFIG_COND_ELSE) {
			fprintf(stdout, " # end of %s", ds->comp_key);
		} else {
			fprintf(stdout, " # end of else");
		}
	}

	if (ds->next) {
		fprintf(stdout, "\n");
		array_print_indent(depth);
		fprintf(stdout, "else ");
		ds->next->fn->print((data_unset *)ds->next, depth);
	}
}

data_config *data_config_init(void) {
	static const struct data_methods fn = {
		data_config_copy,
		data_config_free,
		data_config_insert_dup,
		data_config_print,
	};
	data_config *ds;

	ds = calloc(1, sizeof(*ds));
	force_assert(ds);

	ds->comp_key = "";
	ds->value = array_init(4);
	vector_config_weak_init(&ds->children);

	ds->type = TYPE_CONFIG;
	ds->fn = &fn;

	return ds;
}

int data_config_pcre_compile(data_config *dc) {
#ifdef HAVE_PCRE_H
    /* (use fprintf() on error, as this is called from configparser.y) */
    const char *errptr;
    int erroff, captures;

    if (dc->regex) pcre_free(dc->regex);
    if (dc->regex_study) pcre_free(dc->regex_study);

    dc->regex = pcre_compile(dc->string.ptr, 0, &errptr, &erroff, NULL);
    if (NULL == dc->regex) {
        fprintf(stderr, "parsing regex failed: %s -> %s at offset %d\n",
                dc->string.ptr, errptr, erroff);
        return 0;
    }

    dc->regex_study = pcre_study(dc->regex, 0, &errptr);
    if (NULL == dc->regex_study && errptr != NULL) {
        fprintf(stderr, "studying regex failed: %s -> %s\n",
                dc->string.ptr, errptr);
        return 0;
    }

    erroff = pcre_fullinfo(dc->regex, dc->regex_study, PCRE_INFO_CAPTURECOUNT,
                           &captures);
    if (0 != erroff) {
        fprintf(stderr, "getting capture count for regex failed: %s\n",
                dc->string.ptr);
        return 0;
    } else if (captures > 9) {
        fprintf(stderr, "Too many captures in regex, use (?:...) instead of (...): %s\n",
                dc->string.ptr);
        return 0;
    }
    return 1;
#else
    fprintf(stderr, "can't handle '%s' as you compiled without pcre support. \n"
                    "(perhaps just a missing pcre-devel package ?) \n",
                    dc->comp_key);
    return 0;
#endif
}
