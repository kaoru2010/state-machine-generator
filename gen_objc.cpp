#include <iostream>
#include <string>
#include "smc_compiler.h"
#include "gen.h"

using namespace std;

static void generate_state_map(state_map_t const& state_map, std::string const& package_name, std::string const& fsmclass, std::string const& class_name, std::map<string, argument_list_t> const& action_map, transition_set_t const& transition_set);

void gen_objc(
    std::string const& package_name,
    std::string const& fsmclass,
    state_map_list_t const& state_map_list,
    std::string const& start_map,
    std::string const& start_state,
    transition_set_t const& transition_set,
    std::string const& class_name,
    std::map<std::string, argument_list_t> const& action_map,
    std::vector<std::string> const& include_list,
    std::vector<std::string> const& import_list)
{
    string full_fsm_name = package_name + "_" + fsmclass;

    cout << "// -*- tab-width: 4; -*-\n";
    cout << "// -*- coding: utf-8 -*-\n";
    cout << "\n";
    cout << "//\n";
    cout << "// This file is automatically generated by State Machine Generator.\n";
    cout << "//\n";
    cout << "\n";

    cout << "#import <Foundation/Foundation.h>\n";

    for (auto&& include : include_list) {
        cout << "// #include <" << include << ">\n";
    }

    for (auto&& import: import_list) {
        cout << "import \"" << import << "\"\n";
    }
    cout << "\n";

    cout << "@protocol " << package_name << "_Actions\n";
    for (auto const& action : action_map) {
        cout << "- (void)" << action.first << ":(" << full_fsm_name << " *)fsm";
        bool first = true;
        for (auto const& arg : action.second) {
            if (first) {
                first = false;
            }
            else {
                cout << " ";
            }
            cout << arg << ":" << arg;
        }
        cout << ";\n";
    }
    cout << "@end\n\n";

    cout
        << "\n"
        << "@interface " << full_fsm_name << " : NSObject\n"
        << "\n"
        << "@property (nonatomic) " << package_name << "_" << start_map << "_State *currentState;\n"
        << "@property (nonatomic) " << package_name << "_" << start_map << "_State *previousState;\n"
        << "@property (nonatomic) BOOL debugMode;\n"
        << "@property (nonatomic, waek) " << class_name << " *ctxt;\n"
        << "\n"
        << "- (id)init:(" << class_name << " *)context;\n"
        << "- (void)enterStartState;\n"
        ;

        for (auto&& transition_name : transition_set) {
            cout << "- (void)" << transition_name << ";\n";
        }

    cout
        << "@end\n"
        << "\n"
        << "\n"
        << "@implementation " << full_fsm_name << "\n"
        << "\n"
        << "- (id)init:(" << class_name << " *)context\n"
        << "{\n"
        << "    self = [super init];\n"
        << "    if (self) {\n"
        << "        self.currentState = " << package_name << "_" << start_map << "._" << start_state << ";\n"
        << "        self.ctxt = context;\n"
        << "    }\n"
        << "    return self;\n"
        << "}\n"
        << "\n"
        << "- (void)enterStartState\n"
        << "{\n"
        << "    [self.currentState fsm:self ctxt:self.ctxt Entry];\n"
        << "}\n"
        << "\n"
        << "- (void)setState:(" << package_name << "_" << start_map << "_State)state\n"
        << "{\n"
        << "    _currentState = state;\n"
        << "    if (self.debugMode) {\n"
        << "        NSLog(\"ENTER STATE: %s\", state.getName())\n"
        << "    }\n"
        << "}\n"
        ;

    for (auto&& transition_name : transition_set) {
        cout << "\n";
        cout << "- (void)" << transition_name << "\n";
        cout << "{\n";
        cout << "    NSAssert(self.currentState, @\"State-loss. Cannot send events in an action.\");\n";
        cout << "    [self.currentState fsm:self ctxt:self.ctxt " << transition_name << "];\n";
        cout << "}\n";
    }

    cout << "\n";
    cout << "@end\n";
    cout << "\n";
    cout << "\n";
    cout << "\n";

    for (auto&& state_map : state_map_list) {
        generate_state_map(state_map, package_name, fsmclass, class_name, action_map, transition_set);
    }
}


