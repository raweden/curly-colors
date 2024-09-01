
#ifndef __GRAMMAR_SL_H_
#define __GRAMMAR_SL_H_

// A simple experiment on implementing a syntax highlighting engine which uses the Sublime's syntax defintion

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum SL_GRAMMAR_OPS {
    GRAMMAR_OP_NOOP = 0,
    GRAMMAR_OP_PUSH,
    GRAMMAR_OP_SET,
    GRAMMAR_OP_EMBED,
    GRAMMAR_OP_BRANCH,
    GRAMMAR_OP_FAIL
};

enum SL_GRAMMAR_FLAGS {
    STX_FLAG_APPLY_PROTOTYPE = (1 << 1),
    META_INCLUDE_PROTOTYPE = (1 << 2),
    SL_CONTEXT_IS_COMPILED = (1 << 3),  // when set `path.match_regexp` should be accessed rathr than `pat.match`
    SL_MATCH_PATTERN_PUSHSET_NESTED_CONTEXT = (1 << 4),
    SL_MATCH_PATTERN_PUSHSET_RESOLVED_REF_CONTEXT = (1 << 5),
};

enum SL_PATTERN_TYPE {
    SL_PATTERN_UKNOWN = 0,
    SL_PATTERN_META,
    SL_PATTERN_INCLUDE,
    SL_PATTERN_MATCH,
    SL_CONTEXT_REF,
    SL_PATTERN_NESTED_GROUP,
};

/**
 * The `struct sl_pattern_common` is never actually allocated or found but rather
 * describes the common properties and a way to destinguish the following context types:
 * - `struct sl_meta_pattern` if `type` equals `SL_PATTERN_META`
 * - `struct sl_include_pattern` if `type` equals `SL_PATTERN_INCLUDE`
 * - `struct sl_match_pattern` if `type` equals `SL_PATTERN_MATCH`
 */
struct sl_pattern_common {
    short type;
    char pad[2];
    uint32_t refcount;
};

struct sl_meta_pattern {
    short type;
    uint32_t refcount;
    const char *meta_scope;
    const char *meta_content_scope;
    bool meta_include_prototype;
    bool clear_scopes;
    bool meta_prepend;
    bool meta_append;
};

struct sl_context_ref {
    short type;
    char reftype : 4;
    short refsz : 12;
    uint32_t refcount;
    const char *reference;
};

struct sl_nested_group {
    short type;
    uint32_t refcount;
    char flags;
    int count; // number of points in vector below
    struct sl_pattern_common **contexts;
};

struct sl_capture {
    int group;
    const char *scope;
};

struct sl_match_pattern;

enum sl_context_ref_type {
    SL_CTX_NONE = 0,            // value not defined
    SL_CTX_EXTERNAL_REF_BY_NAME,
    SL_CTX_EXTERNAL_PTR,
    SL_CTX_EXTERNAL_SCOPE,
    SL_CTX_EXTERNAL_SYNTAX_FILE,
    SL_CTX_INLINE,
    // new below:
    SL_CTX_REF_NAMED,
    SL_CTX_REF_BY_SCOPE,
    SL_CTX_REF_SYNTAX_FILE,
    SL_CTX_REF_INLINE,
    SL_CTX_REF_DIRECT,
};

struct sl_scope {
    union {
        struct {
            uint64_t a;
            uint64_t b;
        } u64;
        short atoms[8];
    };
};

struct scope_atom {
    uint8_t atomsz;
    bool inline_atom;   // 
    union {
        const char *atom_ptr;
#ifdef __PTR64__
        char atom_arr[8];
#elif __PTR32__
        char atom_arr[4];
#endif
    };
};

struct scope_repository {
    uint16_t atoms_cnt;
    uint16_t atoms_cap;
    struct scope_atom *atoms;
};

typedef struct sl_scope sl_scope_t;

int sl_scope_from_str(struct scope_repository *repo, const char *str, sl_scope_t *scope);
int sl_scope_from_strn(struct scope_repository *repo, const char *str, size_t strsz, sl_scope_t *scope);

struct scope_repository *sl_scope_repo_create(uint32_t capacity);
struct scope_repository *sl_scope_repo_create_common(uint32_t capacity);
void sl_scope_repo_destroy(struct scope_repository *repo);
int32_t sl_scope_strcpy(struct scope_repository *repo, sl_scope_t *scope, char *buf, size_t bufsz);
char *sl_scope_strdup(struct scope_repository *repo, sl_scope_t *scope);


struct sl_context {
    short type;
    bool meta_include_prototype;
    bool uses_backrefs;
    uint32_t refcount;
    short meta_scope_count;
    short meta_content_scope_count;
    uint32_t patterns_count;
    sl_scope_t **meta_scope;
    sl_scope_t **meta_content_scope;
    struct sl_context *prototype;
    struct sl_pattern_common **patterns;
};

union ctxref {
    struct {
        const char *name;
        short namesz;
    } named;
    struct {
        sl_scope_t scope;
        const char *sub_context;
        short sub_contextsz;
        bool with_escape;
    } by_scope;
    struct {
        const char *filename;
        const char *sub_context;
        short filenamesz;
        short sub_contextsz;
        bool with_escape;
    } file;
    struct {
        const char *value;
        short valuesz;
    } inline_ref;
    struct {
        struct sl_context *context;
    } direct_ref;
};

