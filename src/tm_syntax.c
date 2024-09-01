

#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <libxml/parser.h>

#include "xml_utils.h"
#include "tm_syntax.h"

enum TM_PROP {
    TM_PROP_UNKNOWN = 0,
    TM_PROP_NAME,
    TM_PROP_SCOPE_NAME,
    TM_PROP_FILE_TYPES,
    TM_PROP_FOLDING_START_MARKER,
    TM_PROP_FOLDING_STOP_MARKER,
    TM_PROP_PATTERNS,
    TM_PROP_FIRST_LINE_MATCH,
    TM_PROP_REPOSITORY,
    TM_PROP_HIDE_FROM_USER,
    TM_PROP_UUID,
    TM_PROP_RULE_MATCH,
    TM_PROP_RULE_BEGIN,
    TM_PROP_RULE_END,
    TM_PROP_RULE_CONTENT_NAME,
    TM_PROP_RULE_CAPTURES,
    TM_PROP_RULE_BEGIN_CAPTURES,
    TM_PROP_RULE_END_CAPTURES,
    TM_PROP_RULE_INCLUDE,
    TM_PROP_RULE_PATTERNS = TM_PROP_PATTERNS,
    TM_PROP_RULE_NAME = TM_PROP_NAME,
};

struct tm_prop {
    short keysym;
    short keysz;
    const char *keyname;
};

#define DEFINE_TM_PROP(name, sym) \
    {sym, (sizeof(name) - 1), name}

struct tm_prop tm_syntax_header_keys[] = {
    DEFINE_TM_PROP("name", TM_PROP_NAME),
    DEFINE_TM_PROP("scopeName", TM_PROP_SCOPE_NAME),
    DEFINE_TM_PROP("fileTypes", TM_PROP_FILE_TYPES),
    DEFINE_TM_PROP("foldingStartMarker", TM_PROP_FOLDING_START_MARKER),
    DEFINE_TM_PROP("foldingStopMarker", TM_PROP_FOLDING_STOP_MARKER),
    DEFINE_TM_PROP("patterns", TM_PROP_PATTERNS),
    DEFINE_TM_PROP("firstLineMatch", TM_PROP_FIRST_LINE_MATCH),
    DEFINE_TM_PROP("repository", TM_PROP_REPOSITORY),
    DEFINE_TM_PROP("hideFromUser", TM_PROP_HIDE_FROM_USER),
    DEFINE_TM_PROP("uuid", TM_PROP_UUID),
    
};

struct tm_prop tm_syntax_rule_keys[] = {
    DEFINE_TM_PROP("name", TM_PROP_RULE_NAME),
    DEFINE_TM_PROP("match", TM_PROP_RULE_MATCH),
    DEFINE_TM_PROP("begin", TM_PROP_RULE_BEGIN),
    DEFINE_TM_PROP("end", TM_PROP_RULE_END),
    DEFINE_TM_PROP("patterns", TM_PROP_RULE_PATTERNS),
    DEFINE_TM_PROP("contentName", TM_PROP_RULE_CONTENT_NAME),
    DEFINE_TM_PROP("captures", TM_PROP_RULE_CAPTURES),
    DEFINE_TM_PROP("beginCaptures", TM_PROP_RULE_BEGIN_CAPTURES),
    DEFINE_TM_PROP("endCaptures", TM_PROP_RULE_END_CAPTURES),
    DEFINE_TM_PROP("include", TM_PROP_RULE_INCLUDE),
};

static const int tm_syntax_header_keys_count = sizeof(tm_syntax_header_keys) / sizeof(struct tm_prop);
static const int tm_syntax_rule_keys_count = sizeof(tm_syntax_rule_keys) / sizeof(struct tm_prop);


static short
tm_syntax_header_key(const char *keyname, uint32_t keysz)
{
    int isym;

    for (isym = 0; isym < tm_syntax_header_keys_count; isym++)
        if (keysz == tm_syntax_header_keys[isym].keysz && strcmp(keyname, tm_syntax_header_keys[isym].keyname) == 0)
            break;

    return isym == tm_syntax_header_keys_count ? TM_PROP_UNKNOWN : tm_syntax_header_keys[isym].keysym;
}

