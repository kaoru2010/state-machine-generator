#include <iostream>
#include <string>
#include "smc_compiler.h"
#include "gen.h"

using namespace std;

static void generate_state_map(state_map_t const& state_map);

void gen_swift(std::string const& package_name, std::string const& fsmclass, state_map_list_t const& state_map_list, std::string const& start_map, std::string const& start_state, transition_set_t const& transition_set)
{
    cout
        << "import Foundation\n"
        << "\n"
        << "class " << package_name << "_" << fsmclass << " {\n"
        << "\n"
        << "    protocol State {\n"
        << "    }\n"
        << "\n"
        << "    protocol StateMap {\n"
        << "    }\n"
        << "\n"
        << "    var currentState : State\n"
        << "    var previounsState : State\n"
        << "    public var debugMode : Boolean\n"
        << "    var ctxt : Action\n"
        << "\n"
        << "    public func init() {\n"
        << "        self.currentState = " << start_map << "." << start_state << "()\n"
        << "    }\n"
        << "\n"
        << "    public func enterStartState() {\n"
        << "        currentState.Entry(self)\n"
        << "    }\n"
        << "\n"
        << "    private func setState(state:State) {\n"
        << "        currentState = state;\n"
        << "        if (debugMode) {\n"
        << "            NSLog(\"ENTER STATE: %s\", state.getName())\n"
        << "        }\n"
        << "    }\n"
        ;

    for (auto&& state_map : state_map_list) {
        generate_state_map(state_map);
    }

    for (auto&& transition_name : transition_set) {
        cout << "\n";
        cout << "    public func " << transition_name << "() {\n";
        cout << "        currentState." << transition_name << "(self)\n";
        cout << "    }\n";
        cout << "\n";
    }

    cout << "\n";
    cout << "}\n";
}


static void generate_state_map(state_map_t const& state_map)
{

    string const& state_map_name = state_map.get_state_map_name();
    state_list_t const& state_list = state_map.get_state_list();

    // カスタムアクションの出力とステート遷移の出力
    auto generate_transition = [&](transition_t const& transition, int indent)
    {
        intented_out out(cout, indent);

        // 遷移先のステートがある場合には Exit() を出力する。
        if ( !transition.get_next_state().empty()) {
            out << "currentState.Exit();\n";
        }

        // 現在のステートをNULLにセットする。
        out << "previousState = currentState;\n";

        // カスタムアクションを出力する。
        if ( !transition.get_action_list().empty()) {
            out << "currentState = null;\n";
            out << "try {" << "\n";
            out << "    // Custom action" << "\n";
        }

        for (auto const& action : transition.get_action_list()) {
            out << "    ctxt." << action << "();\n";
        }

        // 次の遷移先への移動をfinally区の中に入れるかどうか
        int next_state_indent = 0;

        if ( !transition.get_action_list().empty()) {
            out << "} finally {" << "\n";
            next_state_indent++;
        }

        // 遷移先のステートがある場合には Entry() を出力する。
        if (transition.get_next_state().empty()) {
             out(next_state_indent) << "currentState = previousState;\n";
        }
        else {
             out(next_state_indent) << "setState(" << state_map_name << "." << transition.get_next_state() << ");\n";
             out(next_state_indent) << "currentState.Entry();\n";
        }

        if ( !transition.get_action_list().empty()) {
            out << "}" << "\n";
        }
    };

    intented_out out(cout, 1);
    bool need_comma = false;
    out << "public class " << state_map_name << " : State {\n";
    for (auto const& state : state_list) {
        if (need_comma) cout << "\n";

        cout << "\n";
        out(1) << "public class " << state.get_state_name() << " : State {\n";
        out(2) << "public func getName() { return \"" << state.get_state_name() << "\"; }\n\n";
        out(2) << "public func Entry() {\n";

        for (auto&& action : state.get_entry()) {
             out(3) << "ctxt." << action << "();\n";
        }
        out(2) << "}\n";
        out << "\n";
        out(2) << "public func Exit() {\n";
             ;
        for (auto const& action : state.get_exit()) {
             out(3) << "ctxt." << action << "();\n";
        }
        out(2) << "}\n";

        for (auto const& transition_name_list_pair : state.get_transitions()) {
            string const& transition_name = transition_name_list_pair.first;

            cout << "\n";
            out(2) << "public func " << transition_name << "() {\n";

            bool else_block = false;
            for (auto&& transition : transition_name_list_pair.second) {
                if ( !transition.get_guard().empty()) {
                    out(3) << (else_block ? "else if" : "if") << " (" << transition.get_guard() << ") {\n";

                    generate_transition(transition, 5);

                    out(3) << "}\n";
                    else_block = true;
                }
                else {
                    if (else_block) {
                        out(3) << "else {\n";
                    }

                    generate_transition(transition, else_block ? 5 : 4);

                    if (else_block) {
                        out(3) << "}\n";
                    }
                }
            }

            out(2) << "}\n";
        }

        cout << "\n";
        out(1) << "}\n"
             ;
        need_comma = true;
    }
    cout << "\n"
         << "    " << "};" << endl;

}
