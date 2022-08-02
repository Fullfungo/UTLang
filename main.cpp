#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include "compiler_stream.hpp"
#include "utlang_parser.hpp"

std::string token_to_string(utlang::tokenisation::token const &t){
    static constexpr std::array token_fields = {
        std::make_pair(&utlang::tokenisation::token::is_general_name, "is_general_name"),
        std::make_pair(&utlang::tokenisation::token::is_ignored_name, "is_ignored_name"),
        std::make_pair(&utlang::tokenisation::token::is_type_identifier, "is_type_identifier"),
        std::make_pair(&utlang::tokenisation::token::is_variable_identifier, "is_variable_identifier"),
        std::make_pair(&utlang::tokenisation::token::is_match_expression_identifier, "is_match_expression_identifier"),
        std::make_pair(&utlang::tokenisation::token::is_match_case_identifier, "is_match_case_identifier"),
        std::make_pair(&utlang::tokenisation::token::is_namespace_identifier, "is_namespace_identifier"),
        std::make_pair(&utlang::tokenisation::token::is_import_identifier, "is_import_identifier"),
        std::make_pair(&utlang::tokenisation::token::is_namespace_resolution_operator, "is_namespace_resolution_operator"),
        std::make_pair(&utlang::tokenisation::token::is_match_case_introduction, "is_match_case_introduction"),
        std::make_pair(&utlang::tokenisation::token::is_function_type_builder, "is_function_type_builder"),
        std::make_pair(&utlang::tokenisation::token::is_type_constructor_list_separator, "is_type_constructor_list_separator"),
        std::make_pair(&utlang::tokenisation::token::is_lambda_expression_identifier, "is_lambda_expression_identifier"),
        std::make_pair(&utlang::tokenisation::token::is_lambda_expression_introduction, "is_lambda_expression_introduction"),
        std::make_pair(&utlang::tokenisation::token::is_type_annotation, "is_type_annotation"),
        std::make_pair(&utlang::tokenisation::token::is_definition_operator, "is_definition_operator"),
        std::make_pair(&utlang::tokenisation::token::is_grouping_bracket_left, "is_grouping_bracket_left"),
        std::make_pair(&utlang::tokenisation::token::is_grouping_bracket_right, "is_grouping_bracket_right"),
        std::make_pair(&utlang::tokenisation::token::is_block_bracket_left, "is_block_bracket_left"),
        std::make_pair(&utlang::tokenisation::token::is_block_bracket_right, "is_block_bracket_right"),
        std::make_pair(&utlang::tokenisation::token::is_statement_separator, "is_statement_separator")
    };

    std::string token_debug_form = "(\"" + t.token_value + "\"";
    for (auto [field, text]: token_fields)
        if (t.*field)
            token_debug_form += std::string(", ") + text;
    token_debug_form += ")";
    return token_debug_form;
}

std::string file_to_string(std::ifstream &file){
    std::ostringstream string_stream;
    string_stream << file.rdbuf();
    return string_stream.str();
}

std::vector<std::string> tokenise_file(std::ifstream &file){
    std::string file_content = file_to_string(file);
    auto const token_stream = utlang::tokenisation::tokenise(file_content);
    std::vector<std::string> token_debug_info_stream;
    for (auto const &t: token_stream)
        token_debug_info_stream.emplace_back(token_to_string(t));
    // token_debug_info_stream.reserve(token_stream.size());
    // std::transform(token_stream.cbegin(), token_stream.cend(), token_debug_info_stream.begin(), token_to_string);
    return token_debug_info_stream;
}

int square(int x){
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return x*x;
}

auto get_sqares(int n){
    auto int_str = utlang::object_pipeline<int>{};
    for (auto i = 0; i < n; ++i)
        int_str << i;
    return int_str.transform(square);
}

auto get_list(int n){
    auto int_list = utlang::object_pipeline<int>{};
    for (auto i = 0; i < n; ++i)
        int_list << i;
    // std::this_thread::sleep_for(std::chrono::seconds(10));
    return int_list;
}

int main(){
    std::ifstream file("clean_test.utlang");
    for (auto td: tokenise_file(file))
        std::cout << td << '\n';

    // auto il = get_list(10);
    // for (auto i: il.transform_and_combine(get_list).get())
    //     std::cout << i << ' ';
    /*// std::cout << '\n';
    */
    // /*
};
