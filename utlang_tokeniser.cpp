#include <cctype>
#include <algorithm>
#include <functional>
#include <numeric>
#include "utlang_tokeniser.hpp"

using namespace utlang::tokenisation;

bool symbol_is_name_like(char c);

constexpr bool is_general_name_like(const std::string_view input_text){
    return std::all_of(input_text.begin(), input_text.end(), symbol_is_name_like) and
            not std::isdigit(static_cast<unsigned char>(input_text.front()));
}

token::token(const std::string_view input_text): token_value(input_text){
    for (auto [a, b] : reserved_values)
        this->*a = (input_text == b);
    
    is_general_name = is_general_name_like(input_text) and
                      std::none_of(reserved_name_fields.cbegin(), reserved_name_fields.cend(), [this](const auto b){return this->*b;});
}

constexpr int token_possibilities_amount(const token &t){
    // std::transform_reduce(token::reserved_indicators.cbegin(), token::reserved_indicators.cend(), int{}, std::plus<int>{}, [&t](bool token::*p)->int{return t.*p;});
    int possibilities{};
    for (auto const p: token::reserved_fields)
        possibilities += t.*p;
    return possibilities;
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


bool symbol_is_space_like(char c){
    return std::isspace(static_cast<unsigned char>(c)) and not (c == '\n' or c == '\v');
}

bool symbol_is_new_line_like(char c){
    return c == '\n' or c == '\v';
}

bool symbol_is_name_like(char c){
    return std::isalnum(static_cast<unsigned char>(c)) or c == '_';
}

bool symbol_is_operator_like(char c){
    return std::ispunct(static_cast<unsigned char>(c)) and c != '_';
}

// enum class token_mode {empty, name_like, operator_like, error};
enum class character_mode {error, name_like, operator_like, space_like, new_line_like};
enum class token_mode {empty, name_like, operator_like, comment};

character_mode symbol_token_mode(char c){
    if (symbol_is_name_like(c))
        return character_mode::name_like;
    if (symbol_is_operator_like(c))
        return character_mode::operator_like;
    if (symbol_is_space_like(c))
        return character_mode::space_like;
    if (symbol_is_new_line_like(c))[[likely]]
        return character_mode::new_line_like;
    return character_mode::error;
}

namespace utlang::tokenisation{

/*
                    (next symbol)   abc         ->;         ' '         '\n'
    (current_token) 
    abc                             += c        push; = c   push        push
    ->;                             push; = c   += c        push        push
    ' '                             = c         = c         ignore      ignore
    comment                         ignore      ignore      ignore      mode = NONE

*/

std::vector<token> tokenise(const std::string_view input_text){
    // characters: 1. spaces; 2. graphical; 3. controles[bad]
    // tokens: 1. names (a-z, A-Z, 0-9, _) 2. special operators
    
    auto token_stream = std::vector<token>{};
    auto current_token_text = std::string{};
    auto current_token_mode = token_mode::empty;

    auto push_name_like_token = [&](){
        auto const new_token = token{current_token_text};
        token_stream.push_back(new_token);
    };

    auto push_operator_like_tokens = [&](){
        auto token_text = std::string_view{current_token_text}; // may contain multiple tokens ;;;

        while (not token_text.empty()){
            auto possible_operator_text = token_text;
            auto possible_token = token{possible_operator_text};
            while (possible_token.is_not_determined() and possible_operator_text.size() > 1){ // find the longest operator
                possible_operator_text.remove_suffix(1);
                possible_token = token{possible_operator_text};
            }
            if (possible_token.is_not_determined())[[unlikely]]{ // return the broken token
                auto broken_token = token{token_text};
                token_stream.push_back(broken_token);
                break;
            }else if (possible_token.is_single_line_comment_identifier)[[unlikely]]{
                current_token_mode = token_mode::comment;
                break;
            }else{ // token found; continue with the rest of the text
                token_stream.push_back(possible_token);
                token_text.remove_prefix(possible_operator_text.size());
            }
        }
    };

    for (auto c : input_text){
        auto symbol_mode = symbol_token_mode(c);
        switch (symbol_mode){
            case character_mode::error:
                throw 0;
            case character_mode::name_like:{
                switch (current_token_mode){
                    case token_mode::name_like:
                        current_token_text += c;
                        break;
                    case token_mode::operator_like:
                        current_token_mode = token_mode::name_like;
                        push_operator_like_tokens();
                        current_token_text = c;
                        break;
                    case token_mode::empty:
                        current_token_mode = token_mode::name_like;
                        current_token_text = c;
                        break;
                    case token_mode::comment:
                        break;
                }
            } break;
            case character_mode::operator_like:{
                switch (current_token_mode){
                    case token_mode::name_like:
                        current_token_mode = token_mode::operator_like;
                        push_name_like_token();
                        current_token_text = c;
                        break;
                    case token_mode::operator_like:
                        current_token_text += c;
                        break;
                    case token_mode::empty:
                        current_token_mode = token_mode::operator_like;
                        current_token_text = c;
                        break;
                    case token_mode::comment:
                        break;
                }
            } break;
            case character_mode::space_like:{
                switch (current_token_mode){
                    case token_mode::name_like:
                        current_token_mode = token_mode::empty;
                        push_name_like_token();
                        break;
                    case token_mode::operator_like:
                        current_token_mode = token_mode::empty;
                        push_operator_like_tokens();
                        break;
                    case token_mode::empty:
                        break;
                    case token_mode::comment:
                        break;
                }
            } break;
            case character_mode::new_line_like:{
                switch (current_token_mode){
                    case token_mode::name_like:
                        push_name_like_token();
                        current_token_mode = token_mode::empty;
                        break;
                    case token_mode::operator_like:
                        push_operator_like_tokens();
                        current_token_mode = token_mode::empty; // mode is set last, since this is always the end of a comment
                        break;
                    case token_mode::empty:
                        break;
                    case token_mode::comment:
                        current_token_mode = token_mode::empty;
                        break;
                }
            } break;
        }
    }

    // for (auto c : input_text){
    //     auto symbol_mode = symbol_token_mode(c);
    //     if (symbol_mode == token_mode::error)[[unlikely]]{
    //         throw 0;
    //     }else if (current_token_mode == token_mode::empty){ // possible start of a new token
    //         switch (symbol_mode){
    //             case token_mode::name_like:
    //             case token_mode::operator_like:
    //                 current_token_text = c;
    //                 current_token_mode = symbol_mode;
    //                 [[fallthrough]];
    //             default:
    //                 continue;
    //         }
    //     }else if (symbol_mode == current_token_mode)[[likely]]{ // token continuation
    //         current_token_text += c;
    //     }else if (current_token_mode == token_mode::name_like){ // end of name_like token
    //         push_name_like_token(token_stream, current_token_text);
    //         current_token_text.clear();
    //         current_token_mode = symbol_mode;
    //         if (symbol_mode != token_mode::empty)
    //             current_token_text = c;
    //     }else{ // end of operator_like token(s)
    //         push_operator_like_tokens(token_stream, current_token_text);
    //         current_token_text.clear();
    //         current_token_mode = symbol_mode;
    //         if (symbol_mode != token_mode::empty)
    //             current_token_text = c;
    //     }
    // }
    // if (not current_token_text.empty()){ // in case the file ends on a token
    //     switch (current_token_mode){
    //         case token_mode::name_like:
    //             push_name_like_token(token_stream, current_token_text);
    //             break;
    //         case token_mode::operator_like: [[likely]]
    //             push_operator_like_tokens(token_stream, current_token_text);
    //             break;
    //         default:
    //             break;
    //     }
    // }
    return token_stream;
}

}