struct sl_context_reference {
    short type;
    short reftype;
    uint32_t refcount;
    union ctxref ctxref;
};

struct sl_context *sl_create_context(void);
struct sl_context_reference *sl_create_named_context_ref(const char *name);
struct sl_context_reference *sl_create_by_scope_context_ref(const char *scope, struct scope_repository *scope_repo, const char *sub_context, bool with_escape);
struct sl_context_reference *sl_create_file_context_ref(const char *filepath, const char *sub_context, bool with_escape);
struct sl_context_reference *sl_create_inline_context_ref(const char *value);
struct sl_context_reference *sl_create_direct_context_ref(struct sl_context *ctx);

// TODO: types
// - context reference by name
// - context reference by ptr
// - reference by scope
// - *.sublime-syntax
struct sl_match_pushset {
    char vtype;
    short flags;
    int array_count;
    union {
        struct sl_match_pattern **contexts;
        struct sl_pattern_common *ctx;
        struct sl_grammar_def *syntax;
        const char **names;
        const char *name;  // used by SL_CTX_EXTERNAL_SCOPE and SL_CTX_EXTERNAL_SYNTAX_FILE
    };
};

struct sl_include_pattern {
    short type;
    char reftype : 4;
    bool apply_prototype : 1;
    uint32_t refcount;
    union ctxref ctxref;
};

struct sl_match_pattern {
    short type;
    uint32_t refcount;
    union {
        const char *match;      // Regexp
        void *match_regexp;
    };
    const char *scope;
    struct sl_capture *captures; // pair of <int, string>
    struct sl_context_reference *with_prototype;
    int pop_count;
    short capture_count;
    short flags;
    bool apply_prototype;
    char opcode;        // enum SL_GRAMMAR_OPS
    union {
        struct sl_match_pushset op_push;
        // push might also be referencing an entire syntax set, might want to preload or simply JIT that syntax tree once it does matches.
        struct sl_match_pushset op_set;
        struct {
            const char *embed;      // context reference
            const char *embed_scope;
            union {
                const char *escape;     // RegExp
                void *escape_regexp;
            };
            struct sl_capture *escape_captures; // pair of <int, string>
            int escape_captures_count;
        } op_embed; // op_embed alone is the larget struct, but also the op which is less common..
                    // TODO: nest this inside a pointer and alloc only when needed.
        struct {
            // inline sl_match_pushset
            char vtype;
            short flags;
            int array_count;
            union {
                struct sl_match_pattern **contexts;
                struct sl_pattern_common *ctx;
                struct sl_grammar_def *syntax;
                const char **names;
                const char *name;  // used by SL_CTX_EXTERNAL_SCOPE and SL_CTX_EXTERNAL_SYNTAX_FILE
            };
            const char *branch_point;   // TODO: what does this do? is it a context ref?
        } op_branch;
        struct {
            char flags;
            union {
                struct sl_match_pattern *context;
                const char *name;
            };
        } op_fail;
    };
};

// TODO: consider to add `hash` and `namesz` to make lookup somewhat faster
struct sl_grammar_variable_pair {
    int hash;
    short namesz;
    bool expanded;
    const char *name;
    const char *value;
};

struct sl_grammar_ctx_map_pair {
    short namesz;                       // number of bytes in the string pointed to by name.
    short contextsz;                    // number of contexts in the context pointer vector
    const char *name;
    struct sl_match_pattern **context;
};

struct sl_grammar_def {
    int refcount;
    short flags;
    short namesz;                   // string-size of name  (used for faster compare)
    short scopesz;                  // string-size of scope
    short file_extensions_count;
    short variables_map_count;
    short context_map_count;
    int version;
    bool hidden;
    const char *filepath;                   // references the absolute file-system path from where the `*.sublime-syntax` file was loaded.
    const char *name;
    const char *scope;
    const char *first_line_match;
    const char **file_extensions;           // NULL terminated list.
    const char **hidden_file_extensions;    // NULL terminated list.
    const char **extends;                   // NULL terminated list.
    struct sl_grammar_variable_pair *variables_map;
    struct sl_grammar_ctx_map_pair *context_map;
};

struct sl_line {
    const char *line;
    uint32_t length;
};

struct sl_syntax_state {
    int captures_count;
    int prototypes_count;
    struct sl_pattern_common *context;
    struct sl_pattern_common **prototypes;
    struct sl_capture *captures;
};

struct sl_vector {
    int cap;
    int cnt;
    void **stack;
};

struct sl_syntax_ctx {
    int stack_cap;
    int stack_cnt;
    struct sl_syntax_state *stack;
    bool first_line;
};

// internal
struct sl_syntax_state *sl_syntax_state_current(struct sl_syntax_ctx *);
void sl_syntax_state_pop(struct sl_syntax_ctx *);
struct sl_syntax_state *sl_syntax_state_push(struct sl_syntax_ctx *);

// public API:

struct sl_grammar_def *sl_syntax_load_file(const char *filepath);
struct sl_grammar_def *sl_syntax_load_data(const char *buf, size_t bufsz, int *error);
void sl_syntax_unload(struct sl_grammar_def *);
void sl_syntax_compile(struct sl_grammar_def *syntax, struct sl_grammar_def *(*include)(const char *filepath));

#ifdef __cplusplus
}
#endif

#endif /* __GRAMMAR_SL_H_ */