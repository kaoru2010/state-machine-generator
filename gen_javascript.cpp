#include <iostream>
#include <string>
#include "smc_compiler.h"
#include "gen.h"

using namespace std;

static void generate_state_map(state_map_t const& state_map, transition_set_t const& transition_set);

/*
var StateMachine = function() {
    var start = function() { setState(1); };
    var stop = function() { setState(0); };
    var invoke = function(param) { alert(param); };
    var doNothing = function() {};

    var entryTable = [
        function() { $('#param').val(''); },
        function() { $('#param').css('background-color' ,'#ffcccc'); },
        doNothing
    ];

    var exitTable = [
        doNothing,
        function() { $('#param').css('background-color' ,'#ffffff'); },
        doNothing
    ];

    var decisionTable = [
        // STOPPED
        [
            start,
            doNothing,
            doNothing
        ],

        // RUNNING
        [
            doNothing,
            stop,
            invoke,
        ]
    ];

    var stateNameMap = ["STOPPED", "RUNNING"];

    var currentState = -1;
    var inTransition = false;
    var setState = function(state, action) {
        if (currentState >= 0) {
            exitTable[currentState]();
        }

        console.log("ENTER STATE: " + stateNameMap[state]);
        currentState = state;

        if (action) {
            action();
        }

        entryTable[state]();
    };
    var doTransition = function(f) {
        if (inTransition) throw "inTransition is true";
        inTransition = true;
        f();
        inTransition = false;
    };

    return {
        enterStartState: function() { setState(0); },
        start: function() { doTransition(function() { decisionTable[currentState][0](); }); },
        stop: function() { doTransition(function() { decisionTable[currentState][1](); }); },
        invoke: function(param) { doTransition(function() { decisionTable[currentState][2](param); }); }
    };
};
*/

namespace {

class generator
{
public:
    typedef std::shared_ptr<generator> shared_ptr;
public:
    virtual ~generator() {}
    virtual void write(std::ostream& out) = 0;
};

class json_object_generator : public generator
{
    std::vector<std::pair<std::string, generator::shared_ptr>> arr_;
    std::string separator_;
    std::string prefix_;

public:
    explicit json_object_generator()
    :   arr_()
    ,   separator_(", ")
    ,   prefix_()
    {}

    explicit json_object_generator(std::string const& separator, std::string const& prefix)
    :   arr_()
    ,   separator_(separator)
    ,   prefix_(prefix)
    {}

    virtual void write(std::ostream& out) {
        bool first = true;
        out << "{";
        for (auto pair : arr_) {
            if (first) {
                first = false;
            } else {
                out << separator_;
            }
            out << prefix_;
            out << pair.first << ": ";
            pair.second->write(out);
        }
        out << "}";
    }

    void add(std::string const& key, generator::shared_ptr ptr) { arr_.push_back(make_pair(key, ptr)); }
};

class json_array_generator : public generator
{
    std::vector<generator::shared_ptr> arr_;
    std::string separator_;
    std::string prefix_;

public:
    explicit json_array_generator()
    :   arr_()
    ,   separator_(", ")
    ,   prefix_()
    {}

    explicit json_array_generator(std::string const& separator, std::string const& prefix)
    :   arr_()
    ,   separator_(separator)
    ,   prefix_(prefix)
    {}

    virtual void write(std::ostream& out) {
        bool first = true;
        out << "[";
        for (auto ptr : arr_) {
            if (first) {
                first = false;
            } else {
                out << separator_;
            }
            out << prefix_;
            ptr->write(out);
        }
        out << "]";
    }

    void add(generator::shared_ptr ptr) { arr_.push_back(ptr); }
};

class line_generator : public generator
{
    std::vector<generator::shared_ptr> arr_;

public:
    virtual void write(std::ostream& out) {
        for (auto ptr : arr_) {
            ptr->write(out);
            out << "\n";
        }
    }

