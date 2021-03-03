#include "http.h"
#include <errno.h>

#define START 0
#define TAGNAME 1
#define ERROR 2
#define END 3
#define TAGNAME_END 4
#define TAGNAME_EAT_WHITESPACE 5
#define AFTER_TAGNAME 6
#define TERMINATE_SLASH 7
#define INSIDE_TAG 8
#define TERMINATE_TAG 9
#define ATTRIBUTE 10
#define ATTRIBUTE_WHITESPACE 11
#define ATTRIBUTE_EQUALS 12
#define ATTR_VALUE_START 13
#define ATTR_VALUE_INSIDE 14
#define ATTR_VALUE_END 15
#define INSIDE_TAG_TEXT 16
#define EVAL_TAG 17
#define HOMESTRETCH 18
#define CLOSE_CURRENT_TAG 19
#define CLOSE_COMMENT 20
#define OPEN_COMMENT_DASH_ONE 21
#define OPEN_COMMENT_DASH_TWO 22
#define CC_DASH_TWO 23
#define CC_BANG 24
#define END_COMMENT 25
#define DOCTYPE 26
#define DOCTYPE_SUBS 27
#define ATTR_VALUE_INSIDE_TIC 28
#define ATTRIBUTE_END_NULL_VALUE 29

int needs_closer( char *tag ){
    if ( !tag ) {
        return 1;
    }

    if ( !strcasecmp( tag, "meta") ) {
        return 0;
    } else if ( !strcasecmp( tag, "br") ) {
        return 0;
    } else if ( !strcasecmp( tag, "hr") ) {
        return 0;
    } else if ( !strcasecmp( tag, "link") ) {
        return 0;
    }

    return 1;
}

void free_tag( tag * t )
{
    attribute *walker = NULL;
    attribute *sw = NULL;

    if ( !t ) {
        return;
    }

    walker = t->attribs;

    while ( walker ) {
        sw = walker;

        walker = walker->next;

        free(sw->param);
        free(sw->value);

        sw->param = NULL;
        sw->value = NULL;

        free(sw);
        sw = NULL;
    }

    if ( t->name ) {
        free(t->name);
    }

    if ( t->subcomps ) {
        for ( int i = 0; i < t->component_count; i++ ) {
            free_tag( t->subcomps[i] );

            t->subcomps[i] = NULL;
        }

        free(t->subcomps);
    }

    free(t);

    return;
}

tag *create_text_tag( char *line )
{
    tag *nt = NULL;

    if ( !line ) {
        return NULL;
    }

    nt = calloc( 1, sizeof(tag) );

    if ( !nt ) {
        return NULL;
    }

    nt->name = strdup(line);
    nt->tn = text;

    return nt;
}

int add_component_tag( tag *base, tag *nt) 
{
    tag **tag_list = NULL;

    if ( !base || !nt ) {
        return -1;
    }

    base->component_count++;

    tag_list = calloc( 1, sizeof(tag*) * base->component_count);

    if ( !tag_list ) {
        return -1;
    }

    if ( base->subcomps ) {
        memcpy( tag_list, base->subcomps, sizeof(tag*) * (base->component_count - 1) );

        tag_list[base->component_count - 1] = nt;

        free(base->subcomps);
    } else {
        tag_list[0] = nt;
    }

    base->subcomps = tag_list;

    return 0;
}

int add_attribute( tag *base, char *attr, char *value)
{
    attribute *new_attrib = NULL;
    attribute *walker = NULL;

    /// The attribute value can be null
    if ( !base || !attr) {
        return -1;
    }

    new_attrib = calloc( 1, sizeof( attribute ) );

    if ( !new_attrib ) {
        return -1;
    }

    /// Base case

    if ( base->attribs == NULL ) {
        base->attribs = new_attrib;
    } else {
        walker = base->attribs;

        while ( walker->next ) {
            walker = walker->next;
        }

        walker->next = new_attrib;
    }

    new_attrib->param = attr;
    new_attrib->value = value;

    return 0;
}

