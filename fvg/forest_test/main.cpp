/**************************************************************************************************/

// stdc++
#include <iostream>

#define FORESTVG_USING_STLAB_FOREST() 1

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

// application
#include "../headers/forest_algorithms.hpp"
#include "../headers/forest_fwd.hpp"

/**************************************************************************************************/

using namespace FNS; // either adobe or stlab, depending.

/**************************************************************************************************/

TEST_CASE("empty forest") {
    forest<int> f;
    REQUIRE(f.empty());
    REQUIRE(f.size() == 0);
    REQUIRE(f.begin() == f.end());
}

/**************************************************************************************************/

SCENARIO("single node forest") {
    forest<int> f;

    auto il = f.insert(f.end(), 42);
    REQUIRE(il.edge() == FNS::forest_leading_edge);
    REQUIRE(!f.empty());

    REQUIRE(f.begin() != f.end());
    REQUIRE(f.size() == 1);

    auto it = trailing_of(il);
    REQUIRE(it.edge() == FNS::forest_trailing_edge);
    REQUIRE(*it == *il);
}

/**************************************************************************************************/

auto big_test_forest() {
    forest<std::string> f;

    auto a_iter = trailing_of(f.insert(f.end(), "A"));

    auto b_iter = trailing_of(f.insert(a_iter, "B"));

    auto c_iter = trailing_of(f.insert(b_iter, "C"));
    auto d_iter = trailing_of(f.insert(b_iter, "D"));
    auto e_iter = f.insert(b_iter, "E");

    f.insert(c_iter, "F");
    f.insert(c_iter, "G");
    f.insert(c_iter, "H");

    f.insert(d_iter, "I");
    f.insert(d_iter, "J");
    f.insert(d_iter, "K");

    REQUIRE(f.size() == 11);

    REQUIRE(has_children(a_iter));
    REQUIRE(has_children(b_iter));
    REQUIRE(has_children(c_iter));
    REQUIRE(has_children(d_iter));
    REQUIRE(!has_children(e_iter));

    return f;
}

/**************************************************************************************************/

template <typename Iterator>
void test_fullorder_traversal(Iterator first, Iterator last, std::string expected) {
    std::string result;

    while (first != last) {
        result += *first;
        ++first;
    }

    REQUIRE(result == expected);
}

/**************************************************************************************************/

template <typename Iterator, std::size_t Edge, typename Forest>
auto test_edge_traversal(Forest& f, Iterator fi, Iterator li) {
    std::string expected;

    {
        Iterator first{fi};
        Iterator last{li};
        while (first != last) {
            if (first.edge() == Edge)
                expected += *first;
            ++first;
        }
        REQUIRE(expected.size() == f.size());
    }

    {
        edge_iterator<Iterator, Edge> first(fi);
        edge_iterator<Iterator, Edge> last(li);
        std::string result;
        while (first != last) {
            result += *first;
            ++first;
        }
        REQUIRE(result == expected);
    }

    return expected;
}

/**************************************************************************************************/

template <std::size_t Edge, typename Forest>
auto test_range_traversal(Forest& f, const std::string& expected) {
    if constexpr (Edge == FNS::forest_leading_edge) {
        std::string result;
        for (const auto& x : preorder_range(f)) {
            result += x;
        }
        REQUIRE(result == expected);
    } else if constexpr (Edge == FNS::forest_trailing_edge) {
        std::string result;
        for (const auto& x : postorder_range(f)) {
            result += x;
        }
        REQUIRE(result == expected);
    }
}

/**************************************************************************************************/

void test_reverse_preorder_traversal() {
    std::string expected;
    const auto f{big_test_forest()};

    {
        auto first{f.rbegin()};
        auto last{f.rend()};
        while (first != last) {
            if (first.edge() == FNS::forest_leading_edge)
                expected += *first;
            ++first;
        }
        REQUIRE(expected.size() == f.size());
    }

    {
        using const_iterator = forest<std::string>::const_reverse_iterator;
        using iterator = edge_iterator<const_iterator, forest_leading_edge>;
        iterator first(f.rbegin());
        iterator last(f.rend());
        std::string result;
        while (first != last) {
            result += *first;
            ++first;
        }
        REQUIRE(result == expected);
    }
}

/**************************************************************************************************/

using iterator_t = FNS::forest<std::string>::iterator;
using const_iterator_t = FNS::forest<std::string>::const_iterator;
using reverse_iterator_t = FNS::forest<std::string>::reverse_iterator;
using const_reverse_iterator_t = FNS::forest<std::string>::const_reverse_iterator;

/**************************************************************************************************/

TEST_CASE("forward traversal") {
    auto f{big_test_forest()};
    auto first{std::begin(f)};
    auto last{std::end(f)};
    static const auto expected{"ABCFFGGHHCDIIJJKKDEEBA"};

    SECTION("fullorder") {
        test_fullorder_traversal<iterator_t>(first, last, expected);
        test_fullorder_traversal<const_iterator_t>(first, last, expected);
    }

    SECTION("preorder") {
        auto a = test_edge_traversal<iterator_t, FNS::forest_leading_edge>(f, first, last);
        auto b = test_edge_traversal<const_iterator_t, FNS::forest_leading_edge>(f, first, last);
        REQUIRE(a == b);
        test_range_traversal<FNS::forest_leading_edge>(f, a);
    }

    SECTION("postorder") {
        auto a = test_edge_traversal<iterator_t, FNS::forest_trailing_edge>(f, first, last);
        auto b = test_edge_traversal<const_iterator_t, FNS::forest_trailing_edge>(f, first, last);
        REQUIRE(a == b);
        test_range_traversal<FNS::forest_trailing_edge>(f, a);
    }
}

/**************************************************************************************************/

TEST_CASE("reverse traversal") {
    auto f{big_test_forest()};
    auto rfirst{std::rbegin(f)};
    auto rlast{std::rend(f)};
    static const auto expected{"ABEEDKKJJIIDCHHGGFFCBA"};

    SECTION("fullorder") {
        test_fullorder_traversal<reverse_iterator_t>(rfirst, rlast, expected);
        test_fullorder_traversal<const_reverse_iterator_t>(rfirst, rlast, expected);
    }

    SECTION("preorder") {
        auto a = test_edge_traversal<reverse_iterator_t, FNS::forest_leading_edge>(f, rfirst, rlast);
        auto b = test_edge_traversal<const_reverse_iterator_t, FNS::forest_leading_edge>(f, rfirst, rlast);
        REQUIRE(a == b);
    }

    SECTION("postorder") {
        auto a = test_edge_traversal<reverse_iterator_t, FNS::forest_trailing_edge>(f, rfirst, rlast);
        auto b = test_edge_traversal<const_reverse_iterator_t, FNS::forest_trailing_edge>(f, rfirst, rlast);
        REQUIRE(a == b);
    }
}

/**************************************************************************************************/
