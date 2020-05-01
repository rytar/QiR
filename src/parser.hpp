#ifndef QIRPARSER
#define QIRPARSER

#include <vector>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/variant.hpp>
#include "ast.hpp"

namespace parser {

    template<typename Iterator>
    struct grammar : public qi::grammar<Iterator, std::vector<ast::value>(), qi::space_type> {
        template<typename T>
        using rule = qi::rule<Iterator, T, qi::space_type>;

        rule<ast::value()> expr, expr2, expr3, expr4, constant, variant, vdec, vid, id, assign;
        rule<std::vector<ast::value>()> code;
        std::string id_save, vdec_id;

        grammar() : grammar::base_type(code) {
            code %= *(
                (vdec[phx::bind(&grammar::id_save, this) = phx::bind(&grammar::vdec_id, this) = ""]
                | assign[phx::bind(&grammar::id_save, this) = phx::bind(&grammar::vdec_id, this) = ""]
                | expr[phx::bind(&grammar::id_save, this) = phx::bind(&grammar::vdec_id, this) = ""]
                ) >> qi::lit(';'));
            vdec = qi::lit("let") >> vid >> qi::lit(':')
                >> (
                    qi::lit("int") >> qi::lit('=') >> expr[_val = phx::construct<ast::vdec<int>>(phx::bind(&grammar::vdec_id, this), _1)]
                    | qi::lit("bool") >> qi::lit('=') >> expr[_val = phx::construct<ast::vdec<bool>>(phx::bind(&grammar::vdec_id, this), _1)]
                )[phx::bind(&grammar::vdec_id, this) = ""];
            assign = vid >> qi::lit('=') >> expr[_val = phx::construct<ast::assign>(phx::bind(&grammar::vdec_id, this), _1)][phx::bind(&grammar::vdec_id, this) = ""];
            vid = *(qi::char_ - qi::char_("=:;"))[phx::push_back(phx::bind(&grammar::vdec_id, this), _1)];
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
            expr4 %= constant
                | (qi::lit('(') >> expr >> qi::lit(')'))[_val = _1]
                | id[_val = phx::construct<ast::var_ref>(phx::bind(&grammar::id_save, this))][phx::bind(&grammar::id_save, this) = ""];
            constant = qi::int_ | qi::bool_;
            id = *(qi::char_ - qi::char_('+') - qi::char_("-*/%<>!=:;"))[phx::push_back(phx::bind(&grammar::id_save, this), _1)];
        }
    };
}

#endif