    void add(generator::shared_ptr ptr) { arr_.push_back(ptr); }
};

class statement_generator : public generator
{
    generator::shared_ptr ptr_;

public:
    explicit statement_generator(generator::shared_ptr ptr)
    :   ptr_(ptr)
    {}

    virtual void write(std::ostream& out) {
        ptr_->write(out);
        out << ";";
    }
};

inline
auto stmt_gen(generator::shared_ptr str) -> std::shared_ptr<statement_generator>
{
    return make_shared<statement_generator>(str);
}

class token_generator : public generator
{
    std::string str_;

public:
    explicit token_generator(std::string const& str)
    :   str_(str)
    {}

    virtual void write(std::ostream& out) { out << str_; }
};

inline
auto token_gen(std::string const& str) -> std::shared_ptr<token_generator>
{
    return make_shared<token_generator>(str);
}

class string_generator : public generator
{
    std::string str_;

public:
    explicit string_generator(std::string const& str)
    :   str_(str)
    {}

    virtual void write(std::ostream& out) { out << "\"" << str_ << "\""; }
};

inline
auto str_gen(std::string const& str) -> std::shared_ptr<string_generator>
{
    return make_shared<string_generator>(str);
}

class function_generator : public generator
{
    std::string function_name_;
    std::vector<std::string> arguments_;
    std::vector<generator::shared_ptr> function_body_;
    std::string prefix_;

public:
    explicit function_generator()
    :   function_name_()
    {}

    explicit function_generator(std::string const& function_name)
    :   function_name_(function_name)
    {}

    virtual void write(std::ostream& out) {
        if ( !function_name_.empty()) {
            out << prefix_ << "var " << function_name_ << " = ";
        } else {
            out << prefix_;
        }
        out << "function(";
        bool need_comma = false;
        for (auto const& argument : arguments_) {
            if (need_comma) out << ", ";
            out << argument;
            need_comma = true;
        }
        out << ") {\n";
        for (auto&& generator : function_body_) {
            out << prefix_ << "    ";
            generator->write(out);
            out << "\n";
        }
        out << prefix_ << "}";
    }

    void add_argument(std::string const& argument) {
        arguments_.push_back(argument);
    }

    void add_body(std::string const& line) {
        function_body_.push_back(token_gen(line));
    }

    void add_body(generator::shared_ptr line) {
        function_body_.push_back(line);
    }

    void set_prefix(std::string const& prefix) {
        prefix_ = prefix;
    }
};

inline
auto func_gen(std::string const& function_name) -> std::shared_ptr<function_generator>
{
    return make_shared<function_generator>(function_name);
}

inline
auto func_gen() -> std::shared_ptr<function_generator>
{
    return func_gen("");
}

class call_generator : public generator
{
    std::string function_name_;
    std::vector<generator::shared_ptr> arguemnts_;
    std::string prefix_;
    std::string separator_;

public:
    explicit call_generator(std::string const& function_name)
    :   function_name_(function_name)
    ,   separator_(", ")
    {}

    virtual void write(std::ostream& out) {
        out << prefix_ << function_name_ << "(";
        bool need_comma = false;
        for (auto&& arguemnt : arguemnts_) {
            if (need_comma) out << separator_;
            arguemnt->write(out);
            need_comma = true;
        }
        out << ")";
    }

    void add_argument(std::string const& arg) {
        add_argument(token_gen(arg));
    }

    void add_argument(generator::shared_ptr arg) {
        arguemnts_.push_back(arg);
    }

    void set_prefix(std::string const& prefix) {
        prefix_ = prefix;
    }

    void set_separator(std::string const& separator) {
        separator_ = separator;
    }
};

inline
auto call_gen(std::string const& function_name) -> shared_ptr<call_generator>
{
    return make_shared<call_generator>(function_name);
}

} // anonymous namespace

