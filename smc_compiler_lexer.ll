%option noyywrap nounput noinput nodefault
%option stack
%x raw_source
%x expect_word raw_code_line
%x in_map map_state_def map_raw_code map_arguments map_transition map_actions map_action_argument map_argument_type
%x comment1
%x comment2

%{
#include <stdio.h>
#include "smc_compiler_parser.h"
#include "smc_compiler.h"
%}

WORD    [A-Za-z][A-Za-z0-9_.]*|_[A-Za-z][A-Za-z0-9_.]*

%%

"%{" BEGIN(raw_source); return RAW_SOURCE_BEGIN;
"%start" BEGIN(expect_word); return START;
"%class" BEGIN(expect_word); return CLASS;
"%fsmclass" BEGIN(expect_word); return FSMCLASS;
"%header" BEGIN(raw_code_line); return HEADER;
"%include" BEGIN(raw_code_line); return INCLUDE;
"%package" BEGIN(expect_word); return PACKAGE;
"%import" BEGIN(raw_code_line); return IMPORT;
"%declare" BEGIN(raw_code_line); return DECLARE;
"%access" BEGIN(raw_code_line); return ACCESS;
"%map" BEGIN(in_map); return MAP;

. printf("unmatched: %s\n", yytext);
\n ;

<*>{
    "//" yy_push_state(comment1);
    "/*" yy_push_state(comment2);
}

<raw_source>{
    "%}" BEGIN(INITIAL); return RAW_SOURCE_END;
    "%" return RAW_SOURCE;
    [^%]+ return RAW_SOURCE;
}

<expect_word>{
    [\t ]+ ;
    "::" return DOUBLE_COLON;
    {WORD} return WORD;
    . printf("unmatched: %s\n", yytext);
    \n BEGIN(INITIAL);
}

<raw_code_line>{
    [\t ]+ ;
    .+ return RAW_CODE_LINE;
    \n BEGIN(INITIAL);
}

<in_map>{
    [\n\t ]+ ;
    {WORD} return WORD;
    "%%" BEGIN(map_state_def); return MAP_BEGIN;
    . printf("unmatched: %s\n", yytext);
}

<map_state_def>{
    [ \t\n] ;
    "%%" BEGIN(INITIAL); return MAP_END;
    Entry yy_push_state(map_actions); return ENTRY;
    Exit yy_push_state(map_actions); return EXIT;
    "{" BEGIN(map_transition); return BLOCK_BEGIN;
    {WORD} return WORD;
    . printf("unmatched: %s\n", yytext);
}

<map_transition>{
    "}" BEGIN(map_state_def); return BLOCK_END;
    "{" yy_push_state(map_actions); return BLOCK_BEGIN;
    "[" yy_push_state(map_raw_code); return CONDITION_BEGIN;
    "]" return CONDITION_END;
    "(" yy_push_state(map_arguments); return PARENTHESIS_BEGIN;
    ")" return PARENTHESIS_END;
    ":" return COLON;
    ";" return SEMICOLON;
    "," return COMMA;
    nil return NIL;
    {WORD} return WORD;
    [\n\t ] ;
    . printf("unmatched: %s\n", yytext);
}

<map_raw_code>{
    [ \t\n] ;
    [^\]{}]+ return RAW_CODE;
    "]" yy_pop_state(); return CONDITION_END;
    "{" return BLOCK_BEGIN;
    "}" yy_pop_state(); return BLOCK_END;
}

<map_actions>{
    [ \t\n] ;
    "{" return BLOCK_BEGIN;
    ";" return SEMICOLON;
    {WORD} return WORD;
    "(" yy_push_state(map_action_argument); return PARENTHESIS_BEGIN;
    "}" yy_pop_state(); return BLOCK_END;
    . printf("unmatched: %s\n", yytext);
}

<map_action_argument>{
    [ \t\n]+ ;
    {WORD} return WORD;
    "," return COMMA;
    ")" yy_pop_state(); return PARENTHESIS_END;
    . printf("unmatched: %s\n", yytext);
}

<map_arguments>{
    [ \t\n]+ ;
    ":" yy_push_state(map_argument_type); return COLON;
    "," return COMMA;
    ")" yy_pop_state(); return PARENTHESIS_END;
    {WORD} return WORD;
    . printf("unmatched: %s\n", yytext);
}

<map_argument_type>{
    [^,)]+ yy_pop_state(); return RAW_CODE;
    \n ;
    ","|")" printf("missing argument type: %s\n", yytext);
}

<comment1>{
    .* ;
    \n yy_pop_state();
}

<comment2>{
    "*/" yy_pop_state();
    [^*]+ ;
    "*" ;
}

%%

int get_token(context_t *context, token_t *token)
{
    int token_id = yylex();
    token->text.reset(new std::string(yytext));
    return token_id;
}
