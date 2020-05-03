#ifndef QIRAST
#define QIRAST
#define BOOST_SPIRIT_USE_PHOENIX_V3 1

#include <vector>
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

    struct ifstat;

    template<typename T>
    struct vdec;

    template<typename Op>
    struct binary_op;

    template<typename T>
    struct unary_op;

    struct var_ref {
        std::string id;

        var_ref(const std::string& id_) : id(id_) {}
    };

    struct assign;

    using value = boost::variant<
        int,
        bool,
        boost::recursive_wrapper<ifstat>,
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
        boost::recursive_wrapper<var_ref>,
        boost::recursive_wrapper<assign>
    >;

    struct ifstat {
        value cond;
        std::vector<value> then_stat;
        std::vector<value> else_stat;

        ifstat(const value& cond_, const std::vector<value>& then_stat_, const std::vector<value>& else_stat_)
            : cond(cond_), then_stat(then_stat_), else_stat(else_stat_) {}
    };

    template<typename T>
    struct vdec {
        std::string id;
        value val;

        vdec(std::string& id_, const value& val_) : id(id_), val(val_) {}
    };

    template<typename Op>
    struct binary_op {
        value lhs;
        value rhs;

        binary_op(const value& lhs_, const value& rhs_) : lhs(lhs_), rhs(rhs_) {}
    };

    template<typename T>
    struct unary_op {
        value val;

        unary_op(const value& val_) : val(val_) {}
    };

    struct assign {
        std::string id;
        value val;

        assign(std::string& id_, const value& val_) : id(id_), val(val_) {}
    };
}

#endif