void gen_javascript(
    std::string const& package_name,
    std::string const& fsmclass,
    state_map_list_t const& state_map_list, // %map %% .. %%
    std::string const& start_map,
    std::string const& start_state,
    transition_set_t const& transition_set,
    std::string const& class_name // action class
)
{
    intented_out out(cout, 0);

    // https://github.com/stevekwan/experiments/blob/master/javascript/module-pattern.html
    out << "// -*- tab-width: 4; -*-\n";
    out << "// -*- coding: utf-8 -*-\n";
    out << "\n";
    out << "//\n";
    out << "// This file is automatically generated by State Machine Generator.\n";
    out << "//\n";
    out << "\n";
    out << "var " << package_name << " = " << package_name << " || {};\n";
    out << "" << package_name << "." << fsmclass << " = function(ctxt) {\n";
    out << "    var debugMode = false;\n";
    out << "    var currentState = null;\n";
    out << "    var inTransition = false;\n";
    out << "    var setState = function(state, f) {\n";
    out << "        // Invoke exit action\n";
    out << "        if (currentState && state) {\n";
    out << "            currentState[2]();\n";
    out << "        }\n";
    out << "\n";
    out << "        // Invoke transition action\n";
    out << "        if (f) {\n";
    out << "            f();\n";
    out << "        }\n";
    out << "\n";
    out << "        if (state) {\n";
    out << "            currentState = state;\n";
    out << "\n";
    out << "            // Invoke entry action\n";
    out << "            state[1]();\n";
    out << "\n";
    out << "            if (debugMode) { console.log('ENTER STATE: %s', state[0]); }\n";
    out << "        }\n";
    out << "    };\n";
    out << "    var doTransition = function(f) {\n";
    out << "        if (inTransition) throw 'In transition';\n";
    out << "\n";
    out << "        inTransition = true;\n";
    out << "        f();\n";
    out << "        inTransition = false;\n";
    out << "    };\n";
    out << "\n";

    for (auto&& state_map : state_map_list) {
        generate_state_map(state_map, transition_set);
    }

    out << "    var self = {\n";
    out << "        enterStartState: function() { setState(" << start_map << "." << start_state << "); },\n";
    out << "        getState: function() { return currentState; },\n";
    out << "        isDebugMode: function() { return debugMode; },\n";
    out << "        setDebugMode: function(mode) { debugMode = mode; },\n";
    out << "        getStateName: function() { return currentState[0]; }";

    int transition_id = 3;
    for (auto const& transition_name : transition_set) {
        if (transition_name != "Default") {
            out << ",\n";
            out << "        " << transition_name << ": function() { currentState[" << transition_id << "].apply(self, arguments); }";
            transition_id++;
        }
    }

    out << "\n";
    out << "    };\n";

    out << "    return self;\n";
    out << "};" << endl;
}

