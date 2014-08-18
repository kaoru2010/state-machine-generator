%start_symbol fsm
%include {
    #include <stdlib.h>
    #include <assert.h>
    #include "smc_compiler.h"
}

%extra_argument { void *context }
%token_type { token_t* }
%token_destructor { destroy_token($$); }

%type raw_code { void * }
%type entry { void * }
%type exit { void * }
%type word { void * }
%type state { void * }
%type next_state { void * }
%type states { void * }
%type transition { void * }
%type transitions { void * }
%type action { void * }
%type actions { void * }
%destructor raw_code { destroy_string($$); }
%destructor entry { destroy_actions($$); }
%destructor exit { destroy_actions($$); }
%destructor word { destroy_string($$); }
%destructor state { destroy_state($$); }
%destructor next_state { destroy_string($$); }
%destructor states { destroy_states($$); }
%destructor transition { destroy_transition($$); }
%destructor transitions { destroy_transitions($$); }
%destructor action { destroy_string($$); }
%destructor actions { destroy_actions($$); }

fsm ::= fsm_tokens.

fsm_tokens ::= fsm_token fsm_tokens.
fsm_tokens ::= .

fsm_token ::= raw_source.
fsm_token ::= start_state.
fsm_token ::= class_name.
fsm_token ::= fsmclass_name.
fsm_token ::= header_file.
fsm_token ::= include_file.
fsm_token ::= package_name.
fsm_token ::= import.
fsm_token ::= declare.
fsm_token ::= access.
fsm_token ::= map.
fsm_token ::= comment1.
fsm_token ::= comment2.

raw_source ::= RAW_SOURCE_BEGIN raw_source_body RAW_SOURCE_END.

raw_source_body ::= RAW_SOURCE raw_source_body.
raw_source_body ::= .

start_state ::= START word(A) DOUBLE_COLON word(B). { set_start(A, B); }

class_name ::= CLASS word.

fsmclass_name ::= FSMCLASS word(A). { set_fsmclass(A); }

header_file ::= HEADER raw_code_line.

include_file ::= INCLUDE raw_code_line.

package_name ::= PACKAGE word(A). { set_package_name(A); }

import ::= IMPORT raw_code_line.

declare ::= DECLARE raw_code_line.

access ::= ACCESS raw_code_line.

raw_code_line ::= RAW_CODE_LINE.

map ::= MAP word(A) MAP_BEGIN states(B) MAP_END. { define_map(A, B); }

states(X) ::= state(A) states(B). { X = add_state(A, B); }
states ::= .

state(X) ::= word(A) entry(B) exit(C) BLOCK_BEGIN transitions(D) BLOCK_END. { X = create_state(A, B, C, D); }

transitions(X) ::= transition(A) transitions(B). { X = add_transition(A, B); }
transitions ::= .

transition(X) ::= word(A) transition_args guard next_state(D) BLOCK_BEGIN actions(E) BLOCK_END. { X = create_transition(A, D, E); }

transition_args ::= PARENTHESIS_BEGIN parameters PARENTHESIS_END.
transition_args ::= .

parameters ::= parameter COMMA parameters.
parameters ::= .

parameter ::= word COLON raw_code.

guard ::= CONDITION_BEGIN raw_code CONDITION_END.
guard ::= .

next_state(X) ::= word(A). { X = A; A = NULL; }
next_state ::= NIL.

actions(X) ::= action(A) actions(B). { X = add_action(A, B); }
actions ::= .

action(X) ::= word(A) PARENTHESIS_BEGIN arguments PARENTHESIS_END SEMICOLON. { X = A; A = NULL; }

arguments ::= raw_code COMMA arguments.
arguments ::= raw_code.

entry(X) ::= ENTRY BLOCK_BEGIN actions(A) BLOCK_END. { X = A; A = NULL; }
entry ::= .

exit(X) ::= EXIT BLOCK_BEGIN actions(A) BLOCK_END. { X = A; A = NULL; }
exit ::= .

raw_code(X) ::= RAW_CODE(A) raw_code(B). { X = add_string(A, B); }
raw_code ::= .

word(X) ::= WORD(A). { X = create_string(A); }

comment1 ::= COMMENT1_BEGIN COMMENT1_BODY.

comment2 ::= COMMENT2_BEGIN comment2_body COMMENT2_END.
comment2_body ::= COMMENT2_CHUNK comment2_body.
comment2_body ::= .
