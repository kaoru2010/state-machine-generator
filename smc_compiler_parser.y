%start_symbol fsm
%include {
    #include <stdlib.h>
    #include <assert.h>
}

fsm ::= fsm_tokens.

fsm_tokens ::= fsm_token fsm_tokens.

fsm_token ::= raw_source.
fsm_token ::= start_state.
fsm_token ::= class_name.
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

start_state ::= START word DOUBLE_COLON word.

class_name ::= CLASS word.

header_file ::= HEADER raw_code_line.

include_file ::= INCLUDE raw_code_line.

package_name ::= PACKAGE word.

import ::= IMPORT raw_code_line.

declare ::= DECLARE raw_code_line.

access ::= ACCESS raw_code_line.

raw_code_line ::= RAW_CODE_LINE.

map ::= MAP word MAP_BEGIN states MAP_END.

states ::= word entry exit BLOCK_BEGIN transitions BLOCK_END.

transitions ::= word transition_args guard next_state BLOCK_BEGIN actions BLOCK_END.

transition_args ::= PARENTHESIS_BEGIN parameters PARENTHESIS_END.
transition_args ::= .

parameters ::= parameter COMMA parameters.
parameters ::= .

parameter ::= word COLON raw_code.

guard ::= CONDITION_BEGIN raw_code CONDITION_END.
guard ::= .

next_state ::= word.
next_state ::= NIL.

actions ::= action actions.
actions ::= .

action ::= word PARENTHESIS_BEGIN arguments PARENTHESIS_END SEMICOLON.

arguments ::= raw_code.
arguments ::= raw_code COMMA arguments.

entry ::= ENTRY.
entry ::= .

exit ::= EXIT.
exit ::= .

raw_code ::= raw_code_chunk raw_code.
raw_code ::= .
raw_code_chunk ::= RAW_CODE.

word ::= WORD.

comment1 ::= COMMENT1_BEGIN COMMENT1_BODY.

comment2 ::= COMMENT2_BEGIN comment2_body COMMENT2_END.
comment2_body ::= COMMENT2_CHUNK comment2_body.
comment2_body ::= .
