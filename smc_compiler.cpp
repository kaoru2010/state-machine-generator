#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <algorithm>
#include "smc_compiler.h"
#include "gen.h"

using namespace std;

set<string> g_transition_set;
set<string> g_action_set;

string g_start_map, g_start_state;

void set_start(std::string const& map, std::string const& state) {
    g_start_map = map;
    g_start_state = state;
}

string g_fsmclass = "MyFsm";
void set_fsmclass(std::string const& fsmclass) {
    g_fsmclass = fsmclass;
}

string g_package_name = "MyNamespace";
void set_package_name(std::string const& package_name) {
    g_package_name = package_name;
}

string g_class_name = "MyAction";
void set_class_name(std::string const& class_name) {
    g_class_name = class_name;
}

vector<state_map_t> g_state_map_list;

void define_map(std::string const& word, state_list_t const& states) {
    g_state_map_list.emplace_back(word, states);
}

void define_transition(std::string const& transition)
{
    g_transition_set.insert(transition);
}

void define_action(std::string const& action)
{
    g_action_set.insert(action);
}

vector<string> g_import_list;
void add_import(std::string const& import)
{
    g_import_list.push_back(import);
}

vector<string> g_include_list;
void add_include(std::string const& include)
{
    g_include_list.push_back(include);
}

int main(int argc, const char *argv[])
{
    context_t context = {};

#ifdef DEBUG
    ParseTrace(stdout, "DEBUG ");
#endif

    void *parser = ParseAlloc( malloc );
    while (1) {
        token_t *token = new token_t();
        int token_id = get_token(&context, token);
        if ( !token_id) {
            delete token;
            break;
        }

        //printf("%4d %s\n", token_id, token.text);
        Parse(parser, token_id, token, &context);
    }
    Parse(parser, 0, NULL, &context);
    ParseFree(parser, free);

    if (argc == 2 && strcmp(argv[1], "--swift") == 0) {
        gen_swift(g_package_name, g_fsmclass, g_state_map_list, g_start_map, g_start_state, g_transition_set, g_class_name, g_action_set, g_include_list, g_import_list);
    }
    else {
        gen_javascript(g_package_name, g_fsmclass, g_state_map_list, g_start_map, g_start_state, g_transition_set, g_class_name);
    }

    return 0;
}