static void generate_state_map(state_map_t const& state_map, std::string const& package_name, std::string const& fsmclass, std::string const& class_name, std::map<string, argument_list_t> const& action_map, transition_set_t const& transition_set)
{
    string full_fsm_name = package_name + "_" + fsmclass;

    string const& state_map_name = package_name + "_" + state_map.get_state_map_name();
    state_list_t const& state_list = state_map.get_state_list();

    // カスタムアクションの出力とステート遷移の出力
    auto generate_transition = [&](transition_t const& transition, int indent)
    {
        intented_out out(cout, indent);

        // 遷移先のステートがある場合には Exit() を出力する。
        if ( !transition.get_next_state().empty()) {
            out << "[fsm.currentState fsm:fsm ctxt:ctxt Exit];\n";
        }

        // 現在のステートをNULLにセットする。
        out << "fsm.previousState = fsm.currentState;\n";

        // カスタムアクションを出力する。
        if ( !transition.get_action_list().empty()) {
            out << "fsm.currentState = nil;\n";
            out << "//try {" << "\n";
            out << "// Custom action" << "\n";
        }

        for (auto const& action : transition.get_action_list()) {
            out << "[ctxt " << action.get_action() << ":fsm ";
            bool first = true;
            for (auto const& arg : action.get_arguments()) {
                if (first) {
                    first = false;
                } else {
                    cout << " ";
                }
                cout << arg << ":" << arg;
            }
            cout << "];\n";
        }

        // 次の遷移先への移動をfinally区の中に入れるかどうか
        int next_state_indent = 0;

        if ( !transition.get_action_list().empty()) {
            out << "//} finally {" << "\n";
            //next_state_indent++;
        }

        // 遷移先のステートがある場合には Entry() を出力する。
        if (transition.get_next_state().empty()) {
             out(next_state_indent) << "fsm.currentState = fsm.previousState;\n";
        }
        else {
             out(next_state_indent) << "[fsm setState:" << state_map_name << "._" << transition.get_next_state() << "];\n";
             out(next_state_indent) << "[fsm.currentState fsm:fsm ctxt:ctxt Entry];\n";
        }

        if ( !transition.get_action_list().empty()) {
            out << "//}" << "\n";
        }
    };

    const string state_map_protocol_name = state_map_name + "_State";
    cout << "private protocol " << state_map_protocol_name << " {\n";
    cout << "    func getName() -> String\n";
    cout << "    func Entry(fsm:" << full_fsm_name << ", ctxt:" << class_name << ")\n";
    cout << "    func Exit(fsm:" << full_fsm_name << ", ctxt:" << class_name << ")\n";
    cout << "    func Default(fsm:" << full_fsm_name << ", ctxt:" << class_name << ")\n";
    for (auto&& transition_name : transition_set) {
        if (transition_name != "Default") {
            cout << "    func " << transition_name << "(fsm:" << full_fsm_name << ", ctxt:" << class_name << ")\n";
        }
    }
    cout << "}\n";

    intented_out out(cout, 0);
    bool need_comma = false;
    out << "let " << state_map_name << " = _" << state_map_name << "()\n";
    cout << "\n";
    out << "public class _" << state_map_name << " {\n";
    for (auto const& state : state_list) {
        cout << "    let _" << state.get_state_name() << " = " << state.get_state_name() << "()\n";
    }

    cout << "\n";
    const string base_state_impl_name = state_map_name + "_StateImpl";

    out(1) << "public class " << base_state_impl_name << " : " << state_map_protocol_name << " {\n";
    out(2) << "func getName() -> String { return \"\" }\n";
    out(2) << "func Entry(fsm:" << full_fsm_name << ", ctxt:" << class_name << ") {}\n";
    out(2) << "func Exit(fsm:" << full_fsm_name << ", ctxt:" << class_name << ") {}\n";
    out(2) << "func Default(fsm:" << full_fsm_name << ", ctxt:" << class_name << ") { abort() }\n";
    for (auto&& transition_name : transition_set) {
        if (transition_name != "Default") {
            out(2) << "func " << transition_name << "(fsm:" << full_fsm_name << ", ctxt:" << class_name << ") {\n";
            out(3) << "Default(fsm, ctxt: ctxt)\n";
            out(2) << "}\n";
        }
    }
    out(1) << "}\n";

    bool default_state_present = false;

    for (auto const& state : state_list) {
        bool default_state = state.get_state_name() == "Default";
        const string func_decl = "override public func ";
        if (default_state)
            default_state_present = true;

        cout << "\n";
        out(0) << "@implementation " << state.get_state_name() << " : "<< (default_state ? base_state_impl_name : "Default") << " {\n";
        out(0) << "- (NSString*)name {\n";
        out(0) << "    return @\"" << state.get_state_name() << "\";\n";
        out(0) << "}\n";
        cout << "\n";
        out(0) << "- (void)fsm:(" << full_fsm_name << " *)fsm ctxt:(" << class_name << " *)ctxt Entry {\n";

        for (auto&& action : state.get_entry()) {
            out(1) << "[ctxt " << action.get_action() << ":fsm ";
            bool first = true;
            for (auto const& arg : action.get_arguments()) {
                if (first) {
                    first = false;
                } else {
                    cout << " ";
                }
                cout << arg << ":" << arg;
            }
            cout << "];\n";
        }
        out(0) << "}\n";
        cout << "\n";
        out(0) << "- (void)fsm:(" << full_fsm_name << " *)fsm ctxt:(" << class_name << " *)ctxt Exit {\n";
        for (auto const& action : state.get_exit()) {
            // out(1) << "ctxt." << action.func() << "\n";
            out(1) << "[ctxt " << action.get_action() << ":fsm ";
            bool first = true;
            for (auto const& arg : action.get_arguments()) {
                if (first) {
                    first = false;
                } else {
                    cout << " ";
                }
                cout << arg << ":" << arg;
            }
            cout << "];\n";
        }
        out(0) << "}\n";

        for (auto const& transition_name_list_pair : state.get_transitions()) {
            string const& transition_name = transition_name_list_pair.first;

            cout << "\n";
            out(0) << "- (void)fsm:(" << full_fsm_name << " *)fsm ctxt:(" << class_name << " *)ctxt " << transition_name << "\n";
            for (auto&& parameter : transition_name_list_pair.second.at(0).get_parameter_list()) {
                cout << ", " << parameter.get_name() << ":" << parameter.get_type();
            }
            cout << "{\n";

            bool else_block = false;
            for (auto&& transition : transition_name_list_pair.second) {
                if ( !transition.get_guard().empty()) {
                    out(1) << (else_block ? "else if" : "if") << " (" << transition.get_guard() << ") {\n";

                    generate_transition(transition, 5);

                    out(1) << "}\n";
                    else_block = true;
                }
                else {
                    if (else_block) {
                        out(1) << "else {\n";
                    }

                    generate_transition(transition, else_block ? 2 : 1);

                    if (else_block) {
                        out(1) << "}\n";
                    }
                }
            }

            out(0) << "}\n";
        }

        cout << "\n";
        out(1) << "}\n"
             ;
        need_comma = true;
    }

    if ( !default_state_present) {
        cout << "\n";
        out(1) << "public class Default : " << state_map_name << "_State {\n";
        out(2) << "public func getName() -> String { return \"DefaultState\" }\n\n";
        out(2) << "public func Entry(fsm:" << full_fsm_name << ", ctxt:" << class_name << ") {}\n";
        out(2) << "public func Exit(fsm:" << full_fsm_name << ", ctxt:" << class_name << ") {}\n";

        for (auto&& transition_name : transition_set) {
             out(2) << "public func " << transition_name << "(fsm:" << full_fsm_name << ", ctxt:" << class_name << ") {}\n";
        }
        out(1) << "}\n";
    }

    cout << "}" << endl;

}
