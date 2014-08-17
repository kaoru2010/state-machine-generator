#include <stdlib.h>
#include <stdio.h>
#include "smc_compiler.h"

using namespace std;

int main()
{
    context_t context = {};
    token_t token = {};

    ParseTrace(stdout, "DEBUG ");

    void *parser = ParseAlloc( malloc );
    while (int token_id = get_token(&context, &token)) {
        printf("%4d %s\n", token_id, token.text);
        Parse(parser, token_id, &token, &context);
    }
    Parse(parser, 0, &token, &context);
    ParseFree(parser, free);

    return 0;
}
