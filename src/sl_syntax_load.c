

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <yaml.h>
#include <oniguruma.h>
//#include <yaml-cpp/yaml.h>

#include "sl_syntax.h"


enum SL_MODEL_KEY {
    SL_PROP_UNKNOWN = 0,
    SL_PROP_NAME,
    SL_PROP_FILE_EXTENSIONS,
    SL_PROP_HIDDEN_FILE_EXTENSIONS,
    SL_PROP_FIRST_LINE_MATCH,
    SL_PROP_SCOPE,
    SL_PROP_VERSION,
    SL_PROP_EXTENDS,
    SL_PROP_HIDDEN,
    SL_PROP_VARIABLES,
    SL_PROP_CONTEXTS,
    // meta scope
    SL_PROP_CTX_META_SCOPE,
    SL_PROP_CTX_META_CONTENT_SCOPE,
    SL_PROP_CTX_META_INCLUDE_PROTOTYPE,
    SL_PROP_CTX_CLEAR_SCOPES,
    SL_PROP_CTX_META_PREPEND,
    SL_PROP_CTX_META_APPEND,
    // context - include
    SL_PROP_CTX_INCLUDE,
    // context - match
    SL_PROP_CTX_MATCH,
    SL_PROP_CTX_SCOPE,
    SL_PROP_CTX_CAPTURES,
    SL_PROP_CTX_PUSH,
    SL_PROP_CTX_POP,
    SL_PROP_CTX_SET,
    SL_PROP_CTX_EMBED,
    SL_PROP_CTX_ESCAPE,
    SL_PROP_CTX_EMBED_SCOPE,
    SL_PROP_CTX_ESCAPE_CAPTURES,
    SL_PROP_CTX_BRANCH,
    SL_PROP_CTX_BRANCH_POINT,
    SL_PROP_CTX_FAIL,
    SL_PROP_CTX_APPLY_PROTOTYPE,
    SL_PROP_CTX_WITH_PROTOTYPE,
};

#define DEFINE_SL_PROP(name, sym) \
    {sym, (sizeof(name) - 1), name}

struct sl_grammar_prop_model {
    char keysym;
    short keysz;
    const char *key;
};

struct sl_grammar_prop_model sl_syntax_root_keys[] = {
    DEFINE_SL_PROP("name", SL_PROP_NAME),
    DEFINE_SL_PROP("file_extensions", SL_PROP_FILE_EXTENSIONS),
    DEFINE_SL_PROP("hidden_file_extensions", SL_PROP_HIDDEN_FILE_EXTENSIONS),
    DEFINE_SL_PROP("first_line_match", SL_PROP_FIRST_LINE_MATCH),
    DEFINE_SL_PROP("scope", SL_PROP_SCOPE),
    DEFINE_SL_PROP("version", SL_PROP_VERSION),
    DEFINE_SL_PROP("extends", SL_PROP_EXTENDS),
    DEFINE_SL_PROP("hidden", SL_PROP_HIDDEN),
    DEFINE_SL_PROP("variables", SL_PROP_VARIABLES),
    DEFINE_SL_PROP("contexts", SL_PROP_CONTEXTS),
};

struct sl_grammar_prop_model sl_syntax_ctx_meta_keys[] = {
    // meta scope
    DEFINE_SL_PROP("meta_scope", SL_PROP_CTX_META_SCOPE),
    DEFINE_SL_PROP("meta_content_scope", SL_PROP_CTX_META_CONTENT_SCOPE),
    DEFINE_SL_PROP("meta_include_prototype", SL_PROP_CTX_META_INCLUDE_PROTOTYPE),
    DEFINE_SL_PROP("clear_scopes", SL_PROP_CTX_CLEAR_SCOPES),
    DEFINE_SL_PROP("meta_prepend", SL_PROP_CTX_META_PREPEND),
    DEFINE_SL_PROP("meta_append", SL_PROP_CTX_META_APPEND),
};

struct sl_grammar_prop_model sl_syntax_ctx_incl_keys[] = {
    DEFINE_SL_PROP("include", SL_PROP_CTX_INCLUDE),
    DEFINE_SL_PROP("apply_prototype", SL_PROP_CTX_APPLY_PROTOTYPE),
};

struct sl_grammar_prop_model sl_syntax_ctx_match_keys[] = {
    // context
    DEFINE_SL_PROP("match", SL_PROP_CTX_MATCH),
    DEFINE_SL_PROP("scope", SL_PROP_CTX_SCOPE),
    DEFINE_SL_PROP("captures", SL_PROP_CTX_CAPTURES),
    DEFINE_SL_PROP("push", SL_PROP_CTX_PUSH),
    DEFINE_SL_PROP("pop", SL_PROP_CTX_POP),
    DEFINE_SL_PROP("set", SL_PROP_CTX_SET),
    DEFINE_SL_PROP("embed", SL_PROP_CTX_EMBED),
    DEFINE_SL_PROP("escape", SL_PROP_CTX_ESCAPE),
    DEFINE_SL_PROP("embed_scope", SL_PROP_CTX_EMBED_SCOPE),
    DEFINE_SL_PROP("escape_captures", SL_PROP_CTX_ESCAPE_CAPTURES),
    DEFINE_SL_PROP("branch", SL_PROP_CTX_BRANCH),
    DEFINE_SL_PROP("branch_point", SL_PROP_CTX_BRANCH_POINT),
    DEFINE_SL_PROP("fail", SL_PROP_CTX_FAIL),
    DEFINE_SL_PROP("apply_prototype", SL_PROP_CTX_APPLY_PROTOTYPE),
    DEFINE_SL_PROP("with_prototype", SL_PROP_CTX_WITH_PROTOTYPE),
};

#undef DEFINE_SL_PROP

static const int sl_syntax_root_keys_count = sizeof(sl_syntax_root_keys) / sizeof(struct sl_grammar_prop_model);
static const int sl_syntax_ctx_meta_keys_count = sizeof(sl_syntax_ctx_meta_keys) / sizeof(struct sl_grammar_prop_model);
static const int sl_syntax_ctx_incl_keys_count = sizeof(sl_syntax_ctx_incl_keys) / sizeof(struct sl_grammar_prop_model);
static const int sl_syntax_ctx_match_keys_count = sizeof(sl_syntax_ctx_match_keys) / sizeof(struct sl_grammar_prop_model);

char
sl_syntax_root_key(const char *keyname, uint32_t keysz)
{
    int isym;

    for (isym = 0; isym < sl_syntax_root_keys_count; isym++)
        if (keysz == sl_syntax_root_keys[isym].keysz && strcmp(keyname, sl_syntax_root_keys[isym].key) == 0)
            break;

    return isym == sl_syntax_root_keys_count ? SL_PROP_UNKNOWN : sl_syntax_root_keys[isym].keysym;
}

char
sl_syntax_ctx_meta_key(const char *keyname, uint32_t keysz)
{
    int isym;

    for (isym = 0; isym < sl_syntax_ctx_meta_keys_count; isym++)
        if (keysz == sl_syntax_ctx_meta_keys[isym].keysz && strcmp(keyname, sl_syntax_ctx_meta_keys[isym].key) == 0)
            break;

    return isym == sl_syntax_ctx_meta_keys_count ? SL_PROP_UNKNOWN : sl_syntax_ctx_meta_keys[isym].keysym;
}

char
sl_syntax_ctx_incl_key(const char *keyname, uint32_t keysz)
{
    int isym;

    for (isym = 0; isym < sl_syntax_ctx_incl_keys_count; isym++)
        if (keysz == sl_syntax_ctx_incl_keys[isym].keysz && strcmp(keyname, sl_syntax_ctx_incl_keys[isym].key) == 0)
            break;

    return isym == sl_syntax_ctx_incl_keys_count ? SL_PROP_UNKNOWN : sl_syntax_ctx_incl_keys[isym].keysym;
}

char
sl_syntax_ctx_match_key(const char *keyname, uint32_t keysz)
{
    int isym;

    for (isym = 0; isym < sl_syntax_ctx_match_keys_count; isym++)
        if (keysz == sl_syntax_ctx_match_keys[isym].keysz && strcmp(keyname, sl_syntax_ctx_match_keys[isym].key) == 0)
            break;

    return isym == sl_syntax_ctx_match_keys_count ? SL_PROP_UNKNOWN : sl_syntax_ctx_match_keys[isym].keysym;
}


// only accepts ascii.
static inline void
strncpylc(char *dst, const char *src, unsigned int sz)
{
    const char *end = src + sz;
    while (src < end) {
        *dst = tolower(*src);
        dst++;
        src++;
    }
}

// maps know boolean scalar values to `1` or `0` and returns `-1` the scalar was not a known boolean.
static int
yaml_scalar_to_bool(const char *scalar, uint32_t length)
{
    char tmp[6];

    switch (length) {
        case 2:
            strncpylc(tmp, scalar, 2);
            if (strncmp(tmp, "on", 2) == 0) {
                return 1;
            } else if (strncmp(tmp, "no", 2) == 0) {
                return 0;
            } else {
                return -1;
            }
            break;
        case 3:
            strncpylc(tmp, scalar, 3);
            if (strncmp(tmp, "yes", 3) == 0) {
                return 1;
            } else if (strncmp(tmp, "off", 3) == 0) {
                return 0;
            } else {
                return -1;
            }
        case 4:
            strncpylc(tmp, scalar, 4);
            if (strncmp(tmp, "true", 4) == 0) {
                return 1;
            } else {
                return -1;
            }
        case 5:
            strncpylc(tmp, scalar, 5);
            if (strncmp(tmp, "false", 5) == 0) {
                return 0;
            } else {
                return -1;
            }
        default:
            return -1;
    }
}

