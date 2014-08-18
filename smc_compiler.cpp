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

extern "C" {

token_t *dup_token(token_t *token) {
    token_t *new_token = (token_t *)malloc(sizeof(token_t));
    new_token->text = token->text ? strdup(token->text) : NULL;
    new_token->size = token->size;
    return new_token;
}

void destroy_token(token_t *token) {
    free(token->text);
    token->text = NULL;
    token->size = 0;
}

char *get_token_text(token_t *token) {
    return token->text;
}

void *add_string(token_t *token, void *raw_code) {
    if ( !token->text) return raw_code;

    if (!raw_code) {
        raw_code = new string(token->text);
    }
    else {
        static_cast<string*>(raw_code)->append(token->text);
    }

    return raw_code;
}

void *create_string(token_t *token) {
    if ( !token->text) return NULL;
    return new string(token->text);
}

void destroy_string(void *str) {
    delete static_cast<string*>(str);
}

typedef vector<string> action_list_t;

class transition_t {
    string transition_name_, guard_, next_state_;
    action_list_t action_list_;

public:
    transition_t(string *transition_name, string *guard, string *next_state, action_list_t *action_list)
    :   transition_name_(transition_name ? *transition_name : "")
    ,   guard_(guard ? *guard : "")
    ,   next_state_(next_state ? *next_state : "")
    ,   action_list_(action_list ? *action_list : action_list_t())
    {
    }

    string get_transition_name() const { return transition_name_; }
    string get_guard() const { return guard_; }
    string get_next_state() const { return next_state_; }
    action_list_t get_action_list() const { return action_list_; }
};

typedef map<string, deque<transition_t>> transition_list_t;

set<string> g_transition_set;

void *create_transition(void *transition_name, void *guard, void *next_state, void *actions) {
    g_transition_set.insert(*static_cast<string*>(transition_name));
    return new transition_t(
        static_cast<string*>(transition_name),
        static_cast<string*>(guard),
        static_cast<string*>(next_state),
        static_cast<action_list_t*>(actions)
    );
}

void destroy_transition(void *transition) {
    delete static_cast<transition_t*>(transition);
}

void *add_transition(void *transition0, void *transitions) {
    if ( !transition0) return transitions;

    if ( !transitions) {
        transitions = new transition_list_t();
    }

    transition_t *transition = static_cast<transition_t*>(transition0);
    transition_list_t& transitions_ref = *static_cast<transition_list_t*>(transitions);
    transitions_ref[transition->get_transition_name()].push_front(*transition);
    return transitions;
}

void destroy_transitions(void *transitions) {
    delete static_cast<transition_list_t*>(transitions);
}

void *add_action(void *action0, void *actions0) {
    action_list_t *actions = static_cast<action_list_t*>(actions0);
    string *action = static_cast<string*>(action0);

    if ( !action) return actions;

    if ( !actions) {
        actions = new action_list_t();
    }
    actions->push_back(*action);
    return actions;
}

void destroy_actions(void *actions) {
    delete static_cast<action_list_t*>(actions);
}

class state {
    string state_name_;
    action_list_t entry_, exit_;
    transition_list_t transitions_;

public:
    state(string *state_name, action_list_t *entry, action_list_t *exit, transition_list_t *transitions)
    :   state_name_(*state_name)
    ,   entry_(entry ? *entry : action_list_t())
    ,   exit_(exit ? *exit : action_list_t())
    ,   transitions_(transitions ? *transitions : transition_list_t())
    {
    }

    string get_state_name() const { return state_name_; }
    action_list_t get_entry() const { return entry_; }
    action_list_t get_exit() const { return exit_; }
    transition_list_t get_transitions() const { return transitions_; }
};

void *create_state(void *state_name, void *entry, void *exit, void *transitions) {
    return new state(
        static_cast<string*>(state_name),
        static_cast<action_list_t*>(entry),
        static_cast<action_list_t*>(exit),
        static_cast<transition_list_t*>(transitions)
    );
}

void destroy_state(void *state1) {
    delete static_cast<state*>(state1);
}

typedef vector<state> state_list_t;

void *add_state(void *state1, void *states) {
    if ( !state1) return states;

    if ( !states) {
        states = new state_list_t();
    }

    static_cast<state_list_t*>(states)->push_back(
        *static_cast<state*>(state1)
    );

    return states;
}

void destroy_states(void *states) {
    delete static_cast<state_list_t *>(states);
}

string g_start_map, g_start_state;

void set_start(void *map, void *state) {
    g_start_map = *static_cast<string*>(map);
    g_start_state = *static_cast<string*>(state);
}

string g_fsmclass = "MyFsm";
void set_fsmclass(void *fsmclass) {
    g_fsmclass = *static_cast<string*>(fsmclass);
}

string g_package_name = "MyNamespace";
void set_package_name(void *package_name) {
    g_package_name = *static_cast<string*>(package_name);
}


void define_map(void *word0, void *states0) {
    string *word = static_cast<string*>(word0);
    state_list_t *list = static_cast<state_list_t *>(states0);

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
             indented_out(next_state_indent) << "setState(" << *word << "." << transition.get_next_state() << ");\n";
             indented_out(next_state_indent) << "currentState.Entry();\n";
        }

        if ( !transition.get_action_list().empty()) {
            indented_out(indent) << "}" << "\n";
        }
    };

    bool need_comma = false;
    cout << "    " << "var " << *word << " = {\n";
    for (auto const& state : *list) {
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

} // extern "C" {

int main()
{
    context_t context = {};
    token_t token = {};

    //ParseTrace(stdout, "DEBUG ");

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

    void *parser = ParseAlloc( malloc );
    while (int token_id = get_token(&context, &token)) {
        //printf("%4d %s\n", token_id, token.text);
        Parse(parser, token_id, dup_token(&token), &context);
    }
    Parse(parser, 0, &token, &context);
    ParseFree(parser, free);

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
