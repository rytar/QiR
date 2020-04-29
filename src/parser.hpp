#ifndef QIRPARSER
#define QIRPARSER

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/variant.hpp>
#include "ast.hpp"

namespace parser {

    template<typename Iterator>
    struct grammar : public qi::grammar<Iterator, ast::expr(), qi::space_type> {
        template<typename T>
        using rule = qi::rule<Iterator, T, qi::space_type>;

        rule<ast::expr()> expr, expr2, expr3, expr4, constant;

        grammar() : grammar::base_type(expr) {
            expr %= expr2[_val = _1]
                    >> *(
                        qi::lit('<') >> expr2[_val = phx::construct<ast::binary_op<ast::lt>>(_val, _1)]
                        | qi::lit("<=") >> expr2[_val = phx::construct<ast::binary_op<ast::lte>>(_val, _1)]
                        | qi::lit('>') >> expr2[_val = phx::construct<ast::binary_op<ast::gt>>(_val, _1)]
                        | qi::lit(">=") >> expr2[_val = phx::construct<ast::binary_op<ast::gte>>(_val, _1)]
                        | qi::lit("==") >> expr2[_val = phx::construct<ast::binary_op<ast::eql>>(_val, _1)]
                        | qi::lit("!=") >> expr2[_val = phx::construct<ast::binary_op<ast::neq>>(_val, _1)]
                    );
            expr2 %= expr3[_val = _1]
                    >> *(
                        qi::lit('+') >> expr3[_val = phx::construct<ast::binary_op<ast::add>>(_val, _1)]
                        | qi::lit('-') >> expr3[_val = phx::construct<ast::binary_op<ast::sub>>(_val, _1)]
                    );
            expr3 %= expr4[_val = _1]
                    >> *(
                        (qi::lit('*') >> expr4[_val = phx::construct<ast::binary_op<ast::mul>>(_val, _1)])
                        | (qi::lit('/') >> expr4[_val = phx::construct<ast::binary_op<ast::div>>(_val, _1)])
                        | (qi::lit('%') >> expr4[_val = phx::construct<ast::binary_op<ast::mod>>(_val, _1)])
                    );
            expr4 %= constant | (qi::lit('(') >> expr >> qi::lit(')'))[_val = _1];
            constant = qi::int_ | qi::bool_;
        }
    };
}

#endif