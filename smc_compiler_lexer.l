%option noyywrap nounput noinput nodefault
%option stack
%x raw_source
%x expect_word raw_code_line
%x in_map map_state_def map_raw_code map_arguments
%x comment1
%x comment2

%{
#include <stdio.h>
#include "smc_compiler_parser.h"
%}

WORD    [A-Za-z][A-Za-z0-9_.]*|_[A-Za-z][A-Za-z0-9_.]*

%%

"%{" BEGIN(raw_source); return RAW_SOURCE_BEGIN;
"%start" BEGIN(expect_word); return START;
"%class" BEGIN(expect_word); return CLASS;
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
    "%%" BEGIN(INITIAL); return MAP_END;
    "{" return BLOCK_BEGIN;
    "}" return BLOCK_END;
    Entry BEGIN(map_raw_code); return ENTRY;
    Exit BEGIN(map_raw_code); return EXIT;
    "[" BEGIN(map_raw_code); return CONDITION_BEGIN;
    "]" return CONDITION_END;
    "(" BEGIN(map_arguments); return PARENTHESIS_BEGIN;
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
    [^\]{}]+ return RAW_CODE;
    "]" BEGIN(map_state_def); return CONDITION_END;
    "{" return BLOCK_BEGIN;
    "}" BEGIN(map_state_def); return BLOCK_END;
}

<map_arguments>{
    [ \t]+ ;
    ":" return COLON;
    "," return COMMA;
    ")" BEGIN(map_state_def); return PARENTHESIS_END;
    {WORD} return WORD;
    [^,)] return RAW_CODE;
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

int main()
{
    while (1) {
        int token_id = yylex();
        if ( !token_id) break;

        printf("token_id=%d\n", token_id);
    }
    return 0;
}