static void generate_state_map(state_map_t const& state_map, transition_set_t const& transition_set)
{

    string const& state_map_name = state_map.get_state_map_name();
    state_list_t const& state_list = state_map.get_state_list();

    // カスタムアクションの出力とステート遷移の出力
    auto generate_transition = [&](transition_t const& transition, std::shared_ptr<function_generator> gen)
    {
        auto set_state = call_gen("setState");
        gen->add_body(stmt_gen(set_state));

        set_state->set_separator(",\n");
        if (transition.get_next_state().empty()) {
            set_state->add_argument("null");
        } else {
            set_state->add_argument(state_map_name + "." + transition.get_next_state());
        }

        if ( !transition.get_action_list().empty()) {
            auto func = func_gen();
            func->set_prefix("            ");
            set_state->add_argument(func);

            for (auto const& action : transition.get_action_list()) {
                func->add_body("ctxt." + action.func() + ";");
            }
        }

/*


        // 遷移先のステートがある場合には Exit() を出力する。
        if ( !transition.get_next_state().empty()) {
            gen->add_body("currentState.Exit();");
        }

        // 現在のステートをNULLにセットする。
        gen->add_body("previousState = currentState;");

        // カスタムアクションを出力する。
        if ( !transition.get_action_list().empty()) {
            gen->add_body("currentState = null;");
            gen->add_body("try {");
            gen->add_body("    // Custom action");
        }

        for (auto const& action : transition.get_action_list()) {
            gen->add_body("    ctxt." + action.func() + ";");
        }

        // 次の遷移先への移動をfinally区の中に入れるかどうか
        int next_state_indent = 0;

        if ( !transition.get_action_list().empty()) {
            gen->add_body("} finally {");
            next_state_indent++;
        }

        // 遷移先のステートがある場合には Entry() を出力する。
        if (transition.get_next_state().empty()) {
             gen->add_body("currentState = previousState;");
        }
        else {
             //gen->add_body("setState(" << state_map_name << "." << transition.get_next_state() << ");\n";
             gen->add_body("currentState.Entry();");
        }

        if ( !transition.get_action_list().empty()) {
            gen->add_body("}");
        }
*/
    };

    auto function_list = make_shared<line_generator>();
    auto state_map_list = make_shared<json_object_generator>(",", "\n        ");

    auto no_ation = func_gen("noAction");
    no_ation->set_prefix("    ");
    function_list->add(no_ation);

    for (auto const& state : state_list) {
        if (state.get_state_name() == "Default") {
            continue;
        }

        auto state_map = make_shared<json_array_generator>();
        state_map_list->add(state.get_state_name(), state_map);

        // State name
        state_map->add(str_gen(state.get_state_name()));

        // Entry
        if (state.get_entry().empty()) {
            state_map->add(token_gen("noAction"));
        } else {
            auto func = func_gen(state.get_state_name() + "_Entry");
            func->set_prefix("    ");
            for (auto&& action : state.get_entry()) {
                 func->add_body("ctxt." + action.func() + ";");
            }
            function_list->add(func);
            state_map->add(token_gen(state.get_state_name() + "_Entry"));
        }

        // Exit
        if (state.get_exit().empty()) {
            state_map->add(token_gen("noAction"));
        } else {
            auto func = func_gen(state.get_state_name() + "_Exit");
            func->set_prefix("    ");
            for (auto&& action : state.get_exit()) {
                 func->add_body("ctxt." + action.func() + ";");
            }
            function_list->add(func);
            state_map->add(token_gen(state.get_state_name() + "_Exit"));
        }

        auto const& transition_map = state.get_transitions();
        for (auto const& transition_name : transition_set) {
            if (transition_name == "Default") continue;


            auto transition_list = transition_map.find(transition_name);
            if (transition_list == transition_map.end()) {
                //cout << ",\n";
                //out(3) << transition_name << ": " << state_map_name << ".Default." << transition_name;
                state_map->add(token_gen("noAction"));
                continue;
            }


            state_map->add(token_gen(state.get_state_name() + "_" + transition_name));

            auto func = func_gen(state.get_state_name() + "_" + transition_name);
            function_list->add(func);
            func->set_prefix("    ");
            for (auto&& parameter : transition_list->second.at(0).get_parameter_list()) {
                func->add_argument(parameter.get_name());
            }
            for (auto&& action : state.get_exit()) {
                 func->add_body("ctxt." + action.func() + ";");
            }


            //out(3) << "// " << transition_name << "\n";
            //out(3) << "function(";
            //bool first_parameter_seen = false;
            //for (auto&& parameter : transition_list->second.at(0).get_parameter_list()) {
            //    if (first_parameter_seen)
            //        cout << ", ";
            //    cout << parameter.get_name();
            //    first_parameter_seen = true;
            //}
            //cout << ") {\n";

            bool else_block = false;
            for (auto&& transition : transition_list->second) {
                if ( !transition.get_guard().empty()) {
                    string line = (else_block ? "else if" : "if");
                    line += " (" + transition.get_guard() + ") {";
                    func->add_body(line);

                    generate_transition(transition, func);

                    func->add_body("}");
                    else_block = true;
                }
                else {
                    if (else_block) {
                        func->add_body("else {");
                    }

                    generate_transition(transition, func);

                    if (else_block) {
                        func->add_body("}");
                    }
                }
            }
        }
    }

    function_list->write(cout);

    cout << "    var " << state_map_name << " = ";
    state_map_list->write(cout);
    cout << ";"  << endl;
}
