#ifndef UTLANG_TOKENISER_HPP
#define UTLANG_TOKENISER_HPP

// #include <iostream>
#include <vector>
#include <array>
// #include <optional>
#include <tuple>
// #include <variant>
#include <string>
#include <string_view>
#include <algorithm>


template<class T, typename std::array<T, 1>::size_type N, typename std::array<T, 1>::size_type M>
constexpr auto combine_arrays(std::array<T, N> const &arr1, std::array<T, M> const &arr2){
    std::array<T, N + M> res;
    for (typename std::array<T, 1>::size_type i = 0; i < N; ++i)
        res[i] = arr1[i];
    for (typename std::array<T, 1>::size_type i = 0; i < M; ++i)
        res[N + i] = arr2[i];
    return res;
}

template<class T, class U, typename std::array<T, 1>::size_type N>
constexpr auto member_fields_only(std::array<std::pair<T, U>, N> const &arr){
    std::array<T, N> res;
    std::transform(arr.cbegin(), arr.cend(), res.begin(), [](std::pair<T, U> const &pair){return pair.first;});
    return res;
}

namespace utlang::tokenisation{

    using namespace std::string_view_literals;

    class token{
        public:
            std::string token_value;

            // name-like tokens
            bool is_general_name                    = false;
            bool is_ignored_name                    = false;
            bool is_type_identifier                 = false;
            bool is_variable_identifier             = false;
            bool is_match_expression_identifier     = false;
            bool is_match_case_identifier           = false;
            bool is_namespace_identifier            = false;
            bool is_import_identifier               = false;

            // operator-like tokens
            bool is_namespace_resolution_operator   = false;
            bool is_match_case_introduction         = false;
            bool is_function_type_builder           = false;
            bool is_type_constructor_list_separator = false;
            bool is_lambda_expression_identifier    = false;
            bool is_lambda_expression_introduction  = false;
            bool is_type_annotation                 = false;
            bool is_definition_operator             = false;
            bool is_grouping_bracket_left           = false;
            bool is_grouping_bracket_right          = false;
            bool is_block_bracket_left              = false;
            bool is_block_bracket_right             = false;
            bool is_statement_separator             = false;

            // // comment token
            // bool is_single_line_comment_identifier  = false;
            // bool is_block_comment_start             = false;
            // bool is_block_comment_end               = false;

        public:
            token(const std::string_view input_text);
            constexpr bool is_not_determined() const;
            constexpr bool is_uniquely_determined() const;
            constexpr bool is_nonuniquely_determined() const;
        
        public:
            static constexpr std::array reserved_name_values = {
                std::make_pair(&token::is_ignored_name,                 "_"sv),
                std::make_pair(&token::is_type_identifier,              "type"sv),
                std::make_pair(&token::is_variable_identifier,          "let"sv),
                std::make_pair(&token::is_match_expression_identifier,  "match"sv),
                std::make_pair(&token::is_match_case_identifier,        "case"sv),
                std::make_pair(&token::is_namespace_identifier,         "namespace"sv),
                std::make_pair(&token::is_import_identifier,            "import"sv)
            };
            static constexpr std::array reserved_name_fields = member_fields_only(reserved_name_values);
            
            static constexpr std::array reserved_operator_values = {
                std::make_pair(&token::is_namespace_resolution_operator,    "::"sv),
                std::make_pair(&token::is_match_case_introduction,          ":"sv),
                std::make_pair(&token::is_function_type_builder,            "->"sv),
                std::make_pair(&token::is_type_constructor_list_separator,  "|"sv),
                std::make_pair(&token::is_lambda_expression_identifier,     "\\"sv),
                std::make_pair(&token::is_lambda_expression_introduction,   "->"sv),
                std::make_pair(&token::is_type_annotation,                  ":"sv),
                std::make_pair(&token::is_definition_operator,              "="sv),
                std::make_pair(&token::is_grouping_bracket_left,            "("sv),
                std::make_pair(&token::is_grouping_bracket_right,           ")"sv),
                std::make_pair(&token::is_block_bracket_left,               "{"sv),
                std::make_pair(&token::is_block_bracket_right,              "}"sv),
                std::make_pair(&token::is_statement_separator,              ";"sv),

                // std::make_pair(&token::is_single_line_comment_identifier,   single_line_comment_identifier),
                // std::make_pair(&token::is_block_comment_start,              block_comment_start),
                // std::make_pair(&token::is_block_comment_end,                block_comment_end)
            };
            static constexpr std::array reserved_operator_fields = member_fields_only(reserved_operator_values);

            static constexpr std::array reserved_values = combine_arrays(reserved_name_values, reserved_operator_values);
            static constexpr std::array reserved_fields = member_fields_only(reserved_values);

            constexpr static auto single_line_comment_identifier    = "//"sv;
            constexpr static auto block_comment_start               = "/*"sv;
            constexpr static auto block_comment_end                 = "*/"sv;
    };

    std::vector<token> tokenise(const std::string_view input_text);
}

#endif
