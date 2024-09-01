


#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdio>

#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include <yaml.h>
#include <oniguruma.h>
//#include <yaml-cpp/yaml.h>

#include "sl_syntax.h"
#include "tm_syntax.h"

void
print_help(void)
{
    printf("yaml-test -s <syntax-file> -f <highlight-file> -I<filepath>");
}

enum SYNTAX_TYPE {
    PARSE_TYPE_UNKOWN = 0,
    PARSE_TYPE_TEXT_MATE_SYNTAX,
    PARSE_TYPE_SUBLIME_SYNTAX,
    PARSE_TYPE_TEXT_MATE_THEME,
    PARSE_TYPE_TEXT_MATE_SNIPPET,
    PARSE_TYPE_SUBLIME_THEME,
    PARSE_TYPE_SUBLIME_SNIPPET,
};

// only supports ascii
static bool
str_ends_with(const char *str1, const char *str2, bool cis)
{
    size_t str1sz, str2sz;

    str1sz = strlen(str1);
    str2sz = strlen(str2);

    if (str1sz < str2sz) {
        return false;
    }

    if (cis) {
        // case-insensitive compare
        const char *s1, *s2;
        int n;

        s1 = str1 + (str1sz - str2sz);
        s2 = str2;
        n = str2sz;

        while ( n && *s1 && ( tolower(*s1) == tolower(*s2) )) {
            ++s1;
            ++s2;
            --n;
        }
        if ( n == 0 ) {
            return true;
        } else {
            //return (tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2));
            return false;
        }

    } else {
        return strncmp(str1 + (str1sz - str2sz), str2, str2sz) == 0;
    }
}

int
main(int argc, char *argv[])
{
    char *arg;
    char *syntaxdef;
    char *inputf;
    int end = argc - 1;
    int syntax_type = PARSE_TYPE_UNKOWN;
    printf("now loading sublime syntax:\n");

    for (int i = 1; i < argc; i++) {
        arg = argv[i];

        // just a brutally simple commandline interface
        if (strcmp(arg, "-s") == 0) {
            uint32_t fnamesz;
            
            if (i == end) {
                printf("invalid arguments");
                print_help();
                return 1;
            }

            syntaxdef = argv[++i];
            fnamesz = strlen(syntaxdef);

            if (str_ends_with(syntaxdef, ".sublime-syntax", true)) {
                syntax_type = PARSE_TYPE_SUBLIME_SYNTAX;
            } else if (str_ends_with(syntaxdef, ".tmLanguage", true)) {
                syntax_type = PARSE_TYPE_TEXT_MATE_SYNTAX;
            } else if (str_ends_with(syntaxdef, ".tmTheme", true)) {
                syntax_type = PARSE_TYPE_TEXT_MATE_THEME;
            } else if (str_ends_with(syntaxdef, ".tmSnippet", true)) {
                syntax_type = PARSE_TYPE_TEXT_MATE_SNIPPET;
            } else if (str_ends_with(syntaxdef, ".sublime-snippet", true)) {
                syntax_type = PARSE_TYPE_SUBLIME_SNIPPET;
            } else {

            }

        } else if (strcmp(arg, "-f") == 0) {

            if (i == end) {
                printf("invalid arguments");
                print_help();
                return 1;
            }

            inputf = argv[++i];
        } else if (strcmp(arg, "-I") == 0) {

            if (i == end) {
                printf("invalid arguments");
                print_help();
                return 1;
            }

            inputf = argv[++i];
        }

        printf("args: '%s'\n", argv[i]);
    }

    printf("now loading syntax-defintion of type '%s'\n", syntax_type == PARSE_TYPE_SUBLIME_SYNTAX ? "sublime" : (syntax_type == PARSE_TYPE_TEXT_MATE_SYNTAX ? "textmate" : "unknown"));

    if (syntax_type == PARSE_TYPE_SUBLIME_SYNTAX) {
        sl_syntax_load_file(syntaxdef);
    } else if (syntax_type == PARSE_TYPE_TEXT_MATE_SYNTAX) {
        tm_syntax_load_file(syntaxdef);
    } else if (syntax_type == PARSE_TYPE_TEXT_MATE_SNIPPET) {
        tm_snippet_parse(syntaxdef);
    } else if (syntax_type == PARSE_TYPE_TEXT_MATE_THEME) {
        tm_theme_parse(syntaxdef);
    } else if (syntax_type == PARSE_TYPE_SUBLIME_SNIPPET) {
        sl_snippet_parse(syntaxdef);
    }

    

    //fp = fopen(fpath, "r");

    return 0;
}