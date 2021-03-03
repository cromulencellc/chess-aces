#ifndef __HTTP__
#define __HTTP__

#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "stream.h"

enum tag_name {
    text = 0,
    html,
    body,
    head_tag,
    li,
    link_tag,
    p_tag,
    source,
    track,
    audio,
    meta,
    script,
    a_tag,
    input,
    div_tag,
    form,
    hr,
    h3,
    span,
    sup,
    cite,
    ol,
    style,
    img,
    ul,
    td,
    i_tag,
    tr_tag,
    table,
    title,
    h2,
    small,
    code,
    tbody,
    th,
    b_tag,
    noscript,
    h1,
    blockquote,
    br,
    pre,
    label,
    abbr,
    unknown
};

typedef struct attribute {
    char *param;
    char *value;

    struct attribute *next;
} attribute;

/// Name will be the ascii representation of the name or a pointer to the text
///   data underneath a tag
typedef struct tag {
    char *name;
    enum tag_name tn;

    attribute *attribs;

    int component_count;
    struct tag ** subcomps;

} tag;

void print_tag( stream *s, tag *tg, int depth );
int cut_all_tags( tag *tg, char *element);
void free_tag( tag * t );
int parse_tag( char *html_data, int length, tag *new_tag);

#endif