static short
tm_syntax_rule_key(const char *keyname, uint32_t keysz)
{
    int isym;

    for (isym = 0; isym < tm_syntax_rule_keys_count; isym++)
        if (keysz == tm_syntax_rule_keys[isym].keysz && strcmp(keyname, tm_syntax_rule_keys[isym].keyname) == 0)
            break;

    return isym == tm_syntax_rule_keys_count ? TM_PROP_UNKNOWN : tm_syntax_rule_keys[isym].keysym;
}

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void
print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }

        print_element_names(cur_node->children);
    }
}

static struct tm_capture_pair *
tm_parse_capture_groups(struct tm_syntax_header *hdr, xmlNode *dict, int *count)
{
    xmlNode *key_child, *val_child, *sub_child, *scope_child;
    const char *sub_key;
    struct tm_capture_pair *vec, *nvec, *pair;
    int cnt, cap;

    if (!xmlElementIsPListDict(dict)) {
        printf("%s expected value for capture-group to be <dict> found <%s> at line %d\n", __func__, dict->name, dict->line);
        return NULL;
    }

    cap = 8;
    vec = (struct tm_capture_pair *)calloc(cap, sizeof(struct tm_capture_pair));
    cnt = 0;
    key_child = xmlNodeNextElementSibling(dict->children); 

    while (key_child != NULL) {
        
        val_child = xmlNodeNextElementSibling(key_child->next);

        if (xmlElementIsPListKey(key_child) == false || xmlElementIsPListDict(val_child) == false) {
            printf("expected dict pair of type <key> + <dict> found <%s> + <%s>\n", key_child->name, val_child->name);
            break;
        }

        // the value of the capture is actually a dictinary with only one property; name .. for optimization we read capture.name into the value instead since there is no win maintainging a object.
        scope_child = NULL;
        sub_child = xmlNodeNextElementSibling(val_child->children);

        while (sub_child != NULL) {

            if (xmlElementIsPListKey(sub_child) == false) {
                printf("capture dict key values out of sync..\n");
                break;
            }

            sub_key = xmlElementGetPListKey(sub_child);
            if (sub_key && strcmp(sub_key, "name") == 0) {
                scope_child = xmlNodeNextElementSibling(sub_child->next);
                if (!xmlElementIsPListString(scope_child))
                    scope_child = NULL;
                break;
            } else {
                sub_child = xmlNodeNextElementSibling(sub_child->next); // value pos
                if (sub_child)
                    sub_child = xmlNodeNextElementSibling(sub_child->next); // key pos
            }
        }

        if (scope_child == NULL) {
            key_child = xmlNodeNextElementSibling(val_child->next);
            continue;
        }

        if (cnt == cap) {
            nvec = (struct tm_capture_pair *)calloc(cap + 8, sizeof(struct tm_capture_pair));
            memcpy(nvec, vec, cap * sizeof(struct tm_capture_pair));
            free(vec);
            vec = nvec;
            cap = cap + 8;
        }

        pair = vec + cnt;
        pair->name = xmlElementDuplicateTextContent(key_child);
        pair->scope = xmlElementDuplicateTextContent(scope_child);
        cnt++;

        key_child = xmlNodeNextElementSibling(val_child->next);
    }

    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (struct tm_capture_pair *)calloc(cnt + 1, sizeof(struct tm_capture_pair));
        memcpy(nvec, vec, cnt * sizeof(struct tm_capture_pair));
        free(vec);
        vec = nvec;
    }

    if (count != NULL)
        *count = cnt;

    return vec;
}

static struct tm_syntax_rule *tm_parse_rule_pattern(struct tm_syntax_header *hdr, xmlNode *dict);

static struct tm_syntax_rule **
tm_parse_xml_pattern_array(struct tm_syntax_header *hdr, xmlNode *arr, int *count)
{
    struct tm_syntax_rule *pat;
    xmlNode *child;
    struct tm_syntax_rule **vec, **nvec;
    int cnt, cap;

    if (!xmlElementIsPListArray(arr)) {
        printf("%s expected value for 'patterns' to be <array> found <%s> at line %d\n", __func__, arr->name, arr->line);
        return NULL;
    }

    cap = 8;
    vec = (struct tm_syntax_rule **)calloc(cap, sizeof(void *));
    cnt = 0;
    child = xmlNodeNextElementSibling(arr->children);

    while (child != NULL) {
        if (!xmlElementIsPListDict(child)) {
            break;
        }

        if (cnt == cap) {
            nvec = (struct tm_syntax_rule **)calloc(cap + 8, sizeof(void *));
            memcpy(nvec, vec, cap * sizeof(void *));
            free(vec);
            vec = nvec;
            cap = cap + 8;
        }

        pat = tm_parse_rule_pattern(hdr, child);

        vec[cnt] = pat;
        cnt++;

        child = xmlNodeNextElementSibling(child->next);
    }

    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (struct tm_syntax_rule **)calloc(cnt + 1, sizeof(void *));
        memcpy(nvec, vec, cnt * sizeof(void *));
        free(vec);
        vec = nvec;
    }

    if (count != NULL)
        *count = cnt;

    return vec;
}

