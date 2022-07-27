#ifndef UTLANG_TOKENISER_HPP
#define UTLANG_TOKENISER_HPP

// #include <iostream>
#include <vector>
// #include <optional>
// #include <tuple>
// #include <variant>
#include <string>
#include <string_view>

namespace utlang{
namespace tokenisation{

    using namespace std::string_view_literals;
    constexpr auto token_value_ignored_name                    = "_"sv;
    constexpr auto token_value_type_identifier                 = "type"sv;
    constexpr auto token_value_variable_identifier             = "let"sv;
    constexpr auto token_value_match_expression_identifier     = "match"sv;
    constexpr auto token_value_match_case_identifier           = "case"sv;
    constexpr auto token_value_match_case_introduction         = ":"sv;
    constexpr auto token_value_function_type_builder           = "->"sv;
    constexpr auto token_value_type_constructor_list_separator = "|"sv;
    constexpr auto token_value_lambda_expression_identifier    = "\\"sv;
    constexpr auto token_value_lambda_expression_introduction  = "->"sv;
    constexpr auto token_value_type_annotation                 = ":"sv;
    constexpr auto token_value_definition_operator             = "="sv;
    constexpr auto token_value_grouping_bracket_left           = "("sv;
    constexpr auto token_value_grouping_bracket_right          = ")"sv;
    constexpr auto token_value_block_bracket_left              = "{"sv;
    constexpr auto token_value_block_bracket_right             = "}"sv;
    constexpr auto token_value_statement_separator             = ";"sv;


    class token{
        public:
            std::string token_value;
            bool is_general_name                    = false;
            bool is_ignored_name                    = false;
            bool is_type_identifier                 = false;
            bool is_variable_identifier             = false;
            bool is_match_expression_identifier     = false;
            bool is_match_case_identifier           = false;
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

        public:
            token(const std::string_view input_text);
            constexpr bool is_not_determined() const;
            constexpr bool is_uniquely_determined() const;
            constexpr bool is_nonuniquely_determined() const;
    };

    std::vector<token> tokenise(const std::string_view input_text);

}
}

#endif