void print_tag( stream *s, tag *tg, int depth )
{
    attribute *walker = NULL;

    if (!tg || !s) {
        return;
    }

    if ( tg->tn == text) {
        for ( int i = 0; i < depth; i++) {
            add_string( s, "\t");
        }

        add_string(s,  tg->name );
        add_string(s, "\n" );
        return;
    }

    for ( int i = 0; i < depth; i++) {
        add_string( s, "\t");
    }

    add_string( s, "<");
    add_string( s, tg->name);

    walker = tg->attribs;

    while ( walker ) {
        add_string( s, " ");
        add_string( s, walker->param);

        if ( walker->value) {
            add_string( s, "=\"");
            add_string( s, walker->value);
            add_string( s, "\"");
        }

        walker = walker->next;
    }

    add_string( s, ">\n");

    for ( int i = 0; i < tg->component_count; i++) {
        print_tag( s, tg->subcomps[i], depth+1);
    }

    if ( !needs_closer( tg->name) ) {
        return;
    }

    for ( int i = 0; i < depth; i++) {
        add_string( s, "\t");
    }

    add_string( s, "</");
    add_string( s, tg->name);
    add_string( s, ">\n");

    return;
}

int set_tag_type( tag *tg )
{
    if ( !tg ) {
        return -1;
    }

    if ( !strcasecmp( tg->name, "html" ) ) {
        tg->tn = html;
    } else if ( !strcasecmp( tg->name, "body" )) {
        tg->tn = body;
    } else if ( !strcasecmp( tg->name, "head" )) {
        tg->tn = head_tag;
    } else if ( !strcasecmp( tg->name, "meta" )) {
        tg->tn = meta;
    } else if ( !strcasecmp( tg->name, "li" )) {
        tg->tn = li;
    } else if ( !strcasecmp( tg->name, "script" )) {
        tg->tn = script;
    } else if ( !strcasecmp( tg->name, "link" )) {
        tg->tn = link_tag;
    } else if ( !strcasecmp( tg->name, "a" )) {
        tg->tn = a_tag;
    } else if ( !strcasecmp( tg->name, "input" )) {
        tg->tn = input;
    } else if ( !strcasecmp( tg->name, "div" )) {
        tg->tn = div_tag;
    } else if ( !strcasecmp( tg->name, "form" )) {
        tg->tn = form;
    } else if ( !strcasecmp( tg->name, "h3" )) {
        tg->tn = h3;
    } else if ( !strcasecmp( tg->name, "ul" )) {
        tg->tn = ul;
    } else if ( !strcasecmp( tg->name, "span" )) {
        tg->tn = span;
    } else if ( !strcasecmp( tg->name, "img" )) {
        tg->tn = img;
    } else if ( !strcasecmp( tg->name, "td" )) {
        tg->tn = td;
    } else if ( !strcasecmp( tg->name, "i" )) {
        tg->tn = i_tag;
    } else if ( !strcasecmp( tg->name, "tr" )) {
        tg->tn = tr_tag;
    } else if ( !strcasecmp( tg->name, "th" )) {
        tg->tn = th;
    } else if ( !strcasecmp( tg->name, "tbody" )) {
        tg->tn = tbody;
    } else if ( !strcasecmp( tg->name, "table" )) {
        tg->tn = table;
    } else if ( !strcasecmp( tg->name, "abbr" )) {
        tg->tn = abbr;
    } else if ( !strcasecmp( tg->name, "b" )) {
        tg->tn = b_tag;
    } else if ( !strcasecmp( tg->name, "noscript" )) {
        tg->tn = noscript;
    } else if ( !strcasecmp( tg->name, "h2" )) {
        tg->tn = h2;
    } else if ( !strcasecmp( tg->name, "label" )) {
        tg->tn = label;
    } else if ( !strcasecmp( tg->name, "sup" )) {
        tg->tn = sup;
    } else if ( !strcasecmp( tg->name, "cite" )) {
        tg->tn = cite;
    } else if ( !strcasecmp( tg->name, "ol" )) {
        tg->tn = ol;
    } else if ( !strcasecmp( tg->name, "style" )) {
        tg->tn = style;
    } else if ( !strcasecmp( tg->name, "h1" )) {
        tg->tn = h1;
    } else if ( !strcasecmp( tg->name, "small" )) {
        tg->tn = small;
    } else if ( !strcasecmp( tg->name, "code" )) {
        tg->tn = code;
    } else if ( !strcasecmp( tg->name, "title" )) {
        tg->tn = title;
    } else if ( !strcasecmp( tg->name, "p" )) {
        tg->tn = p_tag;
    } else if ( !strcasecmp( tg->name, "source" )) {
        tg->tn = source;
    } else if ( !strcasecmp( tg->name, "track" )) {
        tg->tn = track;
    } else if ( !strcasecmp( tg->name, "audio" )) {
        tg->tn = audio;
    } else if ( !strcasecmp( tg->name, "hr" )) {
        tg->tn = hr;
    } else if ( !strcasecmp( tg->name, "blockquote" )) {
        tg->tn = blockquote;
    } else if ( !strcasecmp( tg->name, "br" )) {
        tg->tn = br;
    } else if ( !strcasecmp( tg->name, "pre" )) {
        tg->tn = pre;
    } else {
        printf("Unknown tag: %s\n", tg->name);
        tg->tn = unknown;
    }

    return 1;
}

