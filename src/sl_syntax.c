

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sl_syntax.h"
#include "common.h"


struct sl_context *
sl_create_context(void)
{
    struct sl_context *p;

    p = malloc(sizeof(struct sl_context));
    if (p == NULL)
        return NULL;

    memset(p, 0, sizeof(struct sl_context));



    return p;
}

struct sl_context_reference *
sl_create_named_context_ref(const char *name)
{
    struct sl_context_reference *p;

    p = malloc(sizeof(struct sl_context_reference));
    if (p == NULL)
        return NULL;

    memset(p, 0, sizeof(struct sl_context_reference));

    p->type = SL_CONTEXT_REF;
    p->reftype = SL_CTX_REF_NAMED;
    p->ctxref.named.name = name;
    p->ctxref.named.namesz = strlen(name);

    return p;
}

int
sl_scope_from_str(struct scope_repository *repo, const char *str, sl_scope_t *scope)
{
    return (0);
}

// setting up common scopes, these are parts of the standard set declared by
// TextMate, these are sorted by layers of depth
#define DECLARE_STD_SCOPE(str)      \
    {.atomsz = (sizeof(str) - 1), .inline_atom = 0, .atom_ptr = str}

struct scope_atom std_scopes[] = {
    // level 1
    DECLARE_STD_SCOPE("comment"),
    DECLARE_STD_SCOPE("constant"),
    DECLARE_STD_SCOPE("entity"),
    DECLARE_STD_SCOPE("invalid"),
    DECLARE_STD_SCOPE("keyword"),
    DECLARE_STD_SCOPE("markup"),
    DECLARE_STD_SCOPE("meta"),
    DECLARE_STD_SCOPE("storage"),
    DECLARE_STD_SCOPE("string"),
    DECLARE_STD_SCOPE("support"),
    DECLARE_STD_SCOPE("variable"),
    // level 2
    DECLARE_STD_SCOPE("line"),
    DECLARE_STD_SCOPE("block"),
    DECLARE_STD_SCOPE("numeric"),
    DECLARE_STD_SCOPE("character"),
    DECLARE_STD_SCOPE("language"),
    DECLARE_STD_SCOPE("other"),
    DECLARE_STD_SCOPE("name"),
    DECLARE_STD_SCOPE("illegal"),
    DECLARE_STD_SCOPE("deprecated"),
    DECLARE_STD_SCOPE("control"),
    DECLARE_STD_SCOPE("operator"),
    DECLARE_STD_SCOPE("underline"),
    DECLARE_STD_SCOPE("bold"),
    DECLARE_STD_SCOPE("heading"),
    DECLARE_STD_SCOPE("italic"),
    DECLARE_STD_SCOPE("list"),
    DECLARE_STD_SCOPE("quote"),
    DECLARE_STD_SCOPE("raw"),
    DECLARE_STD_SCOPE("type"),
    DECLARE_STD_SCOPE("modifier"),
    DECLARE_STD_SCOPE("quoted"),
    DECLARE_STD_SCOPE("unquoted"),
    DECLARE_STD_SCOPE("interpolated"),
    DECLARE_STD_SCOPE("regexp"),
    DECLARE_STD_SCOPE("function"),
    DECLARE_STD_SCOPE("class"),
    DECLARE_STD_SCOPE("parameter"),
    // level 3
    DECLARE_STD_SCOPE("double-slash"),
    DECLARE_STD_SCOPE("double-dash"),
    DECLARE_STD_SCOPE("number-sign"),
    DECLARE_STD_SCOPE("percentage"),
    DECLARE_STD_SCOPE("documentation"),
    DECLARE_STD_SCOPE("escape"),
    DECLARE_STD_SCOPE("tag"),
    DECLARE_STD_SCOPE("section"),
    DECLARE_STD_SCOPE("inherited-class"),
    DECLARE_STD_SCOPE("attribute-name"),
    DECLARE_STD_SCOPE("link"),
    DECLARE_STD_SCOPE("numbered"),
    DECLARE_STD_SCOPE("unnumbered"),
    DECLARE_STD_SCOPE("single"),
    DECLARE_STD_SCOPE("double"),
};