static int
tm_parse_top_file_types(struct tm_syntax_header *hdr, xmlNode *arr)
{
    xmlNode *child;
    char **vec, **nvec;
    int cnt, cap;
    bool done = false;

    if (!xmlElementIsPListArray(arr)) {
        printf("expected value for 'fileTypes' to be <array> found <%s>\n", arr->name);
        return -1;
    }

    vec = (char **)calloc(8, sizeof(void *));
    cnt = 0;
    cap = 8;
    child = arr->children; 

    while (child != NULL) {
        if (!xmlElementIsPListString(child)) {
            break;
        }

        if (cnt == cap) {
            nvec = (char **)calloc(cap + 8, sizeof(void *));
            memcpy(nvec, vec, cap * sizeof(void *));
            free(vec);
            vec = nvec;
            cap = cap + 8;
        }

        vec[cnt] = xmlElementDuplicateTextContent(child);
        cnt++;

        child = xmlNodeNextElementSibling(child->next);
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

    hdr->file_extensions_count = cnt;
    hdr->file_extensions = (const char **)vec;

    return (0);
}

static struct tm_syntax_rule *
tm_parse_rule_pattern(struct tm_syntax_header *hdr, xmlNode *dict)
{
    struct tm_syntax_rule *pat;
    xmlNode *key_child, *val_child;
    const char *keystr;
    uint32_t keysz, keysym;
    int count;

    if (!xmlElementIsPListDict(dict)) {
        printf("%s expected base of rule to be <dict> found <%s> at line %d\n", __func__, dict->name, dict->line);
        return NULL;
    }

    pat = malloc(sizeof(struct tm_syntax_rule));
    memset(pat, 0, sizeof(struct tm_syntax_rule));

    key_child = xmlNodeNextElementSibling(dict->children);

    while (key_child != NULL) {
        val_child = xmlNodeNextElementSibling(key_child->next);
        if (val_child == NULL) {
            break;
        }

        if (xmlElementIsPListKey(key_child) == false) {
            printf("expected dict pair of type <key> + <dict> found <%s> + <%s>\n", key_child->name, val_child->name);
            break;
        }

        keystr = xmlElementGetPListKey(key_child);
        if (keystr == NULL) {
            printf("expected <%s> at line %d to have a value.. found NULL\n", key_child->name, key_child->line);
            break;
        }
        keysz = strlen(keystr);
        keysym = tm_syntax_rule_key(keystr, keysz);

        switch (keysym) {
            case TM_PROP_RULE_NAME:
                pat->name = xmlElementDuplicateTextContent(val_child);
                break;
            case TM_PROP_RULE_MATCH:
                pat->match = xmlElementDuplicateTextContent(val_child);
                break;
            case TM_PROP_RULE_BEGIN:
                pat->begin = xmlElementDuplicateTextContent(val_child);
                break;
            case TM_PROP_RULE_END:
                pat->end = xmlElementDuplicateTextContent(val_child);
                break;
            case TM_PROP_RULE_CONTENT_NAME:
                pat->content_name = xmlElementDuplicateTextContent(val_child);
                break;
            case TM_PROP_RULE_INCLUDE:
                pat->include = xmlElementDuplicateTextContent(val_child);
                break;
            
            //
            case TM_PROP_RULE_PATTERNS:
                count = 0;
                pat->patterns = tm_parse_xml_pattern_array(hdr, val_child, &count);
                pat->patterns_count = count;
                break;

            //
            case TM_PROP_RULE_CAPTURES:
                count = 0;
                pat->captures = tm_parse_capture_groups(hdr, val_child, &count);
                pat->captures_count = count;
                break;
            case TM_PROP_RULE_BEGIN_CAPTURES:
                count = 0;
                pat->begin_captures = tm_parse_capture_groups(hdr, val_child, &count);
                pat->begin_captures_count = count;
                break;
            case TM_PROP_RULE_END_CAPTURES:
                count = 0;
                pat->end_captures = tm_parse_capture_groups(hdr, val_child, &count);
                pat->end_captures_count = count;
                break;
        }

        key_child = xmlNodeNextElementSibling(val_child->next);
    }

    if (pat->match == NULL && pat->begin != NULL && pat->end != NULL && pat->begin_captures == NULL && pat->end_captures == NULL && pat->captures != NULL) {
        pat->begin_captures = pat->captures;
        pat->end_captures = pat->captures;
        pat->begin_captures_count = pat->captures_count;
        pat->end_captures_count = pat->captures_count;
        pat->captures = NULL;
        pat->captures_count = 0;
    }
    
    return (0);
}

static int
tm_parse_top_patterns(struct tm_syntax_header *hdr, xmlNode *arr)
{
    struct tm_syntax_rule *pat;
    xmlNode *child;
    struct tm_syntax_rule **vec, **nvec;
    int cnt, cap;
    bool done = false;

    if (!xmlElementIsPListArray(arr)) {
        printf("%s expected value for 'patterns' to be <array> found <%s> at line %d\n", __func__, arr->name, arr->line);
        return -1;
    }

    vec = (struct tm_syntax_rule **)calloc(8, sizeof(void *));
    cnt = 0;
    cap = 8;
    child = arr->children; 

    while (child != NULL) {
        if (!xmlElementIsPListDict(child)) {
            break;
        }

        if (cnt == cap) {
            nvec = (struct tm_syntax_rule **)calloc(cap + 8, sizeof(void *));
            memcpy(nvec, vec, cap * sizeof(void *));
            free(vec);
            vec = nvec;
            cap = cap + 8;
        }

        pat = tm_parse_rule_pattern(hdr, child);

        vec[cnt] = pat;
        cnt++;

        child = xmlNodeNextElementSibling(child->next);
    }

    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (struct tm_syntax_rule **)calloc(cnt + 1, sizeof(void *));
        memcpy(nvec, vec, cnt * sizeof(void *));
        free(vec);
        vec = nvec;
    }

    hdr->patterns = (struct tm_syntax_rule **)vec;
    hdr->patterns_count = cnt;

    return (0);
}

