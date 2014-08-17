#pragma once

#include <stdlib.h>
#ifndef NDEBUG
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct context_tag {
} context_t;

typedef struct token_tag {
    char *text;
    int size;
} token_t;

int get_token(context_t *context, token_t *token);

void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseFree(void *parser,                    /* The parser to be deleted */
               void (*freeProc)(void*)     /* Function used to reclaim memory */
);
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  token_t *yyminor,            /* The value for the token */
  context_t *context           /* Optional %extra_argument parameter */
);

#ifndef NDEBUG
void ParseTrace(FILE *stream, char *zPrefix);
#endif

#ifdef __cplusplus
}
#endif
