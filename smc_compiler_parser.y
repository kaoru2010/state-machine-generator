%start_symbol fsm
%include {
    #include <stdlib.h>
    #include <assert.h>
    #include "smc_compiler.h"
}

%extra_argument { void *context }
%token_type { token_t* }
%token_destructor { delete $$; }

%type guard { std::string * }
%type raw_code { std::string * }
%type raw_code_line { std::string * }
%type entry { action_list_t * }
%type exit { action_list_t * }
%type word { std::string * }
%type state { state * }
%type next_state { std::string * }
%type states { state_list_t * }
%type transition { transition_t * }
%type transitions { transition_list_t * }
%type action { action_t * }
%type actions { action_list_t * }
%type arguments { std::string * }
%destructor guard { delete $$; }
%destructor raw_code { delete $$; }
%destructor raw_code_line { delete $$; }
%destructor entry { delete $$; }
%destructor exit { delete $$; }
%destructor word { delete $$; }
%destructor state { delete $$; }
%destructor next_state { delete $$; }
%destructor states { delete $$; }
%destructor transition { delete $$; }
%destructor transitions { delete $$; }
%destructor action { delete $$; }
%destructor actions { delete $$; }
%destructor arguments { delete $$; }

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

start_state ::= START word(A) DOUBLE_COLON word(B). { set_start(*A, *B); }

class_name ::= CLASS word(A). { set_class_name(*A); }

fsmclass_name ::= FSMCLASS word(A). { set_fsmclass(*A); }

header_file ::= HEADER raw_code_line.

include_file ::= INCLUDE raw_code_line(A). { add_include(*A); }

package_name ::= PACKAGE word(A). { set_package_name(*A); }

import ::= IMPORT raw_code_line(A). { add_import(*A); }

declare ::= DECLARE raw_code_line.

access ::= ACCESS raw_code_line.

raw_code_line(X) ::= RAW_CODE_LINE(A). { X = new std::string(*(A->text)); }

map ::= MAP word(A) MAP_BEGIN states(B) MAP_END. { define_map(*A, *B); }

states(X) ::= state(A) states(B). { if (B) { X = B; B = NULL; } else { X = new state_list_t(); } X->push_back(*A); }
states ::= .

state(X) ::= word(A) entry(B) exit(C) BLOCK_BEGIN transitions(D) BLOCK_END. { X = new state(A, B, C, D); }

transitions(X) ::= transition(A) transitions(B). { if (B) { X = B; B = NULL; } else { X = new transition_list_t(); } (*X)[A->get_transition_name()].push_front(*A); }
transitions ::= .

transition(X) ::= word(A) transition_args guard(C) next_state(D) BLOCK_BEGIN actions(E) BLOCK_END. { X = new transition_t(A, C, D, E); define_transition(*A); }

transition_args ::= PARENTHESIS_BEGIN parameters PARENTHESIS_END.
transition_args ::= PARENTHESIS_BEGIN PARENTHESIS_END.
transition_args ::= .

parameters ::= parameter COMMA parameters.
parameters ::= parameter.

parameter ::= word COLON raw_code.

guard(X) ::= CONDITION_BEGIN raw_code(A) CONDITION_END. { X = A; A = NULL; }
guard ::= .

next_state(X) ::= word(A). { X = A; A = NULL; }
next_state ::= NIL.

actions(X) ::= action(A) actions(B). { if (B) { X = B; B = NULL; } else { X = new action_list_t(); } X->push_back(*A); }
actions ::= .

action(X) ::= word(A) PARENTHESIS_BEGIN arguments(B) PARENTHESIS_END SEMICOLON. { define_action(*A, B ? *B : ""); X = new action_t(*A, B ? *B : ""); }

//arguments ::= raw_code COMMA arguments.
arguments(X) ::= raw_code(A). { X = A; A = NULL; }

entry(X) ::= ENTRY BLOCK_BEGIN actions(A) BLOCK_END. { X = A; A = NULL; }
entry ::= .

exit(X) ::= EXIT BLOCK_BEGIN actions(A) BLOCK_END. { X = A; A = NULL; }
exit ::= .

raw_code(X) ::= RAW_CODE(A) raw_code(B). { if (B) { X = B; B = NULL; } else { X = new std::string(); } *X += *(A->text); }
raw_code ::= .

word(X) ::= WORD(A). { X = new std::string(*(A->text)); }

comment1 ::= COMMENT1_BEGIN COMMENT1_BODY.

comment2 ::= COMMENT2_BEGIN comment2_body COMMENT2_END.
comment2_body ::= COMMENT2_CHUNK comment2_body.
comment2_body ::= .
