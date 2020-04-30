#ifndef QIRAST
#define QIRAST
#define BOOST_SPIRIT_USE_PHOENIX_V3 1

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

    template<typename T>
    struct vdec;

    template<typename Op>
    struct binary_op;

    struct var_ref {
        std::string id;

        var_ref(const std::string& id_) : id(id_) {}
    };

    using value = boost::variant<
        int,
        bool,
        boost::recursive_wrapper<vdec<int>>,
        boost::recursive_wrapper<vdec<bool>>,
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
        boost::recursive_wrapper<binary_op<mod>>,
        boost::recursive_wrapper<var_ref>
    >;

    template<typename T>
    struct vdec {
        value val;
        std::string id;

        vdec(const value& val_, const std::string& id_) : val(val_), id(id_) {}
    };

    template<typename Op>
    struct binary_op {
        value lhs;
        value rhs;

        binary_op(const value& lhs_, const value& rhs_) : lhs(lhs_), rhs(rhs_) {}
    };
}

#endif