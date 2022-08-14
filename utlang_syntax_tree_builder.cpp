#include <tuple>
#include <stack>
#include "utlang_syntax_tree_builder.hpp"
#include "compiler_stream.hpp"

using namespace utlang::syntax;
using token = utlang::tokenisation::token;


Variable                    build_Variable                  (std::vector<token> const &token_list);
Constructor                 build_Constructor               (std::vector<token> const &token_list);
Expression                  build_Expression                (std::vector<token> const &token_list);
Application                 build_Application               (std::vector<token> const &token_list);
Lambda                      build_Lambda                    (std::vector<token> const &token_list);
Case_pattern                build_Case_pattern              (std::vector<token> const &token_list);
Case_pattern_application    build_Case_pattern_application  (std::vector<token> const &token_list);
Case                        build_Case                      (std::vector<token> const &token_list);
Match                       build_Match                     (std::vector<token> const &token_list);
Expression                  build_Expression                (std::vector<token> const &token_list);
Type                        build_Type                      (std::vector<token> const &token_list);
Simple_Type                 build_Simple_Type               (std::vector<token> const &token_list);
Function_Type               build_Function_Type             (std::vector<token> const &token_list);
Type_Application            build_Type_Application          (std::vector<token> const &token_list);
Statement                   build_Statement                 (std::vector<token> const &token_list);
Block                       build_Block                     (std::vector<token> const &token_list);
Type_definition             build_Type_definition           (std::vector<token> const &token_list);
Variable_definition         build_Variable_definition       (std::vector<token> const &token_list);
Namespace_definition        build_Namespace_definition      (std::vector<token> const &token_list);
Import_declaration          build_Import_declaration        (std::vector<token> const &token_list);

// give the tokens between the brackets + all following tokens
std::pair<std::vector<token>, std::vector<token>> find_closing_grouping_bracket(std::vector<token> const &token_list, size_t const opening_bracket_position);
std::pair<std::vector<token>, std::vector<token>> find_closing_block_bracket(std::vector<token> const &token_list, size_t const opening_bracket_position);


std::pair<std::vector<token>, std::vector<token>> find_closing_grouping_bracket(std::vector<token> const &token_list, size_t const opening_bracket_position){
    size_t depth = 1;
    for (size_t i = opening_bracket_position; i < token_list.size(); ++i){
        if (token_list[i].is_grouping_bracket_left)
            ++depth;
        else if (token_list[i].is_grouping_bracket_right){
            -- depth;
            if (depth == 0){
                std::vector<token> inside(token_list.cbegin() + opening_bracket_position + 1, token_list.cbegin() + i);
                std::vector<token> after(token_list.cbegin() + i + 1, token_list.cend());
                return std::make_pair(inside, after);
            }
        }
    }
    throw 0;
}
std::pair<std::vector<token>, std::vector<token>> find_closing_block_bracket(std::vector<token> const &token_list, size_t const opening_bracket_position){
    size_t depth = 1;
    for (size_t i = opening_bracket_position; i < token_list.size(); ++i){
        if (token_list[i].is_block_bracket_left)
            ++depth;
        else if (token_list[i].is_block_bracket_right){
            -- depth;
            if (depth == 0){
                std::vector<token> inside(token_list.cbegin() + opening_bracket_position + 1, token_list.cbegin() + i);
                std::vector<token> after(token_list.cbegin() + i + 1, token_list.cend());
                return std::make_pair(inside, after);
            }
        }
    }
    throw 0;
}

scoped_name_type build_scoped_name(std::vector<token> const &token_list){ // TO DO
    ;
}

Variable build_Variable(std::vector<token> const &token_list){ // TO DO
    return Variable{.name = build_scoped_name(token_list)};
}

Constructor build_Constructor(std::vector<token> const &token_list){ // TO DO
    return Constructor{.name = build_scoped_name(token_list)};
}

Expression build_Expression(std::vector<token> const &token_list){ // TO DO
    ;
}

Application build_Application(std::vector<token> const &token_list){ // TO DO
    ;
}

Lambda build_Lambda(std::vector<token> const &token_list){ // TO DO
    ;
}

Case_pattern build_Case_pattern(std::vector<token> const &token_list){ // TO DO
    ;
}

Case_pattern_application build_Case_pattern_application(std::vector<token> const &token_list){ // TO DO
    ;
}

Case build_Case(std::vector<token> const &token_list){ // TO DO
    ;
}

Match build_Match(std::vector<token> const &token_list){ // TO DO
    ;
}

Expression build_Expression(std::vector<token> const &token_list){ // TO DO
    ;
}

Type build_Type(std::vector<token> const &token_list){ // TO DO
    ;
}

Simple_Type build_Simple_Type(std::vector<token> const &token_list){ // TO DO
    return Simple_Type{.name = build_scoped_name(token_list)};
}

Function_Type build_Function_Type(std::vector<token> const &token_list){ // TO DO
    ;
}

Type_Application build_Type_Application(std::vector<token> const &token_list){ // TO DO
    ;
}

Statement build_Statement(std::vector<token> const &token_list){ // TO DO
    ;
}

Block build_Block(std::vector<token> const &token_list){ // TO DO
    ;
}

Type_definition build_Type_definition(std::vector<token> const &token_list){ // TO DO
    ;
}

Variable_definition build_Variable_definition(std::vector<token> const &token_list){ // TO DO
    ;
}

Namespace_definition build_Namespace_definition(std::vector<token> const &token_list){ // TO DO
    ;
}

Import_declaration build_Import_declaration(std::vector<token> const &token_list){ // TO DO
    ;
}

void check_brackets_paired(std::vector<token> const &token_list){
    enum class bracket_type: bool{grouping_bracket, block_bracket};
    std::stack<bracket_type> bracket_order;
    for (auto const &tok: token_list){
        if (tok.is_grouping_bracket_left)
            bracket_order.push(bracket_type::grouping_bracket);
        else if (tok.is_block_bracket_left)
            bracket_order.push(bracket_type::block_bracket);
        else if (tok.is_grouping_bracket_right){
            if (bracket_order.top() == bracket_type::grouping_bracket)
                bracket_order.pop();
            else
                throw 0;
        }else if (tok.is_block_bracket_right){
            if (bracket_order.top() == bracket_type::block_bracket)
                bracket_order.pop();
            else
                throw 0;
        }
    }
    if (not bracket_order.empty())
        throw 0;
}

Program_AST build_AST(std::vector<token> const &token_list){ // TO DO
    check_brackets_paired(token_list);
    ;
}