static int
sl_yaml_parse_bool(yaml_parser_t *parser, yaml_event_t *event)
{
    const char *value;
    int length;
    int result;

    result = -1;

    if (event->type != YAML_SCALAR_EVENT) {
        printf("expected scalar value at %s\n", __func__);
        yaml_event_delete(event);
        return result;
    }

    result = yaml_scalar_to_bool((const char *)event->data.scalar.value, event->data.scalar.length);

    yaml_event_delete(event);

    return result;
}

// push, set, include should be passed trough this:
static char *
sl_parse_reference(const char *str, int *reftype, int *strsz)
{
    size_t newsz, orgsz;
    char *nstr;
    char type;

    orgsz = strlen(str);

    if (orgsz >= 6 && strncmp(str, "scope:", 6) == 0) {
        type = SL_CTX_REF_BY_SCOPE;
        nstr = strndup(str + 6, orgsz - 6);
        newsz = orgsz - 6;
    } else if (orgsz >= strncmp(str + (orgsz - 15), ".sublime-syntax", 15) == 0) {
        type = SL_CTX_REF_SYNTAX_FILE;
        nstr = strdup(str);
        newsz = orgsz;
    } else {
        type = SL_CTX_REF_NAMED;
        nstr = strdup(str);
        newsz = orgsz;
    }

    if (reftype != NULL)
        *reftype = type;

    if (strsz != NULL)
        *strsz = newsz;

    return nstr;
}

