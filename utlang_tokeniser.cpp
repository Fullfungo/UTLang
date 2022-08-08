#include <cctype>
#include <algorithm>
#include <functional>
#include <numeric>
#include "compiler_stream.hpp"
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


bool symbol_is_new_line_like(char c){
    return c == '\n' or c == '\r';
}

// bool symbol_is_space_like(char c){
//     return std::isspace(static_cast<unsigned char>(c)) and not symbol_is_new_line_like(c);
// }

bool symbol_is_name_like(char c){
    return std::isalnum(static_cast<unsigned char>(c)) or c == '_';
}

bool symbol_is_operator_like(char c){
    return std::ispunct(static_cast<unsigned char>(c)) and c != '_';
}

bool symbol_is_token_like(char c){
    return symbol_is_name_like(c) or symbol_is_operator_like(c);
}

// enum class token_mode {empty, name_like, operator_like, error};
// enum class character_mode {error, name_like, operator_like, space_like, new_line_like};
// enum class token_mode {empty, name_like, operator_like, single_line_comment, block_comment};

// character_mode symbol_token_mode(char c){
//     if (symbol_is_name_like(c))
//         return character_mode::name_like;
//     if (symbol_is_operator_like(c))
//         return character_mode::operator_like;
//     if (symbol_is_space_like(c))
//         return character_mode::space_like;
//     if (symbol_is_new_line_like(c))[[likely]]
//         return character_mode::new_line_like;
//     return character_mode::error;
// }

namespace utlang::tokenisation{

// returns text between comments
// Anything that LOOKS LIKE the start of a comment IS a start if a comment
auto text_to_code_portions(std::string_view input_text){
    auto pipeline = object_pipeline<std::string_view>{};

    while(not input_text.empty()){ // if file ends on a comment, don't push empty portion
        auto const single_line_comment_position   = input_text.find(token::single_line_comment_identifier);
        auto const block_comment_position         = input_text.find(token::block_comment_start);

        if (single_line_comment_position == std::string_view::npos and block_comment_position == std::string_view::npos){ // no comments left
            pipeline << input_text;
            break;
        }else if (single_line_comment_position < block_comment_position){ // single-line comment appears first
            if (single_line_comment_position)
                pipeline << input_text.substr(0, single_line_comment_position);
            input_text.remove_prefix(single_line_comment_position + token::single_line_comment_identifier.length());
            auto const end_of_comment = std::find_if(input_text.cbegin(), input_text.cend(), symbol_is_new_line_like);
            if (end_of_comment == input_text.cend()) // coment ends at the end-of-file [ok]
                break;
            input_text.remove_prefix(end_of_comment - input_text.cbegin() + 1);
        }else{ // multi-line comment appears first
            if (block_comment_position)
                pipeline << input_text.substr(0, block_comment_position);
            input_text.remove_prefix(block_comment_position + token::block_comment_start.length());
            auto const comment_length = input_text.find(token::block_comment_end);
            if (comment_length == input_text.length())  // coment is not closed [bad]
                throw 0;
            input_text.remove_prefix(comment_length + token::block_comment_end.length());
        }
    }

    return pipeline;
}

class token_cluster{
    public:
        std::string_view token_cluster_text;
        enum class cluster_type{name_like, operator_like} type;
};

auto code_portion_to_token_clusters(std::string_view code_portion){
    auto pipeline = object_pipeline<token_cluster>{};

    while (true){
        auto const current_token_position = std::find_if(code_portion.cbegin(), code_portion.cend(), symbol_is_token_like);
        if (current_token_position == code_portion.cend())
            break;
        code_portion.remove_prefix(current_token_position - code_portion.cbegin());  

        auto const search_function = symbol_is_name_like(code_portion.front()) ? symbol_is_name_like : symbol_is_operator_like;
        auto const cluster_type    = symbol_is_name_like(code_portion.front()) ? token_cluster::cluster_type::name_like : token_cluster::cluster_type::operator_like;

        auto const current_token_end = std::find_if_not(code_portion.cbegin(), code_portion.cend(), search_function);
        pipeline << token_cluster{code_portion.substr(0, current_token_end - code_portion.cbegin()), cluster_type};
        code_portion.remove_prefix(current_token_end - code_portion.cbegin());
    }

    return pipeline;
}

auto split_cluster_into_tokens(token_cluster const &cluster){
    auto pipeline = object_pipeline<token>{};

    if (cluster.type == token_cluster::cluster_type::name_like)
        pipeline << token{cluster.token_cluster_text};
    else{
        auto token_text = cluster.token_cluster_text; // may contain multiple tokens like ;;;

        while (not token_text.empty()){
            auto possible_operator_text = token_text;
            auto possible_token = token{possible_operator_text};
            while (possible_token.is_not_determined() and possible_operator_text.size() > 1){ // find the longest operator token
                possible_operator_text.remove_suffix(1);
                possible_token = token{possible_operator_text};
            }

            pipeline << std::move(possible_token);
            if (possible_token.is_not_determined())[[unlikely]] // return the broken token anyway
                throw 0;
            
            token_text.remove_prefix(possible_operator_text.length());
        }
    }

    return pipeline;
}

/*
                    (next symbol)   abc         ->;             ' '         '\n'
    (current_token) 
    abc                             += c        push; = c       push        push
    ->;                             push; = c   += c            push        push
    ' '                             = c         = c             ignore      ignore
    comment                         ignore      ignore          ignore      mode = NONE
    block_comment                   ignore      check == *_/    ignore      ignore
*/

std::vector<token> tokenise(const std::string_view input_text){
    return text_to_code_portions(input_text).transform_and_combine(code_portion_to_token_clusters).transform_and_combine(split_cluster_into_tokens).get();
    /*
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
                current_token_mode = token_mode::single_line_comment;
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
                    case token_mode::single_line_comment:
                        break;
                    case token_mode::block_comment:
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
                    case token_mode::single_line_comment:
                        break;
                    case token_mode::block_comment:
                        //
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
                    case token_mode::single_line_comment:
                        break;
                    case token_mode::block_comment:
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
                    case token_mode::single_line_comment:
                        current_token_mode = token_mode::empty;
                        break;
                    case token_mode::block_comment:
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
    return token_stream;*/
}

}