struct scope_repository *
sl_scope_repo_create(uint32_t capacity)
{
    struct scope_repository *p;
    struct scope_atom *vec;

    if (capacity >= UINT16_MAX) {
        capacity = UINT16_MAX;
    }

    p = malloc(sizeof(struct scope_repository));
    if (p == NULL)
        return NULL;

    vec = malloc(capacity * sizeof(struct scope_atom));
    if (vec == NULL) {
        free(p);
        return NULL;
    }

    p->atoms = vec;
    p->atoms_cap = capacity;
    p->atoms_cnt = 0;

    return p;
}

struct scope_repository *
sl_scope_repo_create_common(uint32_t capacity)
{
    struct scope_repository *p;
    struct scope_atom *vec, *src, *dst;
    uint32_t cnt;

    if (capacity < 64) {
        capacity = 64;
    } else if (capacity >= UINT16_MAX) {
        capacity = UINT16_MAX;
    }

    p = malloc(sizeof(struct scope_repository));
    if (p == NULL)
        return NULL;

    vec = malloc(capacity * sizeof(struct scope_atom));
    if (vec == NULL) {
        free(p);
        return NULL;
    }

    src = std_scopes;
    dst = vec;
    cnt = sizeof(std_scopes) / sizeof(struct scope_atom);
    for (int i = 0; i < cnt; i++) {
        dst->atomsz = src->atomsz;
        if (src->atomsz < __member_size(struct scope_atom, atom_ptr)) {
            dst->inline_atom = 1;
            memcpy(dst->atom_arr, src->atom_ptr, src->atomsz);
        } else {
            dst->inline_atom = -1;
            dst->atom_ptr = src->atom_ptr;
        }
    }

    p->atoms = vec;
    p->atoms_cap = capacity;
    p->atoms_cnt = cnt;

    return p;
}

// finds or insert the provided atom and returns the index of the atom.
int
sl_scope_repo_atom(struct scope_repository *repo, const char *atom, uint32_t atomsz)
{
    struct scope_atom *vec, *nvec, *ptr;
    const char *str2;
    uint32_t idx, cnt, cap;
    int32_t match;

    vec = repo->atoms;
    cnt = repo->atoms_cnt;
    match = -1;

    for (idx = 0; idx < cnt; idx++) {
        ptr = &vec[idx];
        if (ptr->atomsz != atomsz)
            continue;
        if (ptr->inline_atom == true) {
            str2 = ptr->atom_arr;
        } else {
            str2 = ptr->atom_ptr;
        }

        if (strncmp(atom, str2, atomsz) == 0) {
            match = idx;
            break;
        }
    }

    if (match != -1)
        return match;

    cap = repo->atoms_cap;

    if (cap == cnt) {
        // resize vector
        nvec = malloc((cap + 32) * sizeof(struct scope_atom));
        if (nvec == NULL)
            return -1;

        memcpy(nvec, vec, cap * sizeof(struct scope_atom));
        free(vec);
        cap = cap + 32;
        vec = nvec;
        repo->atoms = vec;
        repo->atoms_cap = cap;
    }

    match = cnt++;
    ptr = &vec[match];

    ptr->atomsz = atomsz;
    if (atomsz <= __member_size(struct scope_atom, atom_arr)) {
        memcpy(ptr->atom_arr, atom, atomsz);
        ptr->inline_atom = 1;
    } else {
        str2 = malloc(atomsz + 1);
        if (str2 == NULL)
            return -1;
        memcpy((void *)str2, atom, atomsz);
        *(((char *)(str2)) + atomsz) = '\0';
        ptr->inline_atom = 0;
        ptr->atom_ptr = str2;
    }

    repo->atoms_cnt = cnt;

    return match;
}

int
sl_scope_from_strn(struct scope_repository *repo, const char *str, size_t strsz, sl_scope_t *scope)
{
    uint16_t scope_stack[16];
    const char *nxtp, *strp, *strend, *atomp;
    uint16_t atomsz;
    int32_t atomidx, stackidx, stackmax;

    // initialization
    memset(scope_stack, 0, sizeof(scope_stack));
    strp = str;
    strend = str + strsz;
    stackidx = 0;
    stackmax = sizeof(scope_stack) / sizeof(uint16_t);
 
    while (strp < strend) {
        nxtp = strchr(strp, '.');

        if (stackidx >= stackmax) {
            printf("stack-overflow in %s\n", __func__);
            break;
        }

        if (nxtp == NULL) {
            atomsz = strend - strp;
            atomidx = sl_scope_repo_atom(repo, strp, atomsz);
            scope_stack[stackidx++] = atomidx;
            strp = strend;
        } else {
            atomsz = nxtp - strp;
            atomidx = sl_scope_repo_atom(repo, strp, atomsz);
            scope_stack[stackidx++] = atomidx;
            strp = nxtp + 1;
        }
    }

    return (0);
}