enum tag_name get_tt( char *element )
{
    if ( !element ) {
        return unknown;
    }

    if ( !strcasecmp( element, "html" ) ) {
        return html;
    } else if ( !strcasecmp( element, "body" )) {
        return body;
    } else if ( !strcasecmp( element, "head" )) {
        return head_tag;
    } else if ( !strcasecmp( element, "meta" )) {
        return meta;
    } else if ( !strcasecmp( element, "li" )) {
        return li;
    } else if ( !strcasecmp( element, "script" )) {
        return script;
    } else if ( !strcasecmp( element, "link" )) {
        return link_tag;
    } else if ( !strcasecmp( element, "a" )) {
        return a_tag;
    } else if ( !strcasecmp( element, "input" )) {
        return input;
    } else if ( !strcasecmp( element, "div" )) {
        return div_tag;
    } else if ( !strcasecmp( element, "form" )) {
        return form;
    } else if ( !strcasecmp( element, "h3" )) {
        return h3;
    } else if ( !strcasecmp( element, "ul" )) {
        return ul;
    } else if ( !strcasecmp( element, "span" )) {
        return span;
    } else if ( !strcasecmp( element, "img" )) {
        return img;
    } else if ( !strcasecmp( element, "td" )) {
        return td;
    } else if ( !strcasecmp( element, "i" )) {
        return i_tag;
    } else if ( !strcasecmp( element, "tr" )) {
        return tr_tag;
    } else if ( !strcasecmp( element, "th" )) {
        return th;
    } else if ( !strcasecmp( element, "tbody" )) {
        return tbody;
    } else if ( !strcasecmp( element, "table" )) {
        return table;
    } else if ( !strcasecmp( element, "abbr" )) {
        return abbr;
    } else if ( !strcasecmp( element, "b" )) {
        return b_tag;
    } else if ( !strcasecmp( element, "noscript" )) {
        return noscript;
    } else if ( !strcasecmp( element, "h2" )) {
        return h2;
    } else if ( !strcasecmp( element, "label" )) {
        return label;
    } else if ( !strcasecmp( element, "sup" )) {
        return sup;
    } else if ( !strcasecmp( element, "cite" )) {
        return cite;
    } else if ( !strcasecmp( element, "ol" )) {
        return ol;
    } else if ( !strcasecmp( element, "style" )) {
        return style;
    } else if ( !strcasecmp( element, "h1" )) {
        return h1;
    } else if ( !strcasecmp( element, "small" )) {
        return small;
    } else if ( !strcasecmp( element, "code" )) {
        return code;
    } else if ( !strcasecmp( element, "title" )) {
        return title;
    } else if ( !strcasecmp( element, "p" )) {
        return p_tag;
    } else if ( !strcasecmp( element, "source" )) {
        return source;
    } else if ( !strcasecmp( element, "track" )) {
        return track;
    } else if ( !strcasecmp( element, "audio" )) {
        return audio;
    } else if ( !strcasecmp( element, "hr" )) {
        return hr;
    } else if ( !strcasecmp( element, "blockquote" )) {
        return blockquote;
    } else if ( !strcasecmp( element, "br" )) {
        return br;
    } else if ( !strcasecmp( element, "pre" )) {
        return pre;
    }

    return unknown;
}