static int
tm_parse_top_repository(struct tm_syntax_header *hdr, xmlNode *arr)
{
    struct tm_syntax_rule *pat;
    xmlNode *key_child, *val_child;
    struct tm_repository_pair *vec, *nvec, *pair;
    char *name;
    int cnt, cap;

    if (!xmlElementIsPListDict(arr)) {
        printf("expected value for 'repository' to be <dict> found <%s>\n", arr->name);
        return -1;
    }

    cap = 8;
    vec = (struct tm_repository_pair *)calloc(cap, sizeof(struct tm_repository_pair));
    cnt = 0;
    key_child = xmlNodeNextElementSibling(arr->children); 

    while (key_child != NULL) {
        
        val_child = xmlNodeNextElementSibling(key_child->next);

        if (xmlElementIsPListKey(key_child) == false || xmlElementIsPListDict(val_child) == false) {
            printf("expected dict pair of type <key> + <dict> found <%s> + <%s>\n", key_child->name, val_child->name);
            break;
        }

        if (cnt == cap) {
            nvec = (struct tm_repository_pair *)calloc(cap + 8, sizeof(struct tm_repository_pair));
            memcpy(nvec, vec, cap * sizeof(struct tm_repository_pair));
            free(vec);
            vec = nvec;
            cap = cap + 8;
        }

        pat = tm_parse_rule_pattern(hdr, val_child);

        pair = vec + cnt;
        pair->name = xmlElementGetPListKey(key_child);
        pair->rule = pat;
        cnt++;

        key_child = xmlNodeNextElementSibling(val_child->next);
    }

    if (cnt == 0) {
        free(vec);
        vec = NULL;
    } else if (cnt + 1 != cap) {
        nvec = (struct tm_repository_pair *)calloc(cnt + 1, sizeof(struct tm_repository_pair));
        memcpy(nvec, vec, cnt * sizeof(struct tm_repository_pair));
        free(vec);
        vec = nvec;
    }

    hdr->repository = (struct tm_repository_pair *)vec;
    hdr->repository_count = cnt;

    return (0);
}

