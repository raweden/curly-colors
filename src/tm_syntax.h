
#ifndef __GRAMMAR_TM_H_
#define __GRAMMAR_TM_H_

// A simple experiment on implementing a syntax highlighting engine which uses the Sublime's syntax defintion

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// TM GRAMMAR

struct tm_capture_pair {
    const char *name;
    const char *scope;
};

struct tm_syntax_rule {
    const char *name;
    const char *match;
    const char *begin;
    const char *end;
    const char *content_name;
    struct tm_syntax_rule **patterns;
    struct tm_capture_pair *captures;
    struct tm_capture_pair *begin_captures;
    struct tm_capture_pair *end_captures;
    const char *include;
    int patterns_count;
    short captures_count;
    short begin_captures_count;
    short end_captures_count;
    short flags;
};

struct tm_repository_pair {
    const char *name;
    struct tm_syntax_rule *rule;
};

struct tm_syntax_header {
    const char *name;
    const char *scope_name;
    const char **file_extensions;
    const char *folding_start;
    const char *folding_stop;
    const char *first_line_match;
    struct tm_syntax_rule **patterns;
    struct tm_repository_pair *repository;
    const char *uuid;           // from the plist-encoding itself.
    int patterns_count;
    int repository_count;
    int file_extensions_count;
    short flags;
    bool hide_from_user;
};


struct tm_syntax_header *tm_syntax_load_file(const char *filepath);

// TM Theme (don't belong here)

void tm_theme_parse(const char *filepath);
void tm_snippet_parse(const char *filepath);
void sl_snippet_parse(const char *filepath);

#ifdef __cplusplus
}
#endif

#endif /* __GRAMMAR_TM_H_ */