void
sl_scope_repo_destroy(struct scope_repository *repo)
{
    struct scope_atom *ptr, *end;
    uint32_t cnt;

    ptr = repo->atoms;
    end = ptr + repo->atoms_cnt;

    while (ptr < end) {
        // inline_atom ==  1 : the string is held within the bytes otherwise holding the pointer
        // inline_atom == -1 : the pointer is to a read-only section of memory (static declared) and should never be freed.
        // inline_atom ==  0 : the pointer is to a heap-allocated string and should be freed upon the repo being destroyed.
        if (ptr->inline_atom == 0) {
            free((char *)ptr->atom_ptr);
        }
        ptr++;
    }

    ptr = repo->atoms;
    repo->atoms = NULL;
    repo->atoms_cap = 0;
    repo->atoms_cnt = 0;

    free(ptr);
    free(repo);
}

/**
 * Convert the `scope` to string into the buffer given by `buf` which available size is given by the ´bufsz´ argument. 
 * @param repo The reposity which stores the scope parts.
 * @param scope The scope vector for which to compute the scope string.
 *  
 * @return The length of the actual string content, which might be larger than the value provided in `bufsz`. In case of failure a negative value is returned.
 */
int32_t
sl_scope_strcpy(struct scope_repository *repo, sl_scope_t *scope, char *buf, size_t bufsz)
{
    struct scope_atom *atom, *vec;
    char *bufp;
    const char *src;
    uint16_t indexes[16];
    uint16_t idx;
    uint32_t cnt, end, vecmax, atomsz, strsz;

    cnt = 0;
    strsz = 0;
    vecmax = repo->atoms_cnt;
    vec = repo->atoms;

    for (int i = 0; i < 8; i++) {
        idx = scope->atoms[i];
        if (idx == 0) {
            cnt = i;
            break;
        } else {
            idx = idx - 1;
            if (idx < 0 || idx >= vecmax) {
                printf("index %d is out of bounds..\n", idx);
                return -1;
            }
            atom = &vec[idx];
            strsz += atom->atomsz;
            indexes[i] = idx;
        }
    }

    if (cnt == 0) {
        return -1;
    } else if (cnt > 1) {
        strsz += (cnt - 1);
    }

    end = cnt - 1;

    if (strsz <= bufsz) {
        // copy string without the need to check for the end
        for (int i = 0; i < cnt; i++) {
            idx = indexes[i];
            atom = &vec[idx];
            atomsz = atom->atomsz;
            
            if (atom->inline_atom == 1) {
                src = atom->atom_arr;
            } else {
                src = atom->atom_ptr;
            }

            memcpy(bufp, src, atomsz);
            bufp += atomsz;
            if (i != end) {
                *(bufp) = '.';
                bufp++;
            }
        }

        if (strsz < bufsz) {
            *(bufp) = '\0'; // NULL terminate if possible
        }

    } else {
        char *bufend;
        bool stop;

        bufend = buf + bufsz;
        stop = false;

        for (int i = 0; i < cnt; i++) {
            idx = indexes[i];
            atom = &vec[idx];
            atomsz = atom->atomsz;
            
            if (atom->inline_atom == 1) {
                src = atom->atom_arr;
            } else {
                src = atom->atom_ptr;
            }

            if (bufp + atomsz > bufend) {
                atomsz = bufend - bufp;
                stop = true;
            } else if (bufp + atomsz == bufend) {
                stop = true;
            }

            memcpy(bufp, src, atomsz);
            bufp += atomsz;
            if (stop == false && i != end && bufp < bufend) {
                *(bufp) = '.';
                bufp++;
                if (bufp == bufend)
                    stop = true;
            }

            if (stop)
                break;
        }

        if (bufp < bufend) {
            *(bufp) = '\0'; // NULL terminate if possible
        }

    }

    return 
    ;
}

/**
 * Convert the `scope` argument into a allocated string. Caller is responsible for managing the memory of the returing pointer. 
 */
