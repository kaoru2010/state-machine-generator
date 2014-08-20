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

using namespace std;

set<string> g_transition_set;

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

vector<state_map_t> g_state_map_list;

void define_map(std::string const& word, state_list_t const& states) {
    g_state_map_list.emplace_back(word, states);
}

void generate_state_map(state_map_t const& state_map) {

    string const& state_map_name = state_map.get_state_map_name();
    state_list_t const& state_list = state_map.get_state_list();

    auto indented_out = [](int indent) -> ostream&
    {
        for (int i = 0; i < indent; ++i) {
            cout << "    ";
        }
        return cout;
    };

    // カスタムアクションの出力とステート遷移の出力
    auto generate_transition = [&](transition_t const& transition, int indent)
    {
        // 遷移先のステートがある場合には Exit() を出力する。
        if ( !transition.get_next_state().empty()) {
            indented_out(indent) << "currentState.Exit();\n";
        }

        // 現在のステートをNULLにセットする。
        indented_out(indent) << "previousState = currentState;\n";

        // カスタムアクションを出力する。
        if ( !transition.get_action_list().empty()) {
            indented_out(indent) << "currentState = null;\n";
            indented_out(indent) << "try {" << "\n";
            indented_out(indent) << "    // Custom action" << "\n";
        }

        for (auto const& action : transition.get_action_list()) {
            indented_out(indent) << "    ctxt." << action << "();\n";
        }

        // 次の遷移先への移動をfinally区の中に入れるかどうか
        int next_state_indent = indent;

        if ( !transition.get_action_list().empty()) {
            indented_out(indent) << "} finally {" << "\n";
            next_state_indent++;
        }

        // 遷移先のステートがある場合には Entry() を出力する。
        if (transition.get_next_state().empty()) {
             indented_out(next_state_indent) << "currentState = previousState;\n";
        }
        else {
             indented_out(next_state_indent) << "setState(" << state_map_name << "." << transition.get_next_state() << ");\n";
             indented_out(next_state_indent) << "currentState.Entry();\n";
        }

        if ( !transition.get_action_list().empty()) {
            indented_out(indent) << "}" << "\n";
        }
    };

    bool need_comma = false;
    cout << "    " << "var " << state_map_name << " = {\n";
    for (auto const& state : state_list) {
        if (need_comma) cout << ",\n";

        cout << "    " << "    " << state.get_state_name() << ": {\n"
             << "    " << "    " << "    " << "getName : function() { return \"" << state.get_state_name() << "\"; },\n"
             << "    " << "    " << "    " << "Entry : function() {\n"
             ;
        for (auto const& action : state.get_entry()) {
             cout << "    " << "    " << "    " << "    ctxt." << action << "();\n";
        }
        cout << "    " << "    " << "    " << "},\n"
             << "    " << "    " << "    " << "Exit : function() {\n"
             ;
        for (auto const& action : state.get_exit()) {
             cout << "    " << "    " << "    " << "    ctxt." << action << "();\n";
        }
        cout << "    " << "    " << "    " << "}"
             ;

        for (auto const& transition_name_list_pair : state.get_transitions()) {
            string const& transition_name = transition_name_list_pair.first;

            cout << ",\n"
                 << "    " << "    " << "    " << transition_name << ": function() {\n"
                 ;

            bool else_block = false;
            for (auto const& transition : transition_name_list_pair.second) {
                if ( !transition.get_guard().empty()) {
                    cout << "    " << "    " << "    " << "    " << (else_block ? "else if" : "if") << " (" << transition.get_guard() << ") {\n";

                    generate_transition(transition, 5);

                    cout << "    " << "    " << "    " << "    " << "}\n";
                    else_block = true;
                }
                else {
                    if (else_block) {
                        cout << "    " << "    " << "    " << "    " << "else {\n";
                    }

                    generate_transition(transition, else_block ? 5 : 4);

                    if (else_block) {
                        cout << "    " << "    " << "    " << "    " << "}\n";
                    }
                }
            }

            cout << "    " << "    " << "    " << "}"
                 ;
        }

        cout << "\n";
        cout << "    " << "    " << "}"
             ;
        need_comma = true;
    }
    cout << "\n"
         << "    " << "};" << endl;

}

int main()
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

    // https://github.com/stevekwan/experiments/blob/master/javascript/module-pattern.html
    cout << "// -*- tab-width: 4; -*-\n"
         << "// -*- coding: utf-8 -*-\n"
         << "\n"
         << "//\n"
         << "// This file is automatically generated by State Machine Generator.\n"
         << "//\n"
         << "\n"
         << "var " << g_package_name << " = " << g_package_name << " || {};\n"
         << "" << g_package_name << "." << g_fsmclass << " = function(ctxt) {\n"
         << "    var currentState, previousState, debugMode = false;\n"
         << "    var setState = function(state) { currentState = state; if (debugMode) { console.log('ENTER STATE: %s', state.getName()); } };\n"
         ;

    for (auto&& state_map : g_state_map_list) {
        generate_state_map(state_map);
    }

    cout << "    currentState = " << g_start_map << "." << g_start_state << ";\n"
         << "    return {\n"
         << "        enterStartState: function() { currentState.Entry(); },\n"
         << "        getPreviousState: function() { return previousState; },\n"
         << "        getState: function() { return currentState; },\n"
         << "        isDebugMode: function() { return debugMode; },\n"
         << "        setDebugMode: function(mode) { debugMode = mode; }"
         ;
    for (auto const& transition_name : g_transition_set) {
        cout << ",\n";
        cout << "        " << transition_name << ": function() {\n"
             << "            if (currentState." << transition_name << ") {\n"
             << "                currentState." << transition_name << "();\n"
             << "            } else if (" << g_start_map << ".Default." << transition_name << ") {\n"
             << "                " << g_start_map << ".Default." << transition_name << "();\n"
             << "            } else {\n"
             << "                " << g_start_map << ".Default.Default();\n"
             << "            }\n"
             << "        }"
             ;
    }
    cout << "\n";
    cout << "    };\n";
    cout << "};" << endl;

    return 0;
}
