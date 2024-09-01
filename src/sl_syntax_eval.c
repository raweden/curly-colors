#include <stdint.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <yaml.h>
#include <oniguruma.h>
//#include <yaml-cpp/yaml.h>

#include "sl_syntax.h"


// parsing syntax highlighting

struct sl_regexp_match {
    OnigRegion *regions;
    void *context;
    struct sl_match_pattern *pat;
    bool from_with_prototype;
    bool would_loop;
};

struct sl_syntax_state *
sl_syntax_state_current(struct sl_syntax_ctx *ctx)
{
    if (ctx->stack_cnt == 0) {
        return NULL;
    }

    return ctx->stack + (ctx->stack_cnt - 1);

}

void
sl_syntax_state_pop(struct sl_syntax_ctx *ctx)
{
    if (ctx->stack_cnt > 0) {
        ctx->stack_cnt--;
    }
}

struct sl_syntax_state *
sl_syntax_state_push(struct sl_syntax_ctx *ctx)
{
    struct sl_syntax_state *ptr;
    uint32_t ocap, ncap;

    if (ctx->stack_cnt == ctx->stack_cap) {
        ocap = ctx->stack_cap;
        ncap = ocap + 16;
        ptr = (struct sl_syntax_state *)malloc(ncap + sizeof(struct sl_syntax_state));
        if (ctx->stack != NULL) {
            memcpy(ptr, ctx->stack, ocap + sizeof(struct sl_syntax_state));
            free(ctx->stack);
        }
        memset(ptr + ocap, 0, (ncap - ocap) * sizeof(struct sl_syntax_state));
        ctx->stack = ptr;
    }

    ptr = ctx->stack + ctx->stack_cnt;
    ctx->stack_cnt++;
    return ptr;
}

static int
sl_syntax_perform_op(struct sl_syntax_ctx *ctx, const char *line, OnigRegion *region, struct sl_match_pattern *pat, struct sl_grammar_def *syntax)
{
    if (pat->opcode == 0) {
        return false;
    }

    if (pat->pop_count > 0) {

    }

    return (0);
}

static int
sl_syntax_exec_pattern(struct sl_syntax_ctx *ctx, const char *line, struct sl_grammar_def *syntax, int *start, OnigRegion *region, bool check_pop_loop)
{
    
    return (0);
}

static int
sl_syntax_search(struct sl_line *line, int start, struct sl_match_pattern *pat, struct sl_capture *captures, uint32_t capture_count, OnigRegion *region)
{
    const char *line_start;
    const char *line_end;
    OnigOptionType flags;
    int ret;

    line_start = line->line;
    line_end = line_start + line->length;
    flags = ONIG_OPTION_NONE;

    ret = onig_search((OnigRegex)pat->match_regexp, (const OnigUChar *)line_start, (const OnigUChar *)line_end, (const OnigUChar *)(line_start + start), (const OnigUChar *)line_end, region, flags);

    if (ret != ONIG_MISMATCH) {
        int match_start = region->beg[0];
        int match_end = region->end[0];
    }

    return (0);
}

static int
sl_syntax_find_best_match(struct sl_syntax_ctx *ctx, struct sl_line *line, struct sl_grammar_def *syntax, int start, OnigRegion *region, struct sl_regexp_match *match, bool check_pop_loop)
{
    int cur_level;
    int context;
    int prototype;
    int min_start;
    void *best_match;
    int pop_would_loop;
    void *context_chain;


    min_start = INT32_MAX;
    best_match = NULL;
    pop_would_loop = false;

    while (false) {
        struct sl_match_pattern *match_pat;
        int ret, match_start, match_end;

        ret = sl_syntax_search(line, start, match_pat, match_pat->captures, match_pat->capture_count, region);
        
        if (match_start < min_start || (match_start == min_start && pop_would_loop)) {

            min_start = match_start;

            bool consuming = (match_end > start);
            //pop_would_loop = check_pop_loop &&  !consuming && match_pat->opcode == GRAMMAR_OP_POP;
        }
    }

    
    return (0);
}

static int
sl_syntax_parse_next_token(struct sl_syntax_ctx *ctx, struct sl_line *line, struct sl_grammar_def *syntax, int *start, OnigRegion *region, int non_consuming_push_at[2], struct sl_vector *ops)
{
    struct sl_regexp_match match;
    int ret;
    bool check_pop_loop;

    check_pop_loop = non_consuming_push_at[0] == *start && non_consuming_push_at[1] == ctx->stack_cnt;

    memset(&match, 0, sizeof(struct sl_regexp_match));

    ret = sl_syntax_find_best_match(ctx, line, syntax, *start, region, &match, check_pop_loop);

    return (0);
}

static int
sl_syntax_parse_line(struct sl_syntax_ctx *ctx, struct sl_line *line, struct sl_grammar_def *syntax)
{
    int ret, match_start = 0;
    int non_consuming_push_at[2];

    if (ctx->first_line) {
        struct sl_syntax_state *cur_level;
        struct sl_pattern_common *context;
        struct sl_meta_pattern *meta_pat;
        cur_level = sl_syntax_state_current(ctx);
        context = cur_level->context;
        if (context->type == SL_PATTERN_META && ((struct sl_meta_pattern *)(context))->meta_content_scope != NULL) {
            meta_pat = (struct sl_meta_pattern *)context;
            // res.push((0, ScopeStackOp::Push(context.meta_content_scope[0])));
        }

        ctx->first_line = false;
    }

    OnigRegion *region = onig_region_new();
    non_consuming_push_at[0] = 0;
    non_consuming_push_at[1] = 1;

    while (ret != 0) {
        ret = sl_syntax_parse_next_token(ctx, line, syntax, &match_start, region, non_consuming_push_at, NULL);
    }

    return (0);
}