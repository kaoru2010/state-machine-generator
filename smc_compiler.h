#pragma once

#include <stdlib.h>
#ifndef NDEBUG
#include <stdio.h>
#endif

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <deque>

typedef struct context_tag {
} context_t;

struct token_t {
    std::unique_ptr<std::string> text;
};

int get_token(context_t *context, token_t *token);

void *ParseAlloc(void *(*mallocProc)(size_t));
void ParseFree(void *parser,                    /* The parser to be deleted */
               void (*freeProc)(void*)     /* Function used to reclaim memory */
);
void Parse(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  token_t *yyminor,            /* The value for the token */
  void *context                /* Optional %extra_argument parameter */
);

#ifndef NDEBUG
void ParseTrace(FILE *stream, char *zPrefix);
#endif

using transition_set_t = std::set<std::string>;
using action_list_t = std::vector<std::string>;

class transition_t {
    std::string transition_name_, guard_, next_state_;
    action_list_t action_list_;

public:
    transition_t(std::string *transition_name, std::string *guard, std::string *next_state, action_list_t *action_list)
    :   transition_name_(transition_name ? *transition_name : "")
    ,   guard_(guard ? *guard : "")
    ,   next_state_(next_state ? *next_state : "")
    ,   action_list_(action_list ? *action_list : action_list_t())
    {
    }

    std::string get_transition_name() const { return transition_name_; }
    std::string get_guard() const { return guard_; }
    std::string get_next_state() const { return next_state_; }
    action_list_t get_action_list() const { return action_list_; }
};

using transition_list_t = std::map<std::string, std::deque<transition_t>>;

class state {
    std::string state_name_;
    action_list_t entry_, exit_;
    transition_list_t transitions_;

public:
    state(std::string *state_name, action_list_t *entry, action_list_t *exit, transition_list_t *transitions)
    :   state_name_(*state_name)
    ,   entry_(entry ? *entry : action_list_t())
    ,   exit_(exit ? *exit : action_list_t())
    ,   transitions_(transitions ? *transitions : transition_list_t())
    {
    }

    std::string get_state_name() const { return state_name_; }
    action_list_t get_entry() const { return entry_; }
    action_list_t get_exit() const { return exit_; }
    transition_list_t get_transitions() const { return transitions_; }
};

using state_list_t = std::vector<state>;

class state_map_t
{
    std::string state_map_name_;
    state_list_t state_list_;

public:
    state_map_t(std::string const& state_map_name, state_list_t const& state_list)
    :   state_map_name_(state_map_name)
    ,   state_list_(state_list)
    {
    }

    std::string get_state_map_name() const { return state_map_name_; }
    state_list_t get_state_list() const { return state_list_; }
};

using state_map_list_t = std::vector<state_map_t>;

void set_start(std::string const&, std::string const&);
void set_fsmclass(std::string const&);
void set_package_name(std::string const&);
void set_class_name(std::string const&);
void define_map(std::string const&, std::string const&);
void define_map(std::string const& word, state_list_t const& states);
void define_transition(std::string const& transition);
void define_action(std::string const& action);
