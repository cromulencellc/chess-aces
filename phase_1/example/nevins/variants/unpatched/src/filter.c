#include "filter.h"
int is_filter(char *uri) {
  int result = 0;
  if (!uri) {
    goto end;
  }
  result = !strcasecmp(uri, "/filter");
end:
  return result;
}
void free_filter(filter *f) {
  if (!f) {
    goto end;
  }
  if (f->host) {
    free(f->host);
  }
  if (f->action) {
    free(f->action);
  }
  if (f->ft) {
    free(f->ft);
  }
  if (f->element) {
    free(f->element);
  }
  if (f->from) {
    free(f->from);
  }
  if (f->to) {
    free(f->to);
  }
  if (f->cnt) {
    free(f->cnt);
  }
  if (f->mime_type) {
    free(f->mime_type);
  }
  if (f->mime_subtype) {
    free(f->mime_subtype);
  }
  if (f->mime_parameter) {
    free(f->mime_parameter);
  }
  if (f->mime_value) {
    free(f->mime_value);
  }
  free(f);
end:
  return;
}
filter *parse_filter(char *filter_line) {
  char *temp = NULL;
  filter *new_filter = NULL;
  stream *s = NULL;
  char c;
  if (!filter_line) {
    goto fail;
  }
  new_filter = calloc(1, sizeof(filter));
  if (!new_filter) {
    goto fail;
  }
  s = initstream(filter_line, strlen(filter_line));
  if (!s) {
    goto fail;
  }
  if (readbyte(s, &c)) {
    goto fail;
  }
  if (c == 'P' || c == 'p') {
    new_filter->location = 0;
  } else if (c == 'S' || c == 's') {
    new_filter->location = 1;
  } else if (c == 'B' || c == 'b') {
    new_filter->location = 2;
  } else {
    goto fail;
  }
  if (readbyte(s, &c)) {
    goto fail;
  }
  if (c != ' ') {
    goto fail;
  }
  temp = readuntil(s, ' ');
  if (!temp) {
    goto fail;
  }
  if (!strcasecmp(temp, "tr")) {
    new_filter->c = tr;
  } else if (!strcasecmp(temp, "del")) {
    new_filter->c = del;
  } else if (!strcasecmp(temp, "blk")) {
    new_filter->c = blk;
  } else if (!strcasecmp(temp, "cut")) {
    new_filter->c = cut;
  } else if (!strcasecmp(temp, "hh")) {
    new_filter->c = hh;
  } else if (!strcasecmp(temp, "tgt")) {
    new_filter->c = tgt;
  } else if (!strcasecmp(temp, "src")) {
    new_filter->c = src;
  } else if (!strcasecmp(temp, "bft")) {
    new_filter->c = bft;
  } else if (!strcasecmp(temp, "drp")) {
    new_filter->c = drp;
  } else if (!strcasecmp(temp, "mm_s")) {
    new_filter->c = mm_s;
  } else if (!strcasecmp(temp, "mm_t")) {
    new_filter->c = mm_t;
  } else if (!strcasecmp(temp, "mm_p")) {
    new_filter->c = mm_p;
  } else {
    goto fail;
  }
  incstream(s);
  free(temp);
  temp = NULL;
  switch (new_filter->c) {
  case tr:
    new_filter->from = readuntil(s, ' ');
    if (!new_filter->from) {
      goto fail;
    }
    incstream(s);
    new_filter->to = eatitall(s);
    if (!new_filter->to) {
      goto fail;
    }
    break;
  case del:
    new_filter->from = readuntil(s, ' ');
    if (!new_filter->from) {
      new_filter->from = eatitall(s);
      if (!new_filter->from) {
        goto fail;
      }
    } else {
      incstream(s);
      new_filter->cnt = eatitall(s);
      if (!new_filter->cnt) {
        goto fail;
      }
      for (int i = 0; i < strlen(new_filter->cnt); i++) {
        if (!isdigit(new_filter->cnt[i])) {
          goto fail;
        }
      }
    }
    break;
  case blk:
    new_filter->from = eatitall(s);
    if (!new_filter->from) {
      goto fail;
    }
    break;
  case cut:
    new_filter->element = eatitall(s);
    if (!new_filter->element) {
      goto fail;
    }
    break;
  case hh:
    new_filter->from = eatitall(s);
    if (!new_filter->from) {
      goto fail;
    }
    break;
  case tgt:
    new_filter->host = eatitall(s);
    if (!new_filter->host) {
      goto fail;
    }
    break;
  case src:
    new_filter->host = eatitall(s);
    if (!new_filter->host) {
      goto fail;
    }
    break;
  case bft:
    new_filter->ft = eatitall(s);
    if (!new_filter->ft) {
      goto fail;
    }
    break;
  case drp:
    new_filter->cnt = eatitall(s);
    if (!new_filter->cnt) {
      goto fail;
    }
    for (int i = 0; i < strlen(new_filter->cnt); i++) {
      if (!isdigit(new_filter->cnt[i])) {
        goto fail;
      }
    }
    break;
  case mm_s:
    new_filter->mime_subtype = eatitall(s);
    if (!new_filter->mime_subtype) {
      goto fail;
    }
    break;
  case mm_t:
    new_filter->mime_type = eatitall(s);
    if (!new_filter->mime_type) {
      goto fail;
    }
    break;
  case mm_p:
    new_filter->mime_parameter = eatitall(s);
    if (!new_filter->mime_parameter) {
      goto fail;
    }
    break;
  default:
    goto fail;
    break;
  };
  free_stream(s);
  return new_filter;
fail:
  if (temp) {
    free(temp);
  }
  free_stream(s);
  free_filter(new_filter);
  new_filter = NULL;
  return new_filter;
}
char *list_filters(filter *root) {
  stream *s = init_stream_nd(512);
  char * final = NULL;
  filter *walker = NULL;
  if (!root) {
    goto end;
  }
  if (!s) {
    goto end;
  }
  walker = root;
  while (walker) {
    add_int(s, walker->id);
    add_string(s, " ");
    if (walker->location == 0) {
      add_string(s, "P ");
    } else if (walker->location == 1) {
      add_string(s, "S ");
    } else {
      add_string(s, "B ");
    }
    switch (walker->c) {
    case tr:
      add_string(s, "tr ");
      add_string(s, walker->from);
      add_string(s, " ");
      add_string(s, walker->to);
      add_string(s, "\n");
      break;
    case del:
      add_string(s, "del ");
      add_string(s, walker->from);
      if (walker->cnt) {
        add_string(s, " ");
        add_string(s, walker->cnt);
      }
      add_string(s, "\n");
      break;
    case blk:
      add_string(s, "blk ");
      add_string(s, walker->from);
      add_string(s, "\n");
      break;
    case cut:
      add_string(s, "cut ");
      add_string(s, walker->element);
      add_string(s, "\n");
      break;
    case hh:
      add_string(s, "hh ");
      add_string(s, walker->from);
      add_string(s, "\n");
      break;
    case tgt:
      add_string(s, "tgt ");
      add_string(s, walker->host);
      add_string(s, "\n");
      break;
    case src:
      add_string(s, "src ");
      add_string(s, walker->host);
      add_string(s, "\n");
      break;
    case bft:
      add_string(s, "bft ");
      add_string(s, walker->ft);
      add_string(s, "\n");
      break;
    case mm_t:
      add_string(s, "mm_t ");
      add_string(s, walker->mime_type);
      add_string(s, "\n");
      break;
    case mm_s:
      add_string(s, "mm_s ");
      add_string(s, walker->mime_subtype);
      add_string(s, "\n");
      break;
    case mm_p:
      add_string(s, "mm_p ");
      add_string(s, walker->mime_parameter);
      add_string(s, "\n");
      break;
    default:
      goto end;
      break;
    };
    walker = walker->next;
  }
  final = s->data;
  free(s);
  s = NULL;
end:
  if (s) {
    free_stream(s);
  }
  return final;
}
void renumber_filters(filter *root) {
  filter *walker = NULL;
  int count = 0;
  if (!root) {
    return;
  }
  walker = root;
  while (walker) {
    walker->id = count++;
    walker = walker->next;
  }
  return;
}
void free_mimetype(mimetype *parsed_mime) {
  if (!parsed_mime) {
    return;
  }
  if (parsed_mime->type) {
    free(parsed_mime->type);
  }
  if (parsed_mime->subtype) {
    free(parsed_mime->subtype);
  }
  if (parsed_mime->parameter) {
    free(parsed_mime->parameter);
  }
  free(parsed_mime);
  return;
}
int add_filter(filter **root, filter *f) {
  filter *walker = NULL;
  filter *t = NULL;
  int result = -1;
  int id = 0;
  if (!f) {
    goto end;
  }
  if (!root) {
    goto end;
  }
  if (f->c == drp) {
    id = atoi(f->cnt);
    if (*root) {
      if ((*root)->id == id) {
        walker = *root;
        *root = walker->next;
        free_filter(walker);
        free_filter(f);
        renumber_filters(*root);
        result = 0;
        goto end;
      }
      walker = *root;
      while (walker->next) {
        if (walker->next->id == id) {
          t = walker->next;
          walker->next = t->next;
          free_filter(t);
          free_filter(f);
          renumber_filters(*root);
          result = 0;
          goto end;
        }
      }
    }
    free_filter(f);
    goto end;
  }
  f->next = *root;
  *root = f;
  renumber_filters(*root);
  result = 0;
end:
  return result;
}
mimetype *parse_mimetype(char *mm) {
  mimetype *parsed_mime = NULL;
  char *end = NULL;
  if (!mm) {
    goto end;
  }
  parsed_mime = calloc(1, sizeof(mimetype));
  if (!parsed_mime) {
    goto end;
  }
  end = strchr(mm, '/');
  if (!end) {
    parsed_mime->type = strdup(mm);
    goto end;
  }
  parsed_mime->type = calloc(1, (end - mm) + 1);
  if (!parsed_mime->type) {
    free(parsed_mime);
    parsed_mime = NULL;
    goto end;
  }
  strncpy(parsed_mime->type, mm, end - mm);
  mm = end + 1;
  end = strchr(mm, ';');
  if (!end) {
    parsed_mime->subtype = strdup(mm);
    goto end;
  }
  parsed_mime->subtype = calloc(1, (end - mm) + 1);
  if (!parsed_mime->subtype) {
    free(parsed_mime->type);
    free(parsed_mime);
    parsed_mime = NULL;
    goto end;
  }
  strncpy(parsed_mime->subtype, mm, end - mm);
  mm = end + 1;
  end = strchr(mm, '=');
  if (!end) {
    free(parsed_mime->type);
    free(parsed_mime->subtype);
    free(parsed_mime);
    parsed_mime = NULL;
    goto end;
  }
  parsed_mime->parameter = calloc(1, (end - mm) + 1);
  if (!parsed_mime->parameter) {
    free(parsed_mime->subtype);
    free(parsed_mime->type);
    free(parsed_mime);
    parsed_mime = NULL;
    goto end;
  }
  strncpy(parsed_mime->parameter, mm, end - mm);
  mm = end + 1;
  parsed_mime->value = strdup(mm);
  if (!parsed_mime->value) {
    free(parsed_mime->subtype);
    free(parsed_mime->parameter);
    free(parsed_mime->type);
    free(parsed_mime);
    parsed_mime = NULL;
  }
end:
  return parsed_mime;
}
int filter_applies(field *fld, char *content, filter *f, char *server,
                   char *uri, int is_request) {
  int result = 0;
  char *t = NULL;
  field *walker = NULL;
  if ((!fld && !content && !server && !uri) || !f) {
    goto end;
  }
  if (!(f->location ^ is_request)) {
    goto end;
  }
  switch (f->c) {
  case tr:
    walker = fld;
    while (walker) {
      if (strstr(walker->field_name, f->from)) {
        result = 1;
        goto end;
      }
      if (strstr(walker->field_data, f->from)) {
        result = 1;
        goto end;
      }
      walker = walker->next;
    }
    if (content) {
      if (strstr(content, f->from)) {
        result = 1;
        goto end;
      }
    }
    break;
  case del:
    walker = fld;
    while (walker) {
      if (strstr(walker->field_name, f->from)) {
        result = 1;
        goto end;
      }
      if (strstr(walker->field_data, f->from)) {
        result = 1;
        goto end;
      }
      walker = walker->next;
    }
    if (content) {
      if (strstr(content, f->from)) {
        result = 1;
        goto end;
      }
    }
    break;
  case blk:
    walker = fld;
    while (walker) {
      if (strstr(walker->field_name, f->from)) {
        result = 1;
        goto end;
      }
      if (strstr(walker->field_data, f->from)) {
        result = 1;
        goto end;
      }
      walker = walker->next;
    }
    if (content) {
      if (strstr(content, f->from)) {
        result = 1;
        goto end;
      }
    }
    break;
  case cut:
    t = get_field_data(&fld, "Content-Type");
    if (t) {
      result = 1;
    }
    break;
  case hh:
    t = get_field_data(&fld, f->from);
    if (t) {
      result = 1;
    }
    break;
  case tgt:
    if (server) {
      if (!strcmp(server, f->host)) {
        result = 1;
        goto end;
      }
    }
    break;
  case src:
    t = get_field_data(&fld, "Host");
    result = 1;
    break;
  case bft:
    if (uri) {
      t = strrchr(uri, '.');
      if (t) {
        if (!strcasecmp(t + 1, f->ft)) {
          result = 1;
          goto end;
        }
      }
    }
    break;
  case mm_t:
    t = get_field_data(&fld, "Content-Type");
    if (!t) {
      goto end;
    }
    result = 1;
    break;
  case mm_s:
    t = get_field_data(&fld, "Content-Type");
    result = 1;
    break;
  case mm_p:
    t = get_field_data(&fld, "Content-Type");
    if (!t) {
      goto end;
    }
    result = 1;
    break;
  default:
    break;
  };
end:
  return result;
}
char *strrep(char *haystack, char *needle, char *rep, int max) {
  char * final = NULL;
  int count = 0;
  int needle_length = 0;
  int rep_length = 0;
  char *f = NULL;
  char *e = NULL;
  int final_length = 0;
  char *haystack_walker = NULL;
  if (!haystack || !needle || !rep) {
    goto end;
  }
  needle_length = strlen(needle);
  rep_length = strlen(rep);
  f = strstr(haystack, needle);
  while (f) {
    count++;
    f = strstr(f + needle_length, needle);
  }
  if (!count) {
    goto end;
  }
  final_length = strlen(haystack) + (rep_length * count);
  final = calloc(1, final_length + 1);
  if (!final) {
    goto end;
  }
  e = final;
  haystack_walker = haystack;
  if (max > 0 && count > max) {
    count = max;
  }
  while (count--) {
    f = strstr(haystack_walker, needle);
    strncpy(e, haystack_walker, f - haystack_walker);
    e += f - haystack_walker;
    strncpy(e, rep, rep_length);
    e += rep_length;
    f += needle_length;
    haystack_walker = f;
  }
  strcpy(e, haystack_walker);
end:
  return final;
}
int apply_tr(HTTPRequest *req, filter *tr_filter) {
  int result = -1;
  field *walker = NULL;
  char *temp = NULL;
  if (!req || !tr_filter) {
    goto end;
  }
  walker = req->field_roots;
  while (walker) {
    temp = strrep(walker->field_name, tr_filter->from, tr_filter->to, 0);
    if (temp) {
      free(walker->field_name);
      walker->field_name = temp;
      temp = NULL;
    }
    temp = strrep(walker->field_data, tr_filter->from, tr_filter->to, 0);
    if (temp) {
      free(walker->field_data);
      walker->field_data = temp;
      temp = NULL;
    }
    walker = walker->next;
  }
  if (req->content_length > 0 && req->content) {
    temp = strrep(req->content, tr_filter->from, tr_filter->to, 0);
    if (temp) {
      free(req->content);
      req->content = temp;
      req->content_length = strlen(req->content);
      temp = NULL;
    }
  }
  result = 0;
end:
  return result;
}
int apply_del(HTTPRequest *req, filter *del_filter) {
  int result = -1;
  field *walker = NULL;
  char *temp;
  int count = 0;
  if (!req || !del_filter) {
    goto end;
  }
  walker = req->field_roots;
  if (del_filter->cnt) {
    count = atoi(del_filter->cnt);
  }
  while (walker) {
    temp = strrep(walker->field_name, del_filter->from, "", count);
    if (temp) {
      free(walker->field_name);
      walker->field_name = temp;
      temp = NULL;
    }
    temp = strrep(walker->field_data, del_filter->from, "", count);
    if (temp) {
      free(walker->field_data);
      walker->field_data = temp;
      temp = NULL;
    }
    walker = walker->next;
  }
  if (req->content_length > 0 && req->content) {
    temp = strrep(req->content, del_filter->from, "", count);
    if (temp) {
      free(req->content);
      req->content = temp;
      req->content_length = strlen(req->content);
      temp = NULL;
    }
  }
end:
  return result;
}
int apply_blk(HTTPRequest *req, filter *blk_filter) {
  int result = -1;
  field *walker = NULL;
  if (!req || !blk_filter) {
    goto end;
  }
  walker = req->field_roots;
  while (walker) {
    if (strstr(walker->field_name, blk_filter->from)) {
      goto blockit;
    }
    if (strstr(walker->field_data, blk_filter->from)) {
      goto blockit;
    }
    walker = walker->next;
  }
  if (req->content) {
    if (strstr(req->content, blk_filter->from)) {
      goto blockit;
    }
  }
blockit:
  req->blocked = 1;
  result = 0;
end:
  return result;
}
int apply_hh(HTTPRequest *req, filter *hh_filter) {
  int result = -1;
  field *walker = NULL;
  if (!req || !hh_filter) {
    goto end;
  }
  walker = req->field_roots;
  while (walker) {
    if (strcasecmp(walker->field_name, hh_filter->from) == 0) {
      req->blocked = 1;
      result = 0;
      goto end;
    }
    walker = walker->next;
  }
end:
  return result;
}
int apply_tgt(HTTPRequest *req, filter *tgt_filter) {
  int result = -1;
  if (!req || !tgt_filter) {
    goto end;
  }
  if (!req->dest_server) {
    goto end;
  }
  if (!strcasecmp(req->dest_server, tgt_filter->host)) {
    req->blocked = 1;
  }
  result = 0;
end:
  return result;
}
int apply_src(HTTPRequest *req, filter *src_filter) {
  int result = -1;
  field *walker = NULL;
  if (!req || !src_filter) {
    goto end;
  }
  walker = req->field_roots;
  while (walker) {
    if (strcasecmp(walker->field_name, "Host") == 0) {
      if (strcasecmp(walker->field_data, src_filter->host) == 0) {
        req->blocked = 1;
        result = 0;
        goto end;
      }
    }
    walker = walker->next;
  }
end:
  return result;
}
int apply_bft(HTTPRequest *req, filter *bft_filter) {
  int result = -1;
  char *ext = NULL;
  char *t = NULL;
  if (!req || !bft_filter) {
    goto end;
  }
  if (!req->request_uri) {
    goto end;
  }
  ext = strrchr(req->request_uri, '.');
  if (!ext) {
    goto end;
  }
  t = calloc(1, strlen(ext) + 1);
  if (!t) {
    goto end;
  }
  strcpy(t, ext + 1);
  if (memcmp(t, bft_filter->ft, strlen(t)) == 0) {
    req->blocked = 1;
    result = 0;
  }
  free(t);
end:
  return result;
}
int apply_cut(HTTPRequest *req, filter *cut_filter) {
  int result = -1;
  char *mt = NULL;
  mimetype *parsed_mime = NULL;
  tag *nt = NULL;
  stream *s = NULL;
  if (!req || !cut_filter) {
    printf("NULL in cut\n");
    goto end;
  }
  if (req->content == NULL) {
    goto end;
  }
  mt = get_field_data(&(req->field_roots), "Content-Type");
  if (!mt) {
    goto end;
  }
  parsed_mime = parse_mimetype(mt);
  if (!parsed_mime) {
    goto end;
  }
  if (strcasecmp(parsed_mime->type, "text")) {
    free_mimetype(parsed_mime);
    goto end;
  }
  if (strcasecmp(parsed_mime->subtype, "html")) {
    free_mimetype(parsed_mime);
    goto end;
  }
  free_mimetype(parsed_mime);
  nt = calloc(1, sizeof(tag));
  if (!nt) {
    goto end;
  }
  if (!parse_tag(req->content, req->content_length, nt)) {
    free_tag(nt);
    goto end;
  }
  cut_all_tags(nt, cut_filter->element);
  s = init_stream_nd(1024);
  if (!s) {
    free_tag(nt);
    goto end;
  }
  print_tag(s, nt, 0);
  free(req->content);
  req->content = s->data;
  req->content_length = s->index;
  free(s);
end:
  mt = NULL;
  parsed_mime = NULL;
  nt = NULL;
  s = NULL;
  return result;
}
int apply_mime_check(HTTPRequest *req, filter *mm_filter) {
  int result = -1;
  char *mt = NULL;
  mimetype *parsed_mime = NULL;
  if (!req || !mm_filter) {
    goto end;
  }
  mt = get_field_data(&(req->field_roots), "Content-Type");
  if (!mt) {
    goto end;
  }
  parsed_mime = parse_mimetype(mt);
  if (!parsed_mime) {
    goto end;
  }
  switch (mm_filter->c) {
  case mm_s:
    if (strcasecmp(mm_filter->mime_subtype, parsed_mime->subtype) == 0) {
      req->blocked = 1;
      result = 0;
      goto end;
    }
    break;
  case mm_t:
    if (strcasecmp(mm_filter->mime_type, parsed_mime->type) == 0) {
      req->blocked = 1;
      result = 0;
      goto end;
    }
    break;
  case mm_p:
    if (strcasecmp(mm_filter->mime_parameter, parsed_mime->parameter) == 0) {
      req->blocked = 1;
      result = 0;
      goto end;
    }
    break;
  default:
    goto end;
  }
end:
  if (parsed_mime) {
    free_mimetype(parsed_mime);
    parsed_mime = NULL;
  }
  return result;
}
int gatekeeper(HTTPRequest *req, filter *f) {
  int result = -1;
  if (!req || !f) {
    goto end;
  }
  switch (f->c) {
  case tr:
    result = apply_tr(req, f);
    break;
  case del:
    result = apply_del(req, f);
    break;
  case blk:
    result = apply_blk(req, f);
    break;
  case cut:
    result = apply_cut(req, f);
    break;
  case hh:
    result = apply_hh(req, f);
    break;
  case tgt:
    result = apply_tgt(req, f);
    break;
  case src:
    result = apply_src(req, f);
    break;
  case bft:
    result = apply_bft(req, f);
    break;
  case mm_t:
  case mm_s:
  case mm_p:
    result = apply_mime_check(req, f);
  default:
    break;
  };
end:
  return result;
}
int apply_filters(HTTPRequest *req, filter *root, int is_request) {
  int result = 0;
  filter *walker = NULL;
  if (!req || !root) {
    goto end;
  }
  walker = root;
  while (walker) {
    if (filter_applies(req->field_roots, req->content, walker, req->dest_server,
                       req->request_uri, is_request)) {
      gatekeeper(req, walker);
    }
    walker = walker->next;
  }
  result = 1;
end:
  return result;
}
