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
map<string, string> g_action_map;
map<string, parameter_list_t> g_transition_params;

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

void define_transition(std::string const& transition, parameter_list_t *parameter_list)
{
    g_transition_set.insert(transition);
    if (parameter_list) {
        g_transition_params[transition] = *parameter_list;
    }
}

void define_action(std::string const& action, std::string const& arguments)
{
    g_action_map[action] = action + "(" + arguments + ")";
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
        gen_swift(g_package_name, g_fsmclass, g_state_map_list, g_start_map, g_start_state, g_transition_set, g_class_name, g_action_map, g_include_list, g_import_list, g_transition_params);
    }
    else {
        gen_javascript(g_package_name, g_fsmclass, g_state_map_list, g_start_map, g_start_state, g_transition_set, g_class_name);
    }

    return 0;
}
