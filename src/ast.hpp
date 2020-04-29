#ifndef QIRAST
#define QIRAST

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/variant.hpp>

using namespace boost::spirit;
namespace phx = boost::phoenix;

namespace ast {

    struct lt;
    struct lte;
    struct gt;
    struct gte;
    struct eql;
    struct neq;
    struct add;
    struct sub;
    struct mul;
    struct div;
    struct mod;


    template<typename Op>
    struct binary_op;

    using expr = boost::variant<
        int,
        bool,
        boost::recursive_wrapper<binary_op<lt>>,
        boost::recursive_wrapper<binary_op<lte>>,
        boost::recursive_wrapper<binary_op<gt>>,
        boost::recursive_wrapper<binary_op<gte>>,
        boost::recursive_wrapper<binary_op<eql>>,
        boost::recursive_wrapper<binary_op<neq>>,
        boost::recursive_wrapper<binary_op<add>>,
        boost::recursive_wrapper<binary_op<sub>>,
        boost::recursive_wrapper<binary_op<mul>>,
        boost::recursive_wrapper<binary_op<div>>,
        boost::recursive_wrapper<binary_op<mod>>
    >;

    template<typename Op>
    struct binary_op {
        expr lhs;
        expr rhs;

        binary_op(expr const& lhs_, expr const& rhs_) : lhs(lhs_), rhs(rhs_) {}
    };
}

#endif