char *
sl_scope_strdup(struct scope_repository *repo, sl_scope_t *scope)
{
    struct scope_atom *atom, *vec;
    char *buf, *bufp;
    const char *src;
    uint16_t indexes[16];
    uint16_t idx;
    uint32_t cnt, end, vecmax, atomsz, strsz;

    cnt = 0;
    strsz = 0;
    vecmax = repo->atoms_cnt;
    vec = repo->atoms;

    for (int i = 0; i < 8; i++) {
        idx = scope->atoms[i];
        if (idx == 0) {
            cnt = i;
            break;
        } else {
            idx = idx - 1;
            if (idx < 0 || idx >= vecmax) {
                printf("index %d is out of bounds..\n", idx);
                return NULL;
            }
            atom = &vec[idx];
            strsz += atom->atomsz;
            indexes[i] = idx;
        }
    }

    if (cnt == 0) {
        return NULL;
    } else if (cnt > 1) {
        strsz += (cnt - 1);
    }

    buf = malloc(strsz + 1);
    bufp = buf;
    if (buf == NULL) {
        return NULL;
    }

    end = cnt - 1;

    for (int i = 0; i < cnt; i++) {
        idx = indexes[i];
        atom = &vec[idx];
        atomsz = atom->atomsz;
        
        if (atom->inline_atom == 1) {
            src = atom->atom_arr;
        } else {
            src = atom->atom_ptr;
        }

        memcpy(bufp, src, atomsz);
        bufp += atomsz;
        if (i != end) {
            *(bufp) = '.';
            bufp++;
        }
    }

    *(bufp) = '\0';


    return buf;
}

struct sl_context_reference *
sl_create_by_scope_context_ref(const char *scope, struct scope_repository *scope_repo, const char *sub_context, bool with_escape)
{
    struct sl_context_reference *p;

    p = malloc(sizeof(struct sl_context_reference));
    if (p == NULL)
        return NULL;

    memset(p, 0, sizeof(struct sl_context_reference));

    p->type = SL_CONTEXT_REF;
    p->reftype = SL_CTX_REF_BY_SCOPE;
    
    sl_scope_from_strn(scope_repo, scope, strlen(scope), &p->ctxref.by_scope.scope);
    
    if (sub_context != NULL) {
        p->ctxref.by_scope.sub_context = sub_context;
        p->ctxref.by_scope.sub_contextsz = strlen(sub_context);
    }
    p->ctxref.by_scope.with_escape = with_escape;

    return p;
}

struct sl_context_reference *
sl_create_file_context_ref(const char *filepath, const char *sub_context, bool with_escape)
{
    struct sl_context_reference *p;

    p = malloc(sizeof(struct sl_context_reference));
    if (p == NULL)
        return NULL;

    memset(p, 0, sizeof(struct sl_context_reference));

    p->type = SL_CONTEXT_REF;
    p->reftype = SL_CTX_REF_SYNTAX_FILE;
    if (filepath != NULL) {
        p->ctxref.file.filename = filepath;
        p->ctxref.file.filenamesz = strlen(filepath);
    }
    if (sub_context != NULL) {
        p->ctxref.file.sub_context = sub_context;
        p->ctxref.file.sub_contextsz = strlen(sub_context);
    }
    p->ctxref.file.with_escape = with_escape;

    return p;
}

struct sl_context_reference *
sl_create_inline_context_ref(const char *value)
{
    struct sl_context_reference *p;

    if (value == NULL)
        return NULL;

    p = malloc(sizeof(struct sl_context_reference));
    if (p == NULL)
        return NULL;

    memset(p, 0, sizeof(struct sl_context_reference));

    p->type = SL_CONTEXT_REF;
    p->reftype = SL_CTX_REF_BY_SCOPE;
    p->ctxref.inline_ref.value = value;
    p->ctxref.inline_ref.valuesz = strlen(value);

    return p;
}

struct sl_context_reference *
sl_create_direct_context_ref(struct sl_context *ctx)
{
    struct sl_context_reference *p;

    if (ctx == NULL)
        return NULL;

    p = malloc(sizeof(struct sl_context_reference));
    if (p == NULL)
        return NULL;

    memset(p, 0, sizeof(struct sl_context_reference));

    p->type = SL_CONTEXT_REF;
    p->reftype = SL_CTX_REF_DIRECT;
    p->ctxref.direct_ref.context = ctx;
    ctx->refcount++;

    return p;
}