int cut_all_tags( tag *tg, char *element)
{
    int result = 0;
    int i,j;

    if ( !tg || !element ) {
        return 0;
    }

    /// If this is the element we want to delete then remove it and all subcomponents
    if ( tg->tn == get_tt( element ) ) {
        free_tag( tg );

        return 0;
    }

    /// If this one is valid then just check the subcomponents
    for ( i = 0; i < tg->component_count; i++) {
        result = cut_all_tags( tg->subcomps[i], element);

        if ( !result ) {
            tg->subcomps[i] = NULL;
        }
    }

    i = 0;
    j = 0;

    while (i < tg->component_count) {
        if ( tg->subcomps[i] ) {
            tg->subcomps[j] = tg->subcomps[i];
            j++;
        } 

        i++;
    }

    i = j;

    while ( i < tg->component_count) {
        tg->subcomps[i] = NULL;

        i++;
    }

    tg->component_count = j;

    return 1;
}

int parse_tag( char *html_data, int length, tag *new_tag)
{
    int state = START;
    int index = 0;

    int result = 0;
    char *name_start = NULL;
    char *name_end = NULL;
    char *name = NULL;

    char *attrib_start = NULL;
    char *attrib_end = NULL;
    char *attrib = NULL;

    char *attrib_value_start = NULL;
    char *attrib_value_end = NULL;
    char *attrib_value = NULL;

    char *tag_text_start = NULL;
    char *tag_text_end = NULL;
    char *tag_text = NULL;

    char *eval_tag_start = NULL;
    char *eval_tag_end = NULL;
    char *eval_tag = NULL;

    char *comment_start = NULL;
    char *comment_end = NULL;
    char *comment = NULL;

    char *doctype_start = NULL;
    char *doctype_end = NULL;
    char *doctype = NULL;

    char *save_state = NULL;
    int save_index = 0;
    int prev_state;

    tag *temp_tag = NULL;

    char c;

    if ( !html_data || !new_tag) {
        return 0;
    }

    while ( state != END && index < length ) {
        switch (state) {
            case START:
                //printf("[STATE] Start\n");

                c = html_data[index];

                if ( isspace(c) ) {
                    index++;
                } else if ( c == '<' ) {
                    index++;
                    state = TAGNAME;
                } else {
                    state = ERROR;
                    break;
                }

                prev_state = START;
                break;
            case TAGNAME_EAT_WHITESPACE:
                //printf("[STATE] Eating whitespace before tagname\n");

                c = html_data[index];

                if ( isspace(c) ) {
                    index++;
                } else {
                    state = TAGNAME;
                }

                prev_state = TAGNAME_EAT_WHITESPACE;
                break;
            case TAGNAME:
                //printf("[STATE] Tagname\n");

                c = html_data[index];

                if ( isspace(c) ) {
                    if ( !name_start ) {
                        state = TAGNAME_EAT_WHITESPACE;
                        index++;
                    } else {
                        name_end = html_data + index;
                        state = TAGNAME_END;
                    }
                } else if ( isalnum( c )) {
                    if ( !name_start ) {
                        name_start = html_data + index;

                        /// It can't start with a digit
                        if ( isdigit(c) ) {
                            state = ERROR;
                        }
                    }

                    index++;
                } else if (  c == '/' || c == '>' ) {
                    name_end = html_data + index;
                    state = TAGNAME_END;

                } else if (c == '!' && prev_state == START && doctype == NULL) {
                    index++;

                    state = DOCTYPE;

                    doctype_start = html_data + index;
                } else {
                    state = ERROR;
                }

                prev_state = TAGNAME;
                break;
            case TAGNAME_END:
                //printf("[STATE] Tagname end\n");

                /// If any of the pointers are NULL or there wa
                if ( !name_end || !name_start || name_end == name_start) {
                    state = ERROR;
                    break;
                }

                name = calloc( 1, (name_end - name_start) + 1 );

                if ( !name ) {
                    //printf("[ERROR] calloc tagname fail\n");
                    exit(0);
                }

                memcpy( name, name_start, name_end - name_start);

                //printf("[INFO] Tagname: %s\n", name);

                new_tag->name = name;

                state = AFTER_TAGNAME;

                prev_state = TAGNAME_END;
                break;
            case AFTER_TAGNAME:
                //printf("[STATE] After Tagname\n");

                c = html_data[index];

                if ( isspace(c) ) {
                    index++;
                } else if ( c == '/' ) {
                    state = TERMINATE_SLASH;
                    index++;
                } else if ( c == '>') {
                    state = INSIDE_TAG;
                    index++;
                } else if (isalpha(c) ) {
                    state = ATTRIBUTE;
                } else {
                    state = ERROR;
                }

                prev_state = AFTER_TAGNAME;
                break;
            case TERMINATE_SLASH:
                //printf("[STATE] Terminate slash\n");

                c = html_data[index];

                if ( c == '>') {
                    state = TERMINATE_TAG;
                    index++;
                } else if ( isspace(c) ) {
                    index++;
                } else {
                    state = ERROR;
                }

                prev_state = TERMINATE_SLASH;
                break;
            case TERMINATE_TAG:
                //printf("[STATE] Terminate tag success\n");

                goto end;
                break;
            case INSIDE_TAG:
                //printf("[STATE] Inside tag\n");

                /// Check for meta tag
                if ( !needs_closer(name ) ) {
                    state = TERMINATE_TAG;
                    break;
                }

                c = html_data[index];

                if ( isspace(c) ) {
                    index++;
                } else if ( c == '<' ) {
                    save_state = html_data + index;
                    save_index = index;

                    state = EVAL_TAG;
                    index++;
                } else {
                    state = INSIDE_TAG_TEXT;
                    tag_text_start = html_data + index;
                    index++;
                }
                
                prev_state = INSIDE_TAG;
                break;
            case INSIDE_TAG_TEXT:
                //printf("[STATE] Inside tag text\n");

                c = html_data[index];

                if ( c == '<') {
                    save_state = html_data + index;
                    save_index = index;


                    tag_text_end = html_data + index;

                    tag_text = calloc( 1, (tag_text_end-tag_text_start) + 1);

                    if ( !tag_text ) {
                        //printf("[ERROR] failed to calloc tag text\n");
                        goto fail;
                    }

                    memcpy( tag_text, tag_text_start, tag_text_end - tag_text_start);

                    //printf("[INFO] Tag text: %s\n", tag_text);

                    if ( add_component_tag( new_tag, create_text_tag( tag_text) ) ) {
                        free(tag_text);
                        tag_text = NULL;
                        //printf("[ERROR] Failed to add component tag\n");
                        goto fail;
                    }

                    free(tag_text);
                    tag_text = NULL;

                    state = EVAL_TAG;

                    prev_state = INSIDE_TAG_TEXT;
                    index++;
                } else {
                    index++;
                }
                break;
            case EVAL_TAG:
                //printf("[STATE] Eval tag\n");

                c = html_data[index];

                if ( c == '/' ) {
                    state = CLOSE_CURRENT_TAG;
                    index++;
                } else if ( c == '!') {
                    //// Because EVAL_TAG is reached via INSIDE_TAG I want to jump back to INSIDE_TAG
                    prev_state = INSIDE_TAG;

                    state = OPEN_COMMENT_DASH_ONE;

                    index++;
                } else if ( isspace(c) ) {
                    index++;
                } else if ( isalpha(c) ) {
                    /// This means that a new tag is being opened

                    temp_tag = calloc( 1, sizeof(tag) );

                    if ( !temp_tag ) {
                        //printf("[ERROR] calloc fail\n");
                        goto fail;
                    }
                    result = parse_tag( save_state, length - save_index, temp_tag );

                    if ( !result ) {
                        //printf("[INFO] Exiting after subtag fail\n");
                        goto fail;
                    }

                    if ( add_component_tag( new_tag, temp_tag) ) {
                        //printf("[ERROR] Failed ot add component tag\n");
                        goto fail;
                    }

                    // If the subtag parse was successful then add the returned index
                    index = save_index + result;

                    state = INSIDE_TAG;

                }  else {
                    state = ERROR;
                }

                prev_state = EVAL_TAG;
                break;
            case OPEN_COMMENT_DASH_ONE:
                //printf("[STATE] Open comment first dash\n");

                c = html_data[index];

                if ( c == '-') {
                    state = OPEN_COMMENT_DASH_TWO;
                    index++;
                } else {
                    state = ERROR;
                }

                prev_state = OPEN_COMMENT_DASH_ONE;
                break;
            case OPEN_COMMENT_DASH_TWO:
                //printf("[STATE] Open comment second dash\n");

                c = html_data[index];

                if ( c == '-') {
                    index++;

                    comment_start = html_data + index;

                    state = CLOSE_COMMENT;
                } else { 
                    state = ERROR;
                }

                prev_state = OPEN_COMMENT_DASH_TWO;
                break;
            case CLOSE_COMMENT:
                //printf("[STATE] Close comment\n");

                c = html_data[index];

                if ( c == '-') {
                    index++;

                    state = CC_DASH_TWO;
                } else {
                    index++;
                }

                prev_state = CLOSE_COMMENT;
                break;
            case CC_DASH_TWO:
                //printf("[STATE] CC Dash Two\n");
                /// At this point the only guarantee is "-"

                c = html_data[index];

                if ( c == '-') {
                    index++;

                    state = END_COMMENT;
                } else {
                    index++;

                    state = CLOSE_COMMENT;
                }

                prev_state = CC_DASH_TWO;
                break;
            case END_COMMENT:
                //printf("[STATE] End comment\n");

                c = html_data[index];

                if ( c != '>') {
                    state = CC_DASH_TWO;

                    comment_end = html_data + (index-2);
                } else {
                    comment_end = html_data + (index-2);

                    if ( !comment ) {
                        comment = calloc( 1, ( comment_end - comment_start) + 1 );
                    }

                    memcpy( comment, comment_start, (comment_end-comment_start));

                    //printf("[INFO] Comment: %s\n", comment);

                    //free(comment);

                    state = INSIDE_TAG;

                    index++;
                }

                prev_state = END_COMMENT;
                break;
            case CLOSE_CURRENT_TAG:
                //printf("[STATE] Close current tag\n");

                c = html_data[index];

                if ( isspace(c) ) {
                    if ( eval_tag_start ) {
                        eval_tag_end = html_data + index;

                        eval_tag = calloc( 1, ( eval_tag_end - eval_tag_start) + 1);

                        if ( !eval_tag ) {
                            //printf("[ERROR] Failed to allocate eval tag\n");
                            goto fail;
                        }

                        memcpy( eval_tag, eval_tag_start, eval_tag_end - eval_tag_start);

                        if ( strcasecmp( eval_tag, name ) ) {
                            //printf("[ERROR] Invalid close tag. Expected: %s received: %s\n", name, eval_tag);
                            free(eval_tag);
                            goto fail;
                        }

                        state = HOMESTRETCH;
                    }
                    index++;
                } else if ( isalnum(c) ) {
                    if ( !eval_tag_start) {
                        eval_tag_start = html_data + index;

                        if ( isdigit( c ) ) {
                            state = ERROR;
                        }
                    }

                    index++;
                } else if ( c == '>' ) {
                    eval_tag_end = html_data + index;

                    eval_tag = calloc( 1, ( eval_tag_end - eval_tag_start) + 1);

                    if ( !eval_tag ) {
                        //printf("[ERROR] Failed to allocate eval tag\n");
                        goto fail;
                    }

                    memcpy( eval_tag, eval_tag_start, eval_tag_end - eval_tag_start);

                    if ( strcasecmp( eval_tag, name ) ) {
                        //printf("[ERROR] Invalid close tag. Expected: %s received: %s\n", name, eval_tag);
                        free(eval_tag);
                        goto fail;
                    }

                    free(eval_tag);

                    state = HOMESTRETCH;
                } else {
                    state = ERROR;
                }

                prev_state = CLOSE_CURRENT_TAG;
                break;
            case HOMESTRETCH:
                //printf("[STATE] Homestretch\n");

                c = html_data[index];

                if ( isspace(c) ) {
                    index++;
                } else if ( c == '>' ) {
                    index++;
                    goto end;
                } else {
                    state = ERROR;
                }

                prev_state = HOMESTRETCH;
                break;
            case ATTRIBUTE:
                //printf("[STATE] Attribute\n");

                c = html_data[index];

                if ( isalpha(c) || c == '-') {
                    if ( attrib_start == NULL ) {
                        attrib_start = html_data + index;

                        /// The attribute shouldn't start with a '-'
                        if ( c == '-') {
                            state = ERROR;
                        }
                    }

                    index++;
                } else if ( isspace(c) ) {
                    attrib_end = html_data + index;

                    state = ATTRIBUTE_WHITESPACE;
                    index++;
                } else if ( c == '=') {
                    state = ATTRIBUTE_EQUALS;

                    attrib_end = html_data + index;
                    index++;
                } else if ( c == '>') {
                    attrib_end = html_data + index;

                    state = ATTRIBUTE_END_NULL_VALUE;
                } else {
                    state = ERROR;
                }

                prev_state = ATTRIBUTE;
                break;
            case ATTRIBUTE_EQUALS:
                //printf("[STATE] Attribute equals\n");

                if ( !attrib_start || !attrib_end || attrib_end == attrib_start) {
                    state = ERROR;
                    break;
                }

                attrib = calloc( 1, (attrib_end - attrib_start) + 1);

                if ( !attrib ) {
                    //printf("[ERROR] Failed to calloc attrib\n");
                    goto fail;
                }

                memcpy( attrib, attrib_start, attrib_end - attrib_start);

                //printf("[INFO] Attribute: %s\n", attrib);                

                state = ATTR_VALUE_START;

                prev_state = ATTRIBUTE_EQUALS;
                break;
            case ATTR_VALUE_START:
                //printf("[STATE] Attribute value start\n");

                c = html_data[index];

                if ( isspace(c) ) {
                    index++;
                } else if ( c == '"') {
                    state = ATTR_VALUE_INSIDE;
                    index++;

                    attrib_value_start = html_data + index;
                } else if ( c == '\'') {
                    state = ATTR_VALUE_INSIDE_TIC;
                    index++;

                    attrib_value_start = html_data + index;
                }else {
                    state = ERROR;
                }

                prev_state = ATTR_VALUE_START;
                break;
            case ATTR_VALUE_INSIDE_TIC:
                //printf("[STATE] Attribute value inside\n");

                c = html_data[index];

                if ( c == '\'') {
                    attrib_value_end = html_data + index;
                    index++;

                    state = ATTR_VALUE_END;
                } else {
                    index++;
                }

                prev_state = ATTR_VALUE_INSIDE_TIC;
                break;
            case ATTR_VALUE_INSIDE:
                //printf("[STATE] Attribute value inside\n");

                c = html_data[index];

                if ( c == '"') {
                    attrib_value_end = html_data + index;
                    index++;

                    state = ATTR_VALUE_END;
                } else {
                    index++;
                }

                prev_state = ATTR_VALUE_INSIDE;
                break;
            case ATTR_VALUE_END:
                //printf("[STATE] Attribute value end\n");

                if ( !attrib_value_start || !attrib_value_end ) {
                    state = ERROR;
                    break;
                }

                attrib_value = calloc( 1, (attrib_value_end - attrib_value_start) + 1 );

                if ( !attrib_value) {
                    //printf("[ERROR} Failed to calloc attrib value\n");
                    goto fail;
                }

                memcpy( attrib_value, attrib_value_start, attrib_value_end - attrib_value_start);

                //printf("[INFO] Attribute value: %s\n", attrib_value);

                add_attribute( new_tag, attrib, attrib_value);

                attrib_value_start = NULL;
                attrib_value_end = NULL;
                attrib_value = NULL;

                attrib_end = NULL;
                attrib_start = NULL;
                attrib = NULL;

                state = AFTER_TAGNAME;

                prev_state = ATTR_VALUE_END;
                break;
            case ATTRIBUTE_WHITESPACE:
                //printf("[STATE] Attribute whitespace\n");

                c = html_data[index];

                if ( c == '=') {
                    state = ATTRIBUTE_EQUALS;
                    index++;
                } else if ( isalpha(c) ) {
                    state = ATTRIBUTE_END_NULL_VALUE;
                } else if ( c == '>') {
                    state = ATTRIBUTE_END_NULL_VALUE;
                } else if ( isspace(c) ) {
                    index++;
                } else {
                    state = ERROR;
                }

                prev_state = ATTRIBUTE_WHITESPACE;
                break;
            case ATTRIBUTE_END_NULL_VALUE:
                //printf("[STATE] ATTRIBUTE_END_NULL_VALUE\n");

                add_attribute( new_tag, attrib, NULL);

                attrib_value_start = NULL;
                attrib_value_end = NULL;
                attrib_value = NULL;

                attrib_end = NULL;
                attrib_start = NULL;
                attrib = NULL;

                state = AFTER_TAGNAME;

                prev_state = ATTRIBUTE_END_NULL_VALUE;
                break;
            case DOCTYPE:
                //printf("[STATE] Doctype\n");

                c = html_data[index];

                if ( isspace(c) ) {
                    doctype_end = html_data + index;

                    doctype = calloc( 1, (doctype_end-doctype_start) + 1);

                    if ( !doctype ) {
                        //printf("[ERROR] Failed to calloc doctype\n");
                        goto fail;
                    }

                    memcpy( doctype, doctype_start, doctype_end-doctype_start);

                    //printf("[INFO] Doctype: %s\n", doctype);

                    if ( strcasecmp( doctype, "DOCTYPE") ) {
                        //printf("[ERROR] Invalid DOCTYPE tag\n");
                        goto fail;
                    }

                    free(doctype);

                    doctype_start = html_data + index;
                    doctype_end = NULL;
                    doctype = NULL;

                    state = DOCTYPE_SUBS;
                } else if ( isalpha(c) ) {
                    index++;
                } else {
                    state = ERROR;
                }

                prev_state = DOCTYPE;
                break;
            case DOCTYPE_SUBS:
                //printf("[STATE] Doctype subs\n");

                c = html_data[index];

                if ( c == '>' ) {
                    doctype_end = html_data + index;

                    doctype = calloc( 1, (doctype_end-doctype_start) + 1);

                    if ( !doctype ) {
                        //printf("[ERROR] Failed to calloc doctype subs\n");
                        goto fail;
                    }

                    memcpy( doctype, doctype_start, doctype_end-doctype_start);

                    //printf("[INFO] Doctype subs: %s\n", doctype);

                    index++;

                    free(doctype);

                    state = START;
                } else if ( isascii(c) ) {
                    index++;
                } else {
                    state = ERROR;
                }

                prev_state = DOCTYPE_SUBS;
                break;
            case ERROR:
                printf("[STATE] Error: %s\n", html_data + index);
                goto fail;
            default:
                printf("[ERROR] Unknown state\n");
                goto fail;
        };
    }

fail:
    free_tag( new_tag );
    return 0;

end:
    set_tag_type(new_tag);
    return index;
}

