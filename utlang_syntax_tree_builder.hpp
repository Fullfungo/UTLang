#ifndef UTLANG_SYNTAX_TREE_BUILDER_HPP
#define UTLANG_SYNTAX_TREE_BUILDER_HPP

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "utlang_tokeniser.hpp"

template<class... T>
using indirect_variant = std::variant<std::unique_ptr<T>...>;

using indirect_variant_index_type = size_t;

namespace utlang::syntax{
    /**
     * V - name
     * S - statement
     * E - expression
     * T - type
     * 
     * Type:
     * * _V_
     * * _T1_ -> _T2_
     * * (generic)???
     * 
     * Statement:
     * * type _V_ = _V_ _T1_ _T2_... | _V2_ ...;
     * * let _V_ : _T_ = _E_;
     * * namespace _V_ {_S1_; _S2_;...};
     * * import ???;
     * 
     * Expression:
     * * _V_
     * * (_V1_ _V2_ ...)
     * * match _V_ {case _Con_ ...: _E1_; ...; case _ : _E_}
     * * \_V_ -> _E_
     **/

    // ns1::ns2::ns3::...::name
    using scoped_name_type = std::vector<std::string>;

    struct Variable{
        // can be _ (ignored name)
        scoped_name_type name;
    };

    struct Constructor{ // special names for type constructors
        scoped_name_type name;
        // type is not inferred at this stage
    };

    // Expression
    struct Expression;

    struct Application{
        // (e1 e2 e3 ...)
        std::vector<Expression> arguments;
    };

    struct Lambda{
        // \x -> e
        Variable binder;
        Expression body;
    };

    // Expression: Case, Match
    struct Case_pattern;

    struct Case_pattern_application{
        Constructor cons;
        std::vector<Case_pattern> args;
    };

    struct Case_pattern{
        indirect_variant<Variable, Case_pattern_application> expr;
    };

    struct Case{
        // case ...: ...
        Case_pattern match_expr;
        Expression result_expr;
    };
    
    struct Match{
        // match (x){case ... : ...; case ... : ...; ...}
        std::vector<Case> cases;
    };

    struct Expression{
        // TO DO; index enum
        indirect_variant<Variable, Application, Match, Lambda> expr;
    };

    // Type
    struct Type;

    struct Simple_Type{
        scoped_name_type name;
    };

    struct Function_Type{
        // T1 -> T2
        Type argument_type;
        Type result_type;
    };

    struct Type_Application{
        std::vector<Type> types;
    };

    struct Type{
        // TO DO; index enum
        indirect_variant<Simple_Type, Function_Type, Type_Application> type; // Generics?
    };

    // Statement
    struct Statement;

    struct Block{
        // {st1; st2; ...};
        std::vector<Statement> statement_list;
    };

    struct Type_definition{
        // type T t1 t2 ... = C1 t1' t2' | ...; 
        Simple_Type type;
        std::vector<Simple_Type> parameter_types;
        std::vector<Constructor> constructors;
    };

    struct Variable_definition{
        // let x : T = e;
        Variable name;
        Type type;
        Expression value;
    };

    struct Namespace_definition{
        // namespace ns {st1; st2; ...};
        std::string name;
        Block content;
    };

    [[maybe_unused]] struct Import_declaration{ /* import ???; */};

    struct Statement{
        // TO DO; index enum
        indirect_variant<Block, Type_definition, Variable_definition, Namespace_definition, Import_declaration> st;
    };

    // Program
    struct Program_AST{
        // st1; st2; ...
        Block code;
    };

    Program_AST build_AST(const std::vector<utlang::tokenisation::token>&);
    
}

#endif