static struct tm_syntax_header *
parseTMLangPlist(xmlDoc *doc, int *error)
{
    xmlNode *root_element = NULL;
    xmlNode *dictElem, *key_child, *val_child;
    struct tm_syntax_header *grammar;
    uint32_t keysz, keysym;
    int count;

    grammar = NULL;

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    if (root_element == NULL || root_element->type != XML_ELEMENT_NODE || root_element->name == NULL || strcmp((char *)root_element->name, "plist") != 0) {
        return NULL;
    }

    dictElem = xmlNodeNextElementSibling(root_element->children);

    if (dictElem == NULL || dictElem->type != XML_ELEMENT_NODE || dictElem->name == NULL || strcmp((char *)dictElem->name, "dict") != 0) {
        return NULL;
    }

    grammar = malloc(sizeof(struct tm_syntax_header));
    memset(grammar, 0, sizeof(struct tm_syntax_header));

    key_child = xmlNodeNextElementSibling(dictElem->children);

    while (key_child != NULL) {
        if (xmlElementIsPListKey(key_child) == false) {
            printf("key is not a <key> tag (actual = %s)\n", key_child->name);
            break;
        }
        val_child = xmlNodeNextElementSibling(key_child->next);
        const char *keyname;
        if (val_child == NULL) {
            printf("expected key = '%s' to be followed by a value..\n", key_child->name);
            break;
        }

        keyname = xmlElementGetPListKey(key_child);
        if (keyname == NULL) {
            printf("expected key to have inner content.. found %s\n", keyname);
            break;
        }

        keysz = strlen(keyname);
        keysym = tm_syntax_header_key(keyname, keysz);

        printf("<key>%s</key> sz = %d symbol = %d\n", keyname, keysz, keysym);

        switch (keysym) {
            case TM_PROP_FILE_TYPES:
                if (xmlElementIsPListArray(val_child)) {
                    tm_parse_top_file_types(grammar, val_child);
                } else {
                    printf("expected value for %s to be <array> found <%s>\n", keyname, val_child->name);
                }
                break;
            case TM_PROP_HIDE_FROM_USER:
                if (xmlElementIsPListBoolean(val_child)) {
                    grammar->hide_from_user = xmlElementGetPListBoolean(val_child);
                } else {
                    printf("expected value for %s to be <bool> found <%s>\n", keyname, val_child->name);
                }
                break;
            case TM_PROP_NAME:
                if (xmlElementIsPListString(val_child)) {
                    grammar->name = xmlElementCopyPListString(val_child);
                } else {
                    printf("expected value for %s to be <string> found <%s>\n", keyname, val_child->name);
                }
                break;
            case TM_PROP_PATTERNS:
                if (xmlElementIsPListArray(val_child)) {
                    count = 0;
                    grammar->patterns = tm_parse_xml_pattern_array(grammar, val_child, &count);
                    grammar->patterns_count = count;
                } else {
                    printf("expected value for %s to be <array> found <%s> at line: %d\n", keyname, val_child->name, val_child->line);
                }
                break;
            case TM_PROP_REPOSITORY:
                if (xmlElementIsPListDict(val_child)) {
                    tm_parse_top_repository(grammar, val_child);
                } else {
                    printf("expected value for %s to be <dict> found <%s>\n", keyname, val_child->name);
                }
                break;
            case TM_PROP_SCOPE_NAME:
                if (xmlElementIsPListString(val_child)) {
                    grammar->scope_name = xmlElementCopyPListString(val_child);
                } else {
                    printf("expected value for %s to be <string> found <%s>\n", keyname, val_child->name);
                }
                break;
            case TM_PROP_UUID:
                if (xmlElementIsPListString(val_child)) {
                    grammar->uuid = xmlElementCopyPListString(val_child);
                } else {
                    printf("expected value for %s to be <string> found <%s>\n", keyname, val_child->name);
                }
                break;
            default:
                // do nothing (no need to skip scope in xml)
                break;
        }
        
        // else do nothing do 
    


        if (val_child->next != NULL) {
            key_child = xmlNodeNextElementSibling(val_child->next);
        } else {
            break;
        }
    }

    printf("Grammar { name = '%s' scopeName = '%s' uuid = '%s' }\n", grammar->name, grammar->scope_name, grammar->uuid);

    return grammar;
}


struct tm_syntax_header *
tm_syntax_load_file(const char *filepath)
{
    struct tm_syntax_header *def;
    xmlDoc *doc = NULL;
    int err;
    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(filepath, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", filepath);
        return NULL;
    }

    def = parseTMLangPlist(doc, &err);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return def;
}