int rd( char *fn, char **data, int *len )
{
    struct stat st;
    char *outdata = NULL;
    int fd;

    if ( stat( fn, &st) ) {
        printf("stat failed\n");
        exit(0);
    }

    outdata = calloc( 1, st.st_size + 1);

    if ( !outdata) {
        printf("calloc fail\n");
        exit(0);
    }

    fd = open( fn, O_RDONLY);

    if ( fd < 0 ) {
        free(outdata);
        printf("open failed\n");
        exit(0);
    }

    read( fd, outdata, st.st_size);

    close(fd);

    *data = outdata;
    *len = st.st_size;

    printf("Opened: %s\n", fn);

    return *len;
}

int main( int argc, char**argv )
{
    char *html_data = NULL;
    int length = 0;
    stream *s;
    tag *t;

    if ( argc != 3) {
        printf("%s <inputfile.html> <element-to-cut>\n", argv[0]);
        exit(1);
    }

    rd( argv[1], &html_data, &length);

    memset( &t, 0, sizeof(t));

    t = calloc(1, sizeof(tag));

    if ( !t ) {
        return 1;
    }

    if ( ! parse_tag( html_data, length, t) ) {
        free(html_data);
        printf("[ERROR] Failed to parse html\n");
        return 1;
    }

    free(html_data);

    cut_all_tags( t, argv[2]);

    s = init_stream_nd( 1024);

    print_tag( s, t, 0);

    FILE *fp;

    fp = fopen("out.html", "w");

    if ( fp == NULL ) {
        printf("Open of out.html failed: %s\n", strerror( errno));
	return 1;
    }

    fwrite( s->data, s->index, 1, fp);

    fclose(fp);
    free_tag( t );

    return 0;
}
