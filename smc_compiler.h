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
  void *context                /* Optional %extra_argument parameter */
);

#ifndef NDEBUG
void ParseTrace(FILE *stream, char *zPrefix);
#endif

token_t *dup_token(token_t *token);
void destroy_token(token_t *token);
char *get_token_text(token_t *token);

void *create_string(token_t *token);
void destroy_string(void *str);
void *add_string(token_t *token, void *str);

void *create_state(void *state_name, void *entry, void *exit, void *transitions);
void destroy_state(void *state);

void *add_state(void *state, void *states);
void destroy_states(void *states);

void *add_action(void *action, void *actions);
void destroy_actions(void *actions);

void define_map(void *word, void *states);

void *create_transition(void *action_name, void *next_state, void *actions);
void destroy_transition(void *transition);

void *add_transition(void *transition, void *transitions);

void set_start(void *map, void *state);
void set_fsmclass(void *fsmclass);
void set_package_name(void *package_name);

#ifdef __cplusplus
}
#endif