static void *
sl_yaml_parse_include_pattern(yaml_parser_t *parser, yaml_event_t *event, int flags, int *error)
{
    struct sl_include_pattern *pat;
    char *str;
    int reftype, strsz;
    char keysym;
    bool done;

    done = false;

    pat = (struct sl_include_pattern *)calloc(1, sizeof(struct sl_include_pattern));
    if (pat == NULL)
        return NULL;

    pat->type = SL_PATTERN_INCLUDE;
    pat->refcount = 1;

    while (!done) {

        if (event->type == YAML_MAPPING_END_EVENT) {
            done = true;
            break;
        } else if (event->type == YAML_SCALAR_EVENT) {
            
            keysym = sl_syntax_ctx_incl_key((const char *)event->data.scalar.value, event->data.scalar.length);

            if (keysym == SL_PROP_CTX_INCLUDE) {
                yaml_event_delete(event);
                yaml_parser_parse(parser, event);
                if (event->type != YAML_SCALAR_EVENT) {
                    printf("%s:%d unexpected event-type: %d at %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
                }

                reftype = 0;
                strsz = 0;
                str = sl_parse_reference((const char *)event->data.scalar.value, &reftype, &strsz);
                pat->reftype = reftype;
                if (reftype == SL_CTX_REF_NAMED) {
                    pat->ctxref.named.name = str;
                    pat->ctxref.named.namesz = strsz;
                } else if (reftype == SL_CTX_REF_BY_SCOPE) {
                    pat->ctxref.by_scope.scope = str;
                } else if (reftype == SL_CTX_REF_SYNTAX_FILE) {
                    pat->ctxref.file.filename = str;
                    pat->ctxref.file.filenamesz = strsz;
                } else {
                    printf("unsupported reftype: %d\n", reftype);
                }
                
                yaml_event_delete(event);
                
            } else if (keysym == SL_PROP_CTX_APPLY_PROTOTYPE) {
                yaml_event_delete(event);
                yaml_parser_parse(parser, event);
                if (event->type != YAML_SCALAR_EVENT) {
                    printf("%s:%d unexpected event-type: %d at %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
                }

                reftype = sl_yaml_parse_bool(parser, event);
                if (reftype != -1) {
                    pat->apply_prototype = (bool)reftype;
                }
                
                yaml_event_delete(event);
            } else {
                printf("%s:%d unexpected event-type: %d at %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
            }

        } else {
            printf("%s:%d unexpected event-type: %d at %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
            yaml_event_delete(event);
        }

        if (!done)
            yaml_parser_parse(parser, event);
    }

    return pat;
}

static const char *
sl_parse_pull_next_expect_string(yaml_parser_t *parser, yaml_event_t *event)
{
    yaml_event_delete(event);
    yaml_parser_parse(parser, event);

    if (event->type != YAML_SCALAR_EVENT) {
        return NULL;
    }

    return strdup((const char *)event->data.scalar.value);
}


struct sl_grammar_variable_pair *
sl_yaml_parse_variablesmap(yaml_parser_t *parser, yaml_event_t *event, int *count)
{
    struct sl_grammar_variable_pair *vec, *nvec;
    int cnt, cap;
    bool done;

    if (event->type != YAML_MAPPING_START_EVENT) {
        printf("expected mapping start event at %s:%d got %d\n", __func__, __LINE__, event->type);
        yaml_event_delete(event);
        return NULL;
    }

    yaml_event_delete(event);
    yaml_parser_parse(parser, event);
    
    if (event->type != YAML_SCALAR_EVENT) {
        printf("expected scalar event at %s:%d got %d\n", __func__, __LINE__, event->type);
        yaml_event_delete(event);
        return NULL;
    }

    cap = 8;
    vec = (struct sl_grammar_variable_pair *)calloc(cap, sizeof(struct sl_grammar_variable_pair));
    if (vec == NULL) {
        printf("ENOMEM at %s\n", __func__);
        return NULL;
    }
    done = false;
    cnt = 0;

    while (!done) {
        
        if (event->type == YAML_SCALAR_EVENT) {
            if (cnt == cap) {
                nvec = (struct sl_grammar_variable_pair *)calloc(cap + 8, sizeof(struct sl_grammar_variable_pair));
                memcpy(nvec, vec, cap * sizeof(struct sl_grammar_variable_pair));
                free(vec);
                vec = nvec;
                cap = cap + 8;
            }
            struct sl_grammar_variable_pair *pair = &vec[cnt];
            pair->name = strdup((const char *)event->data.scalar.value);
            pair->namesz = event->data.scalar.length;

            yaml_event_delete(event);
            yaml_parser_parse(parser, event);
            if (event->type != YAML_SCALAR_EVENT) {
                printf("expected scalar event at %s\n", __func__);
                yaml_event_delete(event);
                return NULL;
            }

            pair->value = strdup((const char *)event->data.scalar.value);
            cnt++;
        } else if (event->type == YAML_MAPPING_END_EVENT) {
            done = true;
        } else {
            printf("%s unexpected event-type %d\n", __func__, event->type);
        }
    
        yaml_event_delete(event);
        if (!done)
            yaml_parser_parse(parser, event);
    }

    // optimize return
    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (struct sl_grammar_variable_pair *)calloc(cnt + 1, sizeof(struct sl_grammar_variable_pair));
        memcpy(nvec, vec, cnt * sizeof(struct sl_grammar_variable_pair));
        free(vec);
        vec = nvec;
    }

    if (count != NULL) {
        *count = cnt;
    }

    return vec;
}

/** 
 * expects to enter with a mapping-start in the event argument.
 * TODO: Consider to add the capture scopes into a string-ref-table
 */
struct sl_capture *
sl_yaml_parse_capture_map(yaml_parser_t *parser, yaml_event_t *event, int *count)
{
    struct sl_capture *vec, *nvec;
    int cnt, cap;
    bool done;

    if (event->type != YAML_MAPPING_START_EVENT) {
        printf("expected mapping start event at %s:%d got %d\n", __func__, __LINE__, event->type);
        yaml_event_delete(event);
        return NULL;
    }

    yaml_event_delete(event);

    cap = 8;
    vec = (struct sl_capture *)calloc(cap, sizeof(struct sl_capture));
    if (vec == NULL) {
        printf("ENOMEM at %s\n", __func__);
        return NULL;
    }
    done = false;
    cnt = 0;

    while (!done) {
        
        yaml_parser_parse(parser, event);

        if (event->type == YAML_SCALAR_EVENT) {
            if (cnt == cap) {
                nvec = (struct sl_capture *)calloc(cap + 8, sizeof(struct sl_capture));
                memcpy(nvec, vec, cap * sizeof(struct sl_capture));
                free(vec);
                vec = nvec;
                cap = cap + 8;
            }
            struct sl_capture *pair = &vec[cnt];
            pair->group = atoi((const char *)event->data.scalar.value);

            yaml_event_delete(event);

            yaml_parser_parse(parser, event);
            if (event->type != YAML_SCALAR_EVENT) {
                printf("expected scalar event at %s:%d got %d\n", __func__, __LINE__, event->type);
                yaml_event_delete(event);
                return NULL;
            }

            pair->scope = strdup((const char *)event->data.scalar.value);
            cnt++;
        } else if (event->type == YAML_MAPPING_END_EVENT) {
            done = true;
        } else {
            printf("%s:%d unexpected event-type %d\n", __func__, __LINE__, event->type);
        }
    
        yaml_event_delete(event);
    }

    // optimize return
    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (struct sl_capture *)calloc(cnt + 1, sizeof(struct sl_capture));
        memcpy(nvec, vec, cnt * sizeof(struct sl_capture));
        free(vec);
        vec = nvec;
    }

    if (count != NULL) {
        *count = cnt;
    }

    return vec;
}

static void *
sl_yaml_parse_ctx_name_list(yaml_parser_t *parser, yaml_event_t *event, int *count)
{
    char **vec, **nvec;
    int cnt, cap;
    bool done;

    printf("entering %s\n", __func__);

    if (event->type != YAML_SEQUENCE_START_EVENT) {
        printf("%s:%d expected sequence start found = %d at %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
        yaml_event_delete(event);
        return NULL;
    }

    yaml_event_delete(event);

    cap = 8;
    vec = (char **)calloc(cap, sizeof(void *));
    if (vec == NULL) {
        printf("ENOMEM at %s\n", __func__);
        return NULL;
    }
    done = false;
    cnt = 0;

    while (!done) {
        
        yaml_parser_parse(parser, event);

        if (event->type == YAML_SCALAR_EVENT) {
            if (cnt == cap) {
                nvec = (char **)calloc(cap + 8, sizeof(void *));
                memcpy(nvec, vec, cap * sizeof(void *));
                free(vec);
                vec = nvec;
                cap = cap + 8;
            }
            vec[cnt] = strdup((const char *)event->data.scalar.value);
            cnt++;
        } else if (event->type == YAML_SEQUENCE_END_EVENT) {
            done = true;
        } else {
            printf("%s:%d unexpected event-type: %d at %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
        }
    
        yaml_event_delete(event);
    }

    // optimize return
    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (char **)calloc(cnt + 1, sizeof(void *));
        memcpy(nvec, vec, cnt * sizeof(void *));
        free(vec);
        vec = nvec;
    }

    if (count != NULL) {
        *count = cnt;
    }

    printf("exiting %s\n", __func__);

    return vec;
}

// match pattern parsing

static void * sl_yaml_parse_context_pattern(yaml_parser_t *parser, yaml_event_t *event, int flags, int *error);

static struct sl_pattern_common **
sl_yaml_parse_nested_pushset_args(yaml_parser_t *parser, yaml_event_t *event, int *count)
{

    if (event->type != YAML_SEQUENCE_START_EVENT) {
        printf("%s:%d expected SEQUENCE_START found = %d at %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
        yaml_event_delete(event);
        return NULL;
    }

    yaml_event_delete(event);

    // list of context names (or syntax include paths)
    struct sl_pattern_common **vec, **nvec;
    int cnt, cap;
    bool done = false;

    cap = 8;
    vec = (struct sl_pattern_common **)calloc(cap, sizeof(void *));
    cnt = 0;
    done = false;

    yaml_parser_parse(parser, event);

    while (!done) {

        if (event->type == YAML_MAPPING_START_EVENT) {
            struct sl_pattern_common *result;
            if (cnt == cap) {
                nvec = (struct sl_pattern_common **)calloc(cap + 8, sizeof(void *));
                memcpy(nvec, vec, cap * sizeof(void *));
                free(vec);
                vec = nvec;
                cap = cap + 8;
            }
            
            result = (struct sl_pattern_common *)sl_yaml_parse_context_pattern(parser, event, 0, NULL);
            vec[cnt] = result;
            cnt++;
        } else if (event->type == YAML_SEQUENCE_END_EVENT) {
            done = true;
        } else {
            printf("unexpected event type..\n");
            yaml_event_delete(event);
            return NULL;
        }

        yaml_event_delete(event);
        
        if (!done)
            yaml_parser_parse(parser, event);
    }

    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (struct sl_pattern_common **)calloc(cnt + 1, sizeof(void *));
        memcpy(nvec, vec, cnt * sizeof(void *));
        free(vec);
        vec = nvec;
    }

    if (count != NULL)
        *count = cnt;

    return vec;
}

static struct sl_context_ref *
sl_create_inline_context_ref(const char *str, size_t strsz)
{
    struct sl_context_ref *ctxref;
    char *strbuf;

    strbuf = malloc(sizeof(struct sl_context_ref) + strsz + 1);
    if (strbuf == NULL)
        return NULL;

    ctxref = (struct sl_context_ref *)strbuf;
    memset(ctxref, 0, sizeof(struct sl_context_ref));
    strbuf = strbuf + sizeof(struct sl_context_ref);
    memcpy(strbuf, str, strsz);
    *(strbuf + strsz) = '\0';

    ctxref->type = SL_CONTEXT_REF;
    ctxref->type = 1;
    ctxref->refsz = strsz;
    ctxref->reference = strbuf;
    
    return ctxref;
}

static int
sl_yaml_parse_match_pushset(yaml_parser_t *parser, yaml_event_t *event, struct sl_match_pattern *ctx, struct sl_match_pushset *dst)
{
    if (event->type == YAML_SCALAR_EVENT) {
        struct sl_context_ref *ctxref;
        struct sl_pattern_common **vec;

        vec = (struct sl_pattern_common **)calloc(2, sizeof(void *));

        ctxref = sl_create_inline_context_ref((const char *)event->data.scalar.value, event->data.scalar.length);
        if (ctxref != NULL) {
            vec[0] = (struct sl_pattern_common *)ctxref;
        }

        dst->array_count = 1;
        dst->contexts = (struct sl_match_pattern **)vec;

        yaml_event_delete(event);

        return 0;
    } else if (event->type == YAML_SEQUENCE_START_EVENT) {
        
        yaml_event_delete(event);

        // list of context names (or syntax include paths)
        struct sl_pattern_common **vec, **nvec;
        int cnt, cap;
        bool done = false;

        cap = 8;
        vec = (struct sl_pattern_common **)calloc(cap, sizeof(void *));
        cnt = 0;
        done = false;

        yaml_parser_parse(parser, event);

        while (!done) {

            if (event->type == YAML_SEQUENCE_START_EVENT) {
                struct sl_pattern_common **vecp;
                struct sl_nested_group *ngrp;
                int vector_count;
                if (cnt == cap) {
                    nvec = (struct sl_pattern_common **)calloc(cap + 8, sizeof(void *));
                    memcpy(nvec, vec, cap * sizeof(void *));
                    free(vec);
                    vec = nvec;
                    cap = cap + 8;
                }
                vector_count = 0;
                vecp = (struct sl_pattern_common **)sl_yaml_parse_nested_pushset_args(parser, event, &vector_count);
                
                if (vecp != NULL) {
                    ngrp = (struct sl_nested_group *)calloc(1, sizeof(struct sl_nested_group));
                    ngrp->type = SL_PATTERN_NESTED_GROUP;
                    ngrp->count = vector_count;
                    ngrp->contexts = vecp;
                    vec[cnt] = (struct sl_pattern_common *)ngrp;
                    cnt++;
                }
                
            } else if (event->type == YAML_MAPPING_START_EVENT) {
                struct sl_pattern_common *result;
                if (cnt == cap) {
                    nvec = (struct sl_pattern_common **)calloc(cap + 8, sizeof(void *));
                    memcpy(nvec, vec, cap * sizeof(void *));
                    free(vec);
                    vec = nvec;
                    cap = cap + 8;
                }
                
                result = (struct sl_pattern_common *)sl_yaml_parse_context_pattern(parser, event, 0, NULL);
                vec[cnt] = result;
                cnt++;
            } else if (event->type == YAML_SCALAR_EVENT) {
                struct sl_context_ref *ctxref;
                if (cnt == cap) {
                    nvec = (struct sl_pattern_common **)calloc(cap + 8, sizeof(void *));
                    memcpy(nvec, vec, cap * sizeof(void *));
                    free(vec);
                    vec = nvec;
                    cap = cap + 8;
                }

                /*strsz = event->data.scalar.length;
                strbuf = malloc(sizeof(struct sl_context_ref) + strsz + 1);
                ctxref = (struct sl_context_ref *)strbuf;
                strbuf = strbuf + sizeof(struct sl_context_ref);
                memcpy(strbuf, (const char *)event->data.scalar.value, strsz);
                *(strbuf + strsz) = '\0';

                ctxref->type = SL_CONTEXT_REF;
                ctxref->type = 1;
                ctxref->refsz = strsz;
                ctxref->reference = strbuf;*/
                ctxref = sl_create_inline_context_ref((const char *)event->data.scalar.value, event->data.scalar.length);
                if (ctxref != NULL) {
                    vec[cnt] = (struct sl_pattern_common *)ctxref;
                    cnt++;
                }
            } else if (event->type == YAML_SEQUENCE_END_EVENT) {
                done = true;
            } else {
                printf("unexpected event type..\n");
                yaml_event_delete(event);
                return -1;
            }

            yaml_event_delete(event);
            
            if (!done)
                yaml_parser_parse(parser, event);
        }

        if (cnt == 0) {
            free(vec);
            vec = NULL;
        } else if (cnt + 1 != cap) {
            nvec = (struct sl_pattern_common **)calloc(cnt + 1, sizeof(void *));
            memcpy(nvec, vec, cnt * sizeof(void *));
            free(vec);
            vec = nvec;
        }

        dst->array_count = cnt;
        dst->contexts = (struct sl_match_pattern **)vec;

        return 0;
    } else {
        printf("%s:%d unexpected event %d at %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
    }
        

    return 0;
}

static inline bool
sl_is_valid_varname_char(int chr)
{
    return (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z') || (chr >= '0' && chr <= '9') || chr == '_';
}

/**
 * Replaces variable substitution.
 * 
 * @return In case of no substitution is found; the pointer given in `src` returned, otherwise a new string is allocated. The caller is responsible for managing memory of the new and old string.
 */
static const char *
sl_compile_regexp_variable_substitution(const char *src, const char *(*sub_find)(void *data, const char *name, uint32_t namesz), void *data, const char *self_name)
{
    struct sub_range {
        int offset;          // offset into src
        int length;          // length of the substitution relative to `offset` including the `{{VAR_NAME}}`
        const char *value;  // cache of the value that the range is to be substituted with.
    } subvec[32];
    struct sub_range *vecp, *subp; // pointer to allows subvec to be heap allocated if needed.
    const char *ptr, *ptrend, *sub, *subval, *var_start, *var_end, *name_start, *name_end;
    char *dst, *dstptr;
    uint32_t cnt, cap, namesz, dstsz, snamesz;
    bool var_done, str_done, error;

    if (src == NULL || *src == '\0') {
        return NULL;
    }

    vecp = subvec;      // there is no need to zero intialize the vector.
    cap = sizeof(subvec) / sizeof(struct sub_range);
    cnt = 0;            // number of substitution found
    dstsz = 0;          // number of bytes required for the resulting string.
    snamesz = 0;
    ptr = src;
    var_end = NULL;     // set to null to indicate that there is initially no last subsitution end.
    str_done = false;
    error = false;

    if (self_name != NULL)
        snamesz = strlen(self_name);

    while (!str_done) {

        if (*(ptr) == '{' && *(ptr + 1) == '{') {
            var_start = ptr;
            sub = ptr + 2;
            dstsz += var_end != NULL ? ptr - var_end : ptr - src;
            name_start = sub;
            name_end = NULL;
            var_done = false;
            while (!var_done) {
                if (*(sub) == '}' && *(sub + 1) == '}') {
                    var_end = sub + 2;
                    name_end = sub;
                    sub += 2;
                    var_done = true;
                } else if (*(sub) == '\0') {
                    // invalid end
                    sub++;
                    var_done = true;
                    str_done = true;
                    ptrend = sub;
                } else if (sl_is_valid_varname_char(*sub) == false) {
                    var_done = true;
                } else {
                    sub++;
                }
            }

            if (name_end != NULL && name_end > name_start) {
                namesz = name_end - name_start;

                if (self_name && namesz == snamesz && strncmp(name_start, self_name, snamesz) == 0) {
                    printf("recursive variable reference %.*s in variable declaration\n", namesz, name_start);
                    return src;
                }

                subval = sub_find(data, name_start, namesz);

                if (subval == NULL) {
                    if (vecp != subvec) {
                        free(vecp);
                    }

                    printf("missing %.*s in variable declaration\n", namesz, name_start);
                    
                    return src;
                }

                //printf("should replace '%.*s' with '%s'\n", namesz, name_start, subval);
                
                subp = vecp + cnt;
                subp->offset = var_start - src;
                subp->length = var_end - var_start;
                subp->value = subval;
                dstsz += strlen(subval);
                cnt++;

                ptr = sub;
            
            } else if (!str_done) {
                ptr += 2;
            }

        } else if (*(ptr) == '\0') {
            str_done = true;
            ptrend = ptr;
        } else {
            ptr++;   
        }
    }

    // if no substitutions return the original string
    if (cnt == 0) {
        return src;
    }

    if (var_end != ptrend) {
        dstsz += ptr - var_end;
    }

    var_end = src;

    dst = (char *)malloc(dstsz + 1);
    if (dst == NULL)
        return NULL;

    dstptr = dst;
    for (int i = 0; i < cnt; i++) {
        subp = vecp + i;
        sub = src + subp->offset;
        if (var_end != sub) {
            namesz = sub - var_end;
            memcpy(dstptr, var_end, namesz);
            dstptr += namesz;
        }
        namesz = strlen(subp->value);
        memcpy(dstptr, subp->value, namesz);
        dstptr += namesz;
        var_end = sub + subp->length;
    }

    // if a substitution wasn't the end copy the last portion of the original.
    if (var_end != ptrend) {
        namesz = ptrend - var_end;
        memcpy(dstptr, var_end, namesz);
        dstptr += namesz;
    }

    *dstptr = '\0';

    return dst;
}

static const char *
sl_find_mapped_variable(void *data, const char *name, uint32_t namesz)
{
    struct sl_grammar_def *hdr;
    struct sl_grammar_variable_pair *ptr, *end;
    size_t vnamesz;

    hdr = (struct sl_grammar_def *)data;
    if (hdr->variables_map == NULL || hdr->variables_map_count == 0)
        return NULL;

    ptr = hdr->variables_map;
    end = ptr + hdr->variables_map_count;

    while (ptr < end) {
        // to avoid copying we check the length as well.
        if (namesz == ptr->namesz && strncmp(ptr->name, name, namesz) == 0) {
            return ptr->value;
        }

        ptr++;
    }

    return NULL;
}



static int
sl_yaml_parse_match_pop(yaml_parser_t *parser, yaml_event_t *event)
{
    const char *value;
    int length, result;

    yaml_event_delete(event);
    yaml_parser_parse(parser, event);

    if (event->type != YAML_SCALAR_EVENT) {
        yaml_event_delete(event);
        return -1;
    }

    value = (const char *)event->data.scalar.value;
    length = event->data.scalar.length;

    result = yaml_scalar_to_bool(value, length);
    if (result == -1) {
        result = atoi(value);
    }

    yaml_event_delete(event);

    return result;
}

static void *
sl_yaml_parse_match_pattern(yaml_parser_t *parser, yaml_event_t *event, int flags, int *error)
{
    struct sl_match_pattern *ctx;
    int count, ret;
    char keysym;
    bool done;

    ctx = (struct sl_match_pattern *)calloc(1, sizeof(struct sl_match_pattern));
    if (ctx == NULL) {
        return NULL;
    }
    done = false;
    ctx->type = SL_PATTERN_MATCH;

    while (!done) {

        if (event->type == YAML_SCALAR_EVENT) {
            keysym = sl_syntax_ctx_match_key((const char *)event->data.scalar.value, event->data.scalar.length);
            if (keysym == SL_PROP_UNKNOWN) {
                printf("found unkown key '%s'\n", (const char *)event->data.scalar.value);
                return NULL;
                // TODO: skip sub-scope
            }
            
            switch (keysym) {
                case SL_PROP_CTX_MATCH:
                    ctx->match = sl_parse_pull_next_expect_string(parser, event);
                    break;
                case SL_PROP_CTX_SCOPE:
                    ctx->scope = sl_parse_pull_next_expect_string(parser, event);
                    break;
                case SL_PROP_CTX_CAPTURES:
                    yaml_event_delete(event);
                    yaml_parser_parse(parser, event);
                    count = 0;
                    ctx->captures = sl_yaml_parse_capture_map(parser, event, &count);
                    ctx->capture_count = count;
                    break;
                case SL_PROP_CTX_POP:
                    ret = sl_yaml_parse_match_pop(parser, event);
                    if (ret != -1) {
                        ctx->pop_count = ret;
                    }
                    break;
                case SL_PROP_CTX_PUSH:
                    if (ctx->opcode != 0) {

                    }

                    yaml_event_delete(event);
                    yaml_parser_parse(parser, event);

                    ctx->opcode = GRAMMAR_OP_PUSH;
                    ret = sl_yaml_parse_match_pushset(parser, event, ctx, &ctx->op_push);
                    break;
                case SL_PROP_CTX_SET:
                    if (ctx->opcode != 0) {

                    }
                    
                    yaml_event_delete(event);
                    yaml_parser_parse(parser, event);

                    ctx->opcode = GRAMMAR_OP_SET;
                    ret = sl_yaml_parse_match_pushset(parser, event, ctx, &ctx->op_set);
                    break;
                case SL_PROP_CTX_EMBED:
                    if (ctx->opcode != 0) {

                    }
                    ctx->opcode = GRAMMAR_OP_EMBED;
                    break;
                case SL_PROP_CTX_BRANCH:
                    if (ctx->opcode != 0 && ctx->opcode != GRAMMAR_OP_BRANCH) {

                    }
                    
                    yaml_event_delete(event);
                    yaml_parser_parse(parser, event);

                    count = 0;
                    ctx->opcode = GRAMMAR_OP_BRANCH;
                    //ctx->op_branch.names = (const char **)sl_yaml_parse_ctx_name_list(parser, event, &count);
                    ret = sl_yaml_parse_match_pushset(parser, event, ctx, (struct sl_match_pushset *)&ctx->op_branch);
                    break;
                case SL_PROP_CTX_BRANCH_POINT:
                    if (ctx->opcode != 0 && ctx->opcode != GRAMMAR_OP_BRANCH) {

                    }
                    ctx->opcode = GRAMMAR_OP_BRANCH;
                    ctx->op_branch.branch_point = sl_parse_pull_next_expect_string(parser, event);
                    break;
                case SL_PROP_CTX_FAIL:
                    if (ctx->opcode != 0) {

                    }
                    ctx->opcode = GRAMMAR_OP_FAIL;
                    ctx->op_fail.name = sl_parse_pull_next_expect_string(parser, event);
                    break;
                case SL_PROP_CTX_ESCAPE:
                    ctx->op_embed.escape = sl_parse_pull_next_expect_string(parser, event);
                    break;
                case SL_PROP_CTX_ESCAPE_CAPTURES:
                    count = 0;
                    ctx->op_embed.escape_captures = sl_yaml_parse_capture_map(parser, event, &count);
                    ctx->op_embed.escape_captures_count = count;
                    break;
                default:
                    printf("%sunexpected key symbol %s\n", __func__, (const char *)event->data.scalar.value);
            }

        } else if (event->type == YAML_MAPPING_END_EVENT) {
            done = true;
        } else {
            printf("%s:%d found unexpected token %d %zu:%zu\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
        }
        
        yaml_event_delete(event);
        if (!done)
            yaml_parser_parse(parser, event);
    }
    
    return ctx;
}

static void *
sl_yaml_parse_meta_pattern(yaml_parser_t *parser, yaml_event_t *event, int flags, int *error)
{
    struct sl_meta_pattern *pat;
    int result;
    char keysym;
    bool done;

    pat = (struct sl_meta_pattern *)malloc(sizeof(struct sl_meta_pattern));
    if (pat == NULL)
        return NULL;
    
    memset(pat, 0, sizeof(struct sl_meta_pattern));
    pat->type = SL_PATTERN_META;
    pat->refcount = 1;
    done = false;

    while (!done) {
        
        if (event->type == YAML_MAPPING_END_EVENT) {
            done = true;
            break;
        } else if (event->type == YAML_SCALAR_EVENT) {

            keysym = sl_syntax_ctx_meta_key((const char *)event->data.scalar.value, event->data.scalar.length);
            switch (keysym) {
                case SL_PROP_CTX_META_SCOPE:
                    pat->meta_scope = sl_parse_pull_next_expect_string(parser, event);
                    break;
                case SL_PROP_CTX_META_CONTENT_SCOPE:
                    pat->meta_content_scope = sl_parse_pull_next_expect_string(parser, event);
                    break;
                case SL_PROP_CTX_META_INCLUDE_PROTOTYPE:
                    result = -1;
                    yaml_event_delete(event);
                    yaml_parser_parse(parser, event);
                    result = sl_yaml_parse_bool(parser, event);
                    if (result != -1) {
                        pat->meta_include_prototype = result;
                    }
                    break;
                case SL_PROP_CTX_CLEAR_SCOPES:
                    result = -1;
                    yaml_event_delete(event);
                    yaml_parser_parse(parser, event);
                    result = sl_yaml_parse_bool(parser, event);
                    if (result != -1) {
                        pat->clear_scopes = result;
                    }
                    break;
                case SL_PROP_CTX_META_PREPEND:
                    result = -1;
                    yaml_event_delete(event);
                    yaml_parser_parse(parser, event);
                    result = sl_yaml_parse_bool(parser, event);
                    if (result != -1) {
                        pat->meta_prepend = result;
                    }
                    break;
                case SL_PROP_CTX_META_APPEND:
                    result = -1;
                    yaml_event_delete(event);
                    yaml_parser_parse(parser, event);
                    result = sl_yaml_parse_bool(parser, event);
                    if (result != -1) {
                        pat->meta_append = result;
                    }
                    break;
                default:
                    printf("unexpected key value %s at %s\n", event->data.scalar.value, __func__);
                    break;
            }

        } else {
            printf("expected yaml value %d at %s\n", event->type, __func__);
            continue;
        }
        
        if (!done)
            yaml_parser_parse(parser, event);
    }

    return pat;
}

static void *
sl_yaml_parse_context_pattern(yaml_parser_t *parser, yaml_event_t *event, int flags, int *error)
{
    char keysym;

    if (event->type != YAML_MAPPING_START_EVENT) {
        printf("expected mapping start at %s\n", __func__);
        yaml_event_delete(event);
        return NULL;
    }

    yaml_event_delete(event);

    yaml_parser_parse(parser, event);
    if (event->type != YAML_SCALAR_EVENT) {
        printf("expected scalar value at %s..\n", __func__);
        yaml_event_delete(event);
        return NULL;
    }

    keysym = sl_syntax_ctx_incl_key((const char *)event->data.scalar.value, event->data.scalar.length);

    if (keysym != SL_PROP_UNKNOWN) {
        return sl_yaml_parse_include_pattern(parser, event, 0, error);
    }

    keysym = sl_syntax_ctx_meta_key((const char *)event->data.scalar.value, event->data.scalar.length);
    if (keysym != SL_PROP_UNKNOWN) {
        return sl_yaml_parse_meta_pattern(parser, event, 0, error);
    } else {
        return sl_yaml_parse_match_pattern(parser, event, 0, error);
    }

    return NULL;
}

static struct sl_grammar_ctx_map_pair *
sl_ctxmap_get_from_name(struct sl_grammar_ctx_map_pair *ctxmap, int mapcnt, const char *keyname, uint32_t keysz)
{
    struct sl_grammar_ctx_map_pair *ptr, *end;

    if (mapcnt == 0)
        return NULL;

    ptr = ctxmap;
    end = ptr + mapcnt;

    while (ptr < end) {
        if (ptr->namesz == keysz && strcmp(ptr->name, keyname) == 0) {
            return ptr;
        }
        ptr++;
    }

    return NULL;
}

static inline void *
sl_resize_vector(void *src, int newsz, int elemsz)
{
    void *dst;

    dst = calloc(newsz, elemsz);
    if (dst == NULL) {
        printf("did run out of memory at %s:%d", __func__, __LINE__);
        return NULL;
    }

    if (src == NULL)
        return dst;

    memcpy(dst, src, newsz * elemsz);
    free(src);
    return dst;
}

static void *
sl_yaml_parse_contextmap(struct sl_grammar_def *syntax, yaml_parser_t *parser, yaml_event_t *event, int *count, int *error)
{
    struct sl_grammar_ctx_map_pair *ctxmap;
    int mapcnt, mapcap;
    bool done;

    if (event->type != YAML_MAPPING_START_EVENT) {
        printf("expected mapping start at %s\n", __func__);
        yaml_event_delete(event);
        return NULL;
    }

    yaml_event_delete(event);

    done = false;
    mapcnt = 0;
    mapcap = 8;
    ctxmap = (struct sl_grammar_ctx_map_pair *)calloc(mapcap, sizeof(struct sl_grammar_ctx_map_pair));
    if (ctxmap == NULL) {
        printf("alloc failed returning NULL\n");
        return NULL;
    }


    while (!done) {

        yaml_parser_parse(parser, event);
        if (event->type == YAML_SCALAR_EVENT) {
            struct sl_grammar_ctx_map_pair *pair;
            struct sl_pattern_common **vec, **nvec, *result;
            int cnt, cap;
            bool arr_done;

            pair = sl_ctxmap_get_from_name(ctxmap, mapcnt, (const char *)event->data.scalar.value, event->data.scalar.length);
            if (pair != NULL) {
                printf("%s already exists in context-map..\n", pair->name);
                yaml_event_delete(event);
                return NULL; // TODO: free memory
            }

            if (mapcnt >= mapcap) {
                struct sl_grammar_ctx_map_pair *new_map = (struct sl_grammar_ctx_map_pair *)calloc(mapcap + 8, sizeof(struct sl_grammar_ctx_map_pair));
                memcpy(new_map, ctxmap, sizeof(struct sl_grammar_ctx_map_pair) * mapcnt);
                free(ctxmap);
                ctxmap = new_map;
                mapcap = mapcap + 8;
            }

            pair = ctxmap + mapcnt;
            mapcnt++;
            pair->name = strdup((const char *)event->data.scalar.value);
            pair->namesz = event->data.scalar.length;
            yaml_event_delete(event);

            printf("created context-map entry with name '%s'\n", pair->name);

            yaml_parser_parse(parser, event);
            if (event->type != YAML_SEQUENCE_START_EVENT) {
                printf("expected sequence start at %s..\n", __func__);
                yaml_event_delete(event);
                return NULL; // TODO: free memory
            }
            
            yaml_event_delete(event);
            arr_done = false;
            cnt = 0;
            cap = 8;
            vec = (struct sl_pattern_common **)calloc(cap, sizeof(void *));

            while (!arr_done) {
                // next is expected to be a mapping start
                yaml_parser_parse(parser, event);

                if (event->type == YAML_MAPPING_START_EVENT) {
                    
                    result = (struct sl_pattern_common *)sl_yaml_parse_context_pattern(parser, event, 0, error);
                    if (cap == cnt) {
                        nvec = (struct sl_pattern_common **)sl_resize_vector(vec, cap + 8, sizeof(void *));
                        if (nvec == NULL) {
                            return NULL;
                        }
                        vec = nvec;
                        cap = cap + 8;
                    }

                    vec[cnt] = result;
                    cnt++;
                    
                    yaml_event_delete(event);

                } else if (event->type == YAML_SEQUENCE_END_EVENT) {
                    arr_done = true;
                    if (cnt + 1 != cap) {
                        nvec = (struct sl_pattern_common **)sl_resize_vector(vec, cnt + 1, sizeof(void *));
                        if (nvec != NULL) {
                            vec = nvec;
                            cap = cnt + 1;
                        }
                    }
                    
                    pair->contextsz = cnt;
                    pair->context = (struct sl_match_pattern **)vec;

                    yaml_event_delete(event);

                } else {
                    printf("%s:%d unexpected type = %d at %zu:%zu..\n", __func__, __LINE__, event->type, event->start_mark.line, event->start_mark.column);
                    yaml_event_delete(event);
                    return NULL; // TODO: free memory
                }
            }

        } else if (event->type == YAML_MAPPING_END_EVENT) {
            done = true;
            yaml_event_delete(event);
        } else {
            printf("%s:%d unexpected type = %d..\n", __func__, __LINE__, event->type);
            yaml_event_delete(event);
            return NULL; // TODO: free memory
        }
    }

    if (count)
        *count = mapcnt;

    return ctxmap;
}

static char **
sl_yaml_parse_string_array(yaml_parser_t *parser, yaml_event_t *event, int *count, int *error)
{
    char **vec, **nvec;
    int cnt, cap;
    bool done = false;

    cap = 8;
    vec = (char **)calloc(cap, sizeof(void *));
    cnt = 0;
    done = false;

    while (!done) {
        yaml_parser_parse(parser, event);
        if (event->type == YAML_SCALAR_EVENT) {
            if (cnt == cap) {
                nvec = (char **)calloc(cap + 8, sizeof(void *));
                memcpy(nvec, vec, cap * sizeof(void *));
                free(vec);
                vec = nvec;
                cap = cap + 8;
            }
            vec[cnt] = strdup((const char *)event->data.scalar.value);
            cnt++;
        } else if (event->type == YAML_SEQUENCE_END_EVENT) {
            done = true;
        } else {
            printf("unexpected event type..\n");
            yaml_event_delete(event);
            return NULL;
        }
        yaml_event_delete(event);
    }

    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (char **)calloc(cnt + 1, sizeof(void *));
        memcpy(nvec, vec, cnt * sizeof(void *));
        free(vec);
        vec = nvec;
    }

    if (count != NULL) {
        *count = cnt;
    }

    return vec;
}

static void *
sl_yaml_parse_root_full(struct sl_grammar_def *syntax, yaml_parser_t *parser, yaml_event_t *event, int flags, int *error)
{
    int count;
    char keysym;
    bool done = false;

    if (event->type != YAML_MAPPING_START_EVENT) {
        yaml_event_delete(event);
        return NULL;
    } else {
        yaml_event_delete(event);
    }
    
    while (!done) {

        yaml_parser_parse(parser, event);
        if (event->type == YAML_MAPPING_END_EVENT) {
            done = true;
            yaml_event_delete(event);
            return syntax;
        } else if (event->type != YAML_SCALAR_EVENT) {
            printf("expected scalar value at %s:%d got %d..\n", __func__, __LINE__, event->type);
            yaml_event_delete(event);
            return NULL;
        }

        keysym = sl_syntax_root_key((const char *)event->data.scalar.value, event->data.scalar.length);
        if (keysym == SL_PROP_UNKNOWN) {
            printf("unexpected scalar value %s at %s:%d..\n", (const char *)event->data.scalar.value, __func__, __LINE__);
            yaml_event_delete(event);
            return NULL;
        }

        yaml_event_delete(event);

        if (keysym == SL_PROP_NAME) {
            yaml_parser_parse(parser, event);
            if (event->type != YAML_SCALAR_EVENT) {
                printf("expected scalar value %s at %s:%d..\n", event->data.scalar.value, __func__, __LINE__);
                yaml_event_delete(event);
                return NULL;
            }
            syntax->name = strdup((const char *)event->data.scalar.value);
            syntax->namesz = event->data.scalar.length;
            yaml_event_delete(event);

        } else if (keysym == SL_PROP_FILE_EXTENSIONS) {
            char **vec;
            
            yaml_parser_parse(parser, event);
            if (event->type != YAML_SEQUENCE_START_EVENT) {
                printf("expected sequence start at %s:%d..\n", __func__, __LINE__);
                yaml_event_delete(event);
                return NULL;
            }

            yaml_event_delete(event);
            count = 0;
            vec = sl_yaml_parse_string_array(parser, event, &count, error);
            syntax->file_extensions = (const char **)vec;
            syntax->file_extensions_count = count;

        } else if (keysym == SL_PROP_HIDDEN_FILE_EXTENSIONS) {
            char **vec;
            
            yaml_parser_parse(parser, event);
            if (event->type != YAML_SEQUENCE_START_EVENT) {
                printf("expected sequence start at %s:%d..\n", __func__, __LINE__);
                yaml_event_delete(event);
                return NULL;
            }

            yaml_event_delete(event);
            count = 0;
            vec = sl_yaml_parse_string_array(parser, event, &count, error);
            syntax->hidden_file_extensions = (const char **)vec;
            
        } else if (keysym == SL_PROP_FIRST_LINE_MATCH) {
            yaml_parser_parse(parser, event);
            if (event->type != YAML_SCALAR_EVENT) {
                printf("expected scalar value at %s:%d..\n", __func__, __LINE__);
                yaml_event_delete(event);
                return NULL;
            }
            syntax->first_line_match = strdup((const char *)event->data.scalar.value);
            yaml_event_delete(event);
            
        } else if (keysym == SL_PROP_SCOPE) {
            yaml_parser_parse(parser, event);
            if (event->type != YAML_SCALAR_EVENT) {
                printf("expected scalar value at %s:%d..\n", __func__, __LINE__);
                yaml_event_delete(event);
                return NULL;
            }
            syntax->scope = strdup((const char *)event->data.scalar.value);
            syntax->scopesz = event->data.scalar.length;
            yaml_event_delete(event);
            
        } else if (keysym == SL_PROP_VERSION) {
            yaml_parser_parse(parser, event);
            if (event->type != YAML_SCALAR_EVENT) {
                printf("expected scalar value at %s:%d..\n", __func__, __LINE__);
                yaml_event_delete(event);
                return NULL;
            }
            syntax->version = atoi((const char *)event->data.scalar.value);
            yaml_event_delete(event);
            
        } else if (keysym == SL_PROP_EXTENDS) {
            yaml_parser_parse(parser, event);
            if (event->type == YAML_SCALAR_EVENT) {
                char **vec;
                vec = (char **)calloc(2, sizeof(void *));
                vec[0] = strdup((const char *)event->data.scalar.value);
                syntax->extends = (const char **)vec;
                yaml_event_delete(event);

            } else if (event->type == YAML_SEQUENCE_START_EVENT) {
                char **vec;
                
                yaml_event_delete(event);
                count = 0;
                vec = sl_yaml_parse_string_array(parser, event, &count, error);
                syntax->extends = (const char **)vec;

            } else {
                printf("expected scalar or sequence at %s\n", __func__);
                yaml_event_delete(event);
                return NULL;
            }
            
        } else if (keysym == SL_PROP_HIDDEN) {
            yaml_parser_parse(parser, event);
            count = sl_yaml_parse_bool(parser, event);
            if (count != -1) {
                syntax->hidden = count;
            }
            
        } else if (keysym == SL_PROP_VARIABLES) {
            yaml_parser_parse(parser, event);
            count = 0;
            syntax->variables_map = sl_yaml_parse_variablesmap(parser, event, &count);
            syntax->variables_map_count = count;
            
        } else if (keysym == SL_PROP_CONTEXTS) {

            yaml_parser_parse(parser, event);
            count = 0;
            syntax->context_map = (struct sl_grammar_ctx_map_pair *)sl_yaml_parse_contextmap(syntax, parser, event, &count, error);
            syntax->context_map_count = count;
            printf("parsed context-map %p count %d\n", syntax->context_map, syntax->context_map_count);

        } else {
            
        }        
    }

    return syntax;
}

/** 
 * only loads the bare minimum of  properties required to map a source-code file by file-extention or content of first line or mapping by scope or file-path when extending or referencing another `*.sublime-syntax` file within a grammar definition.
 * - name
 * - scope
 * - file_extensions
 * - hidden_file_extensions
 * - first_line_match
 * - hidden
 */
struct sl_grammar_def *
sl_yaml_parse_root_preload(yaml_parser_t *parser, yaml_event_t *event, int *error)
{

    return NULL;
}

// compiling grammar object

// compiles the regexp string(s) into oniguruma regexp
static int
sl_compile_match_pattern(struct sl_grammar_def *def, struct sl_match_pattern *pat)
{
    OnigRegexType *reg;
    OnigErrorInfo einfo;
    const char *match, *new_match, *old_match;
    uint32_t matchsz;
    int ret;

    match = pat->match;

    if (match != NULL) {

        new_match = sl_compile_regexp_variable_substitution(match, sl_find_mapped_variable, def, NULL);
        old_match = match;

        if (new_match != NULL && new_match != match) {
            //printf("replaced substitution:\n  old (%zu) = '%s'\n  new (%zu) = '%s'\n", strlen(match), match, strlen(new_match), new_match);
            pat->match = new_match;
            match = new_match;
        }

        if (match == NULL) {
            return EINVAL;
        }
        
        matchsz = strlen(match);
        reg = NULL;

        ret = onig_new(&reg, (const uint8_t *)match, (const uint8_t *)(match + matchsz), ONIG_OPTION_CAPTURE_GROUP, ONIG_ENCODING_UTF8, ONIG_SYNTAX_DEFAULT, &einfo);
        if (ret != 0) {
            printf("got error when compiling regexp '%s' org = '%s'\n", match, old_match);
            return EINVAL;
        }

        if (old_match) {
            free((char *)old_match);
        }

        pat->match_regexp = reg;
        pat->flags |= SL_CONTEXT_IS_COMPILED;
    }

    // compile embed regular expression if op is set.
    if (pat->opcode == GRAMMAR_OP_EMBED) {
        match = pat->op_embed.escape;

        new_match = sl_compile_regexp_variable_substitution(match, sl_find_mapped_variable, def, NULL);
        if (new_match != match) {
            free((char *)match);
            pat->op_embed.escape = new_match;
            match = new_match;
        }

        matchsz = strlen(match);
        reg = NULL;

        ret = onig_new(&reg, (const uint8_t *)match, (const uint8_t *)(match + matchsz), ONIG_OPTION_CAPTURE_GROUP, ONIG_ENCODING_UTF8, ONIG_SYNTAX_DEFAULT, &einfo);
        pat->op_embed.escape_regexp = reg;
    } else if ((pat->opcode == GRAMMAR_OP_SET || pat->opcode == GRAMMAR_OP_PUSH || pat->opcode == GRAMMAR_OP_BRANCH) && ((struct sl_match_pushset *)&pat->op_set)->contexts != NULL) {
        int ret, count;
        struct sl_pattern_common **subvec, *subpat;
        
        count = pat->op_push.array_count;
        subvec = (struct sl_pattern_common **)(((struct sl_match_pushset *)&pat->op_set)->contexts);

        for (int i = 0; i < count; i++) {
            subpat = subvec[i];
            if (subpat != NULL && subpat->type == SL_PATTERN_MATCH) {
                ret = sl_compile_match_pattern(def, (struct sl_match_pattern *)subpat);
            }
        }
    }

    return 0;
}

static int
sl_compile_grammar_object(struct sl_grammar_def *def)
{
    int ret, xcnt, ycnt;
    struct sl_grammar_ctx_map_pair *ctxpair;
    struct sl_pattern_common *pat;

    // TODO: extend before

    // expand the variable table before expanding match regexp
    ycnt = def->variables_map_count;
    for (int y = 0; y < ycnt; y++) {
        struct sl_grammar_variable_pair *pair = &def->variables_map[y];
        const char *org_value, *new_value;
        org_value = pair->value;
        new_value = sl_compile_regexp_variable_substitution(org_value, sl_find_mapped_variable, def, pair->name);
        if (new_value == NULL) {
            printf("variables: got NULL for expand of '%s'\n", pair->name);
        } else if (new_value != org_value) {
            //printf("replaced substitution of '%s':\n  old (%zu) = '%s'\n  new (%zu) = '%s'\n", pair->name, strlen(org_value), org_value, strlen(new_value), new_value);
            pair->value = new_value;
            free((char *)org_value);
        }
    }
    // since expanding this recursivly would push 512 bytes of stack per recursion this would not be a good option. Instead a number 
    // iterations could be performed until change count is either zero or the limit of interations is reached (example 5) 
    // for as long as the variable is interpolated before it would not cause a problem, but when it interpolated after it causes some issues.

    ycnt = def->context_map_count;

    for (int y = 0; y < ycnt; y++) {
        ctxpair = (struct sl_grammar_ctx_map_pair *)&def->context_map[y];
        if (ctxpair == NULL)
            continue;

        xcnt = ctxpair->contextsz;
        for (int x = 0; x < xcnt; x++) {
            pat = (struct sl_pattern_common *)ctxpair->context[x];
            if (pat == NULL)
                continue;
            if (pat->type == SL_PATTERN_MATCH) {
                ret = sl_compile_match_pattern(def, (struct sl_match_pattern *)pat);
            }
        }
    }

    return 0;
}


// dumping grammar object

// dumps the grammar object to stdout

static void
sl_dump_grammar_pattern(struct sl_pattern_common *pat, int indent)
{
    char tmpbuf[512];
    const char *prefix;
    
    if (indent > 0) {
        char *ptr, *end;
        int count = indent;

        ptr = tmpbuf;
        end = ptr + sizeof(tmpbuf) - 2;

        while (count >= 0 && ptr < end) {
            *(ptr) = '\x20';
            *(ptr + 1) = '\x20';
            ptr += 2;
            count--;
        }

        *ptr = '\0';
        prefix = tmpbuf;

    } else {
        prefix = "";
    }

    if (pat->type == SL_PATTERN_INCLUDE) {
        struct sl_include_pattern *incl_pat = (struct sl_include_pattern *)pat;
        printf("%s{include_pattern include = \"%s\" apply_prototype = %s }\n", prefix, incl_pat->reference, incl_pat->apply_prototype ? "true" : "false");
    } else if (pat->type == SL_PATTERN_META) {

        struct sl_meta_pattern *meta_pat = (struct sl_meta_pattern *)pat;
        printf("%s{meta_pattern\n", prefix);
        printf("%s  meta_scope: '%s'\n", prefix, meta_pat->meta_scope);
        printf("%s  meta_content_scope: '%s'\n", prefix, meta_pat->meta_content_scope);
        printf("%s  meta_include_prototype: %s\n", prefix, meta_pat->meta_include_prototype ? "true" : "false");
        printf("%s  clear_scopes: %s\n", prefix, meta_pat->clear_scopes ? "true" : "false");
        printf("%s  meta_prepend: %s\n", prefix, meta_pat->meta_prepend ? "true" : "false");
        printf("%s  meta_append: %s\n", prefix, meta_pat->meta_append ? "true" : "false");
        printf("%s}\n", prefix);

    } else if (pat->type == SL_PATTERN_MATCH) {

        struct sl_match_pattern *match_pat = (struct sl_match_pattern *)pat;
        printf("%s{match_pattern\n", prefix);
        printf("%s  match: '%s'\n", prefix, match_pat->match);
        printf("%s  scope: '%s'\n", prefix, match_pat->scope);
        if (match_pat->captures != NULL) {
            printf("%s  captures: (%d)\n", prefix, match_pat->capture_count);
            int count = match_pat->capture_count;
            for (int i = 0; i < count; i++) {
                printf("%s    '%d': \"%s\"\n", prefix, match_pat->captures[i].group, match_pat->captures[i].scope);
            }
        } else {
            printf("%s  captures: <null>\n", prefix);
        }
        printf("%s  pop: %d\n", prefix, match_pat->pop_count);

        if (match_pat->opcode == GRAMMAR_OP_SET) {

            printf("%s  set: (%d)\n", prefix, match_pat->op_set.array_count);
            if ((match_pat->flags & SL_MATCH_PATTERN_PUSHSET_NESTED_CONTEXT) != 0) {
                int count = match_pat->op_set.array_count;
                for (int i = 0; i < count; i++) {
                    sl_dump_grammar_pattern((struct sl_pattern_common *)match_pat->op_set.contexts[i], indent + 2);
                }
            } else {
                int count = match_pat->op_set.array_count;
                for (int i = 0; i < count; i++) {
                    printf("%s    \"%s\"\n", prefix, match_pat->op_set.names[i]);
                }
            }
            
        } else if (match_pat->opcode == GRAMMAR_OP_PUSH) {

            printf("%s  push: (%d)\n", prefix, match_pat->op_push.array_count);
            if ((match_pat->flags & SL_MATCH_PATTERN_PUSHSET_NESTED_CONTEXT) != 0) {
                int count = match_pat->op_set.array_count;
                for (int i = 0; i < count; i++) {
                    sl_dump_grammar_pattern((struct sl_pattern_common *)match_pat->op_set.contexts[i], indent + 2);
                }
            } else {
                int count = match_pat->op_set.array_count;
                for (int i = 0; i < count; i++) {
                    printf("%s    \"%s\"\n", prefix, match_pat->op_set.names[i]);
                }
            }

        } else if (match_pat->opcode == GRAMMAR_OP_EMBED) {

            printf("%s  embed: %s'\n", prefix, match_pat->op_embed.embed);
            printf("%s  embed_scope: %s'\n", prefix, match_pat->op_embed.embed_scope);
            printf("%s  escape: %s'\n", prefix, match_pat->op_embed.escape);

            if (match_pat->op_embed.escape_captures != NULL) {
                int count = match_pat->op_embed.escape_captures_count;
                printf("%s  escape_captures: (%d)\n", prefix, match_pat->capture_count);
                for (int i = 0; i < count; i++) {
                    printf("%s    '%d': \"%s\"\n", prefix, match_pat->op_embed.escape_captures[i].group, match_pat->op_embed.escape_captures[i].scope);
                }
            } else {
                printf("%s  escape_captures: <null>\n", prefix);
            }
            
        } else if (match_pat->opcode == GRAMMAR_OP_FAIL) {
            
        } else if (match_pat->opcode == GRAMMAR_OP_BRANCH) {
            
        } else {

        }

        printf("%s  apply_prototype: %s\n", prefix, match_pat->apply_prototype ? "true" : "false");
        printf("%s}\n", prefix);
    } else {
        printf("%s<INVALID PATTERN TYPE { type = %d, @ = %p }>\n", prefix, pat->type, pat);
    }
}

static int
sl_dump_grammar_object(struct sl_grammar_def *def)
{
    if (def == NULL) {
        printf("--- grammar object is null ---\n");
        return -1;
    }

    printf("-------- SL GRAMMAR OBJECT START ---------\n");

    if (def->name != NULL) {
        printf("name: \"%s\" (%d)\n", def->name, def->namesz);
    }

    if (def->file_extensions != NULL && def->file_extensions_count > 0) {
        printf("file_extensions:\n");
        int count = def->file_extensions_count;
        for (int i = 0; i < count; i++) {
            printf("  \"%s\"\n", def->file_extensions[i]);
        }
    }

    if (def->hidden_file_extensions != NULL) {
        printf("hidden_file_extensions:\n");
        int i = 0;
        while (true) {
            const char *strptr = def->hidden_file_extensions[i++];
            if (strptr == NULL)
                break;
            printf("  \"%s\"\n", strptr);
        }
    }

    if (def->first_line_match != NULL) {
        printf("first_line_match: \"%s\"\n", def->first_line_match);
    }

    if (def->scope != NULL) {
        printf("scope: \"%s\" (%d)\n", def->scope, def->scopesz);
    }

    if (def->extends != NULL) {
        printf("extends:\n");
        int i = 0;
        while (true) {
            const char *strptr = def->extends[i++];
            if (strptr == NULL)
                break;
            printf("  \"%s\"\n", strptr);
        }
    } else {
        printf("extends: <null>\n");
    }

    printf("version: %d\n", def->version);

    printf("hidden: %s\n", def->hidden ? "true" : "false");

    if (def->variables_map != NULL) {
        int count = def->variables_map_count;
        printf("variables: (%d)\n", count);
        for (int i = 0; i < count; i++) {
            printf("  \'%s\': \"%s\"\n", def->variables_map[i].name, def->variables_map[i].value);
        }
    } else {
        printf("variables: <null>\n");
    }

    if (def->context_map != NULL) {
        int xcnt, ycnt = def->context_map_count;
        printf("contexts: (%d)\n", ycnt);
        for (int y = 0; y < ycnt; y++) {
            struct sl_grammar_ctx_map_pair *pair = &def->context_map[y];
            xcnt = pair->contextsz;
            printf("  \'%s\': (%d)\n", pair->name, xcnt);
            for (int x = 0; x < xcnt; x++) {
                sl_dump_grammar_pattern((struct sl_pattern_common *)pair->context[x], 2);
            }
        }
    } else {
        printf("contexts: <null>\n");
    }

    printf("--------  SL GRAMMAR OBJECT END  ---------\n");

    return 0;
}

// common handler in-dependant of source for the grammar.
static struct sl_grammar_def *
sl_syntax_load_yaml(yaml_parser_t *parser, int *error)
{
    yaml_event_t event;
    struct sl_grammar_def *def;
    int done, err;
    bool skip_next = false;
    done = 0;

    yaml_parser_parse(parser, &event);
    if (event.type != YAML_STREAM_START_EVENT) {
        printf("expected YAML_STREAM_START_EVENT at %s\n", __func__);
        yaml_event_delete(&event);
        return NULL;
    }

    yaml_event_delete(&event);
    yaml_parser_parse(parser, &event);
    if (event.type != YAML_DOCUMENT_START_EVENT) {
        printf("expected YAML_DOCUMENT_START_EVENT at %s\n", __func__);
        yaml_event_delete(&event);
        return NULL;
    }

    yaml_event_delete(&event);
    yaml_parser_parse(parser, &event);
    if (event.type != YAML_MAPPING_START_EVENT) {
        printf("expected YAML_MAPPING_START_EVENT at %s\n", __func__);
        yaml_event_delete(&event);
        return NULL;
    }

    def = (struct sl_grammar_def *)calloc(1, sizeof(struct sl_grammar_def));

    sl_yaml_parse_root_full(def, parser, &event, 0, &err);

    yaml_event_delete(&event);


    /* Read the event sequence. */
    while (!done) {

        /* Get the next event. */
        if (!yaml_parser_parse(parser, &event)) {
            printf("got error while parsing yaml..\n");
            break;
        }

        if (event.type == YAML_SCALAR_EVENT) {
            printf("yaml event type = YAML_SCALAR_EVENT data.scalar.length = %zu data.scalar.value = '%s'\n", event.data.scalar.length, event.data.scalar.value);
        } else if (event.type == YAML_MAPPING_START_EVENT) {
            printf("yaml event type = YAML_MAPPING_START_EVENT %zu:%zu\n", event.start_mark.line, event.start_mark.column);
        } else if (event.type == YAML_MAPPING_END_EVENT) {
            printf("yaml event type = YAML_MAPPING_END_EVENT %zu:%zu\n", event.start_mark.line, event.start_mark.column);
        } else if (event.type == YAML_SEQUENCE_START_EVENT) {
            printf("yaml event type = YAML_SEQUENCE_START_EVENT %zu:%zu\n", event.start_mark.line, event.start_mark.column);
        } else if (event.type == YAML_SEQUENCE_END_EVENT) {
            printf("yaml event type = YAML_SEQUENCE_END_EVENT %zu:%zu\n", event.start_mark.line, event.start_mark.column);
        } else if (event.type == YAML_DOCUMENT_START_EVENT) {
            printf("yaml event type = YAML_DOCUMENT_START_EVENT\n");
        } else if (event.type == YAML_DOCUMENT_END_EVENT) {
            printf("yaml event type = YAML_DOCUMENT_END_EVENT\n");
        } else {
            printf("yaml got other event type = %d\n", event.type);
        }

        /* Are we finished? */
        done = (event.type == YAML_STREAM_END_EVENT);

        /* The application is responsible for destroying the event object. */
        yaml_event_delete(&event);

    }

    sl_dump_grammar_object(def);

    sl_compile_grammar_object(def);

    return def;
}

/**
 * Loads `*.sublime-syntax` grammar from a in-memory string buffer. Usefull when pre-processing or generating such grammar.
 */
struct sl_grammar_def *
sl_syntax_load_data(const char *buf, size_t bufsz, int *error)
{
    yaml_parser_t parser;
    struct sl_grammar_def *hdr;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_string(&parser, (const unsigned char *)buf, bufsz);

    hdr = sl_syntax_load_yaml(&parser, error);

    yaml_parser_delete(&parser);

    return hdr;
}


// In C/C++ this processing becomes task of keeping what fits in our data-template and skip the rest.
// later for yaml based themes we might want to keep comments to make a adoptive save of our theme.
struct sl_grammar_def *
sl_syntax_load_file(const char *filepath)
{
    yaml_parser_t parser;
    yaml_event_t event;
    struct sl_grammar_def *def;
    int done, err;
    int pre_types[3] = {YAML_STREAM_START_EVENT, YAML_DOCUMENT_START_EVENT, YAML_MAPPING_START_EVENT};
    int post_types[2] = {YAML_DOCUMENT_END_EVENT, YAML_STREAM_END_EVENT};
    bool skip_next = false;

    FILE *fp = fopen(filepath, "rb");

    if (fp == NULL) {
        printf("failed to open file '%s'\n", filepath);
        return NULL;
    }

    yaml_parser_initialize(&parser);

    yaml_parser_set_input_file(&parser, fp);

    done = 0;

    yaml_parser_parse(&parser, &event);
    if (event.type != YAML_STREAM_START_EVENT) {
        printf("expected YAML_STREAM_START_EVENT at %s\n", __func__);
        yaml_event_delete(&event);
        return NULL;
    }

    yaml_event_delete(&event);
    yaml_parser_parse(&parser, &event);
    if (event.type != YAML_DOCUMENT_START_EVENT) {
        printf("expected YAML_DOCUMENT_START_EVENT at %s\n", __func__);
        yaml_event_delete(&event);
        return NULL;
    }

    yaml_event_delete(&event);
    yaml_parser_parse(&parser, &event);
    if (event.type != YAML_MAPPING_START_EVENT) {
        printf("expected YAML_MAPPING_START_EVENT at %s\n", __func__);
        yaml_event_delete(&event);
        return NULL;
    }

    def = (struct sl_grammar_def *)calloc(1, sizeof(struct sl_grammar_def));

    sl_yaml_parse_root_full(def, &parser, &event, 0, &err);

    yaml_event_delete(&event);

    yaml_parser_parse(&parser, &event);

    printf("reached out of sl_yaml_parse_root_full at %zu:%zu\n", event.end_mark.line, event.end_mark.column);    


    /* Read the event sequence. */
    while (!done) {

        /* Get the next event. */
        if (!yaml_parser_parse(&parser, &event)) {
            printf("got error while parsing yaml..\n");
            break;
        }

        if (event.type == YAML_SCALAR_EVENT) {
            //printf("yaml event type = YAML_SCALAR_EVENT data.scalar.length = %zu data.scalar.value = '%s'\n", event.data.scalar.length, event.data.scalar.value);
        } else if (event.type == YAML_MAPPING_START_EVENT) {
            //printf("yaml event type = YAML_MAPPING_START_EVENT %zu:%zu\n", event.start_mark.line, event.start_mark.column);
        } else if (event.type == YAML_MAPPING_END_EVENT) {
            //printf("yaml event type = YAML_MAPPING_END_EVENT %zu:%zu\n", event.start_mark.line, event.start_mark.column);
        } else if (event.type == YAML_SEQUENCE_START_EVENT) {
            //printf("yaml event type = YAML_SEQUENCE_START_EVENT %zu:%zu\n", event.start_mark.line, event.start_mark.column);
        } else if (event.type == YAML_SEQUENCE_END_EVENT) {
            //printf("yaml event type = YAML_SEQUENCE_END_EVENT %zu:%zu\n", event.start_mark.line, event.start_mark.column);
        } else if (event.type == YAML_DOCUMENT_START_EVENT) {
            //printf("yaml event type = YAML_DOCUMENT_START_EVENT\n");
        } else if (event.type == YAML_DOCUMENT_END_EVENT) {
            //printf("yaml event type = YAML_DOCUMENT_END_EVENT\n");
        } else {
            //printf("yaml got other event type = %d\n", event.type);
        }

        /* Are we finished? */
        done = (event.type == YAML_STREAM_END_EVENT);

        /* The application is responsible for destroying the event object. */
        yaml_event_delete(&event);

    }

    //sl_dump_grammar_object(def);

    sl_compile_grammar_object(def);

    yaml_parser_delete(&parser);

    return def;
}