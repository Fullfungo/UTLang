#include <cctype>
#include <algorithm>
#include "utlang_tokeniser.hpp"

using namespace utlang::tokenisation;

bool symbol_is_name_like(char c);

constexpr bool is_general_name_like(const std::string_view input_text){
    return std::all_of(input_text.begin(), input_text.end(), symbol_is_name_like) and
            not std::isdigit(static_cast<unsigned char>(input_text.front()));
}

token::token(const std::string_view input_text):
    token_value(input_text),
    is_ignored_name(                    input_text == token_value_ignored_name),
    is_type_identifier(                 input_text == token_value_type_identifier),
    is_variable_identifier(             input_text == token_value_variable_identifier),
    is_match_expression_identifier(     input_text == token_value_match_expression_identifier),
    is_match_case_identifier(           input_text == token_value_match_case_identifier),
    is_match_case_introduction(         input_text == token_value_match_case_introduction),
    is_function_type_builder(           input_text == token_value_function_type_builder),
    is_type_constructor_list_separator( input_text == token_value_type_constructor_list_separator),
    is_lambda_expression_identifier(    input_text == token_value_lambda_expression_identifier),
    is_lambda_expression_introduction(  input_text == token_value_lambda_expression_introduction),
    is_type_annotation(                 input_text == token_value_type_annotation),
    is_definition_operator(             input_text == token_value_definition_operator),
    is_grouping_bracket_left(           input_text == token_value_grouping_bracket_left),
    is_grouping_bracket_right(          input_text == token_value_grouping_bracket_right),
    is_block_bracket_left(              input_text == token_value_block_bracket_left),
    is_block_bracket_right(             input_text == token_value_block_bracket_right),
    is_statement_separator(             input_text == token_value_statement_separator){

    is_general_name = is_general_name_like(input_text) and not 
                     (is_ignored_name or 
                      is_type_identifier or 
                      is_variable_identifier or 
                      is_match_expression_identifier or 
                      is_match_case_identifier);
}

constexpr int token_possibilities_amount(const token &t){
    return  t.is_general_name + 
            t.is_ignored_name + 
            t.is_type_identifier + 
            t.is_variable_identifier + 
            t.is_match_expression_identifier + 
            t.is_match_case_identifier + 
            t.is_match_case_introduction + 
            t.is_function_type_builder + 
            t.is_type_constructor_list_separator + 
            t.is_lambda_expression_identifier + 
            t.is_lambda_expression_introduction + 
            t.is_type_annotation + 
            t.is_definition_operator + 
            t.is_grouping_bracket_left + 
            t.is_grouping_bracket_right + 
            t.is_block_bracket_left + 
            t.is_block_bracket_right + 
            t.is_statement_separator;
}

constexpr bool token::is_not_determined() const{
    return token_possibilities_amount(*this) == 0;
}

constexpr bool token::is_uniquely_determined() const{
    return token_possibilities_amount(*this) == 1;
}

constexpr bool token::is_nonuniquely_determined() const{
    return token_possibilities_amount(*this) > 1;
}


bool symbol_is_space(char c){
    return std::isspace(static_cast<unsigned char>(c));
}

bool symbol_is_operator_like(char c){
    return std::ispunct(static_cast<unsigned char>(c)) and c != '_';
}

bool symbol_is_name_like(char c){
    return std::isalnum(static_cast<unsigned char>(c)) or c == '_';
}

enum class token_mode {none, name_like, operator_like, error};

token_mode symbol_token_mode(char c){
    if (symbol_is_space(c))
        return token_mode::none;
    if (symbol_is_operator_like(c))
        return token_mode::operator_like;
    if (symbol_is_name_like(c))
        return token_mode::name_like;
    return token_mode::error;
}

std::vector<token> tokenise(const std::string_view input_text){
    // characters: 1. spaces; 2. graphical; 3. controles[bad]
    // tokens: 1. names (a-z, A-Z, 0-9, _) 2. special operators
    
    auto token_stream = std::vector<token>{};
    auto current_token_text = std::string{};
    auto current_token_mode = token_mode::none;

    auto push_name_like_token = [](auto &token_stream, auto &current_token_text){
        auto const new_token = token{current_token_text};
        token_stream.push_back(new_token);
    };

    auto push_operator_like_tokens = [](auto &token_stream, auto &current_token_text){
        auto token_text = std::string_view{current_token_text}; // may contain multiple tokens ;;;

        while (not token_text.empty()){
            auto possible_operator_text = token_text;
            auto possible_token = token{possible_operator_text};
            while (possible_token.is_not_determined() and possible_operator_text.size() > 1){ // find the longest operator
                possible_operator_text.remove_suffix(1);
                possible_token = token{possible_operator_text};
            }
            if (possible_token.is_not_determined()){ // return the broken token
                auto broken_token = token{token_text};
                token_stream.push_back(broken_token);
                break;
            }else{ // continue with the rest of the text
                token_stream.push_back(possible_token);
                token_text.remove_prefix(possible_operator_text.size());
            }
        }
    };

    for (auto c : input_text){
        auto symbol_mode = symbol_token_mode(c);
        if (symbol_mode == token_mode::error){
            throw 0;
        }else if (current_token_mode == token_mode::none){ // possible start of a new token
            switch (symbol_mode){
                case token_mode::name_like:
                case token_mode::operator_like:
                    current_token_text = c;
                    current_token_mode = symbol_mode;
                    [[fallthrough]];
                default:
                    continue;
            }
        }else if (symbol_mode == current_token_mode){ // token continuation
            current_token_text += c;
        }else if (current_token_mode == token_mode::name_like){ // end of name_like token
            push_name_like_token(token_stream, current_token_text);
            current_token_text.clear();
            current_token_mode = symbol_mode;
        }else{ // end of operator_like token(s)
            push_operator_like_tokens(token_stream, current_token_text);
            current_token_text.clear();
            current_token_mode = symbol_mode;
        }
    }
    if (not current_token_text.empty()){ // in case the file ends on a token
        switch (current_token_mode){
            case token_mode::name_like:
                push_name_like_token(token_stream, current_token_text);
                break;
            case token_mode::operator_like:
                push_operator_like_tokens(token_stream, current_token_text);
                break;
            default:
                break;
        }
    }
    return token_stream;
}
