/**************************************************************************************************/

// stdc++
#include <iostream>

// catch
#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

// must be defined before forest_fwd to select the forest we want to test.
#define FORESTVG_USING_STLAB_FOREST() 1

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
    REQUIRE(*il == 42);
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

template <typename R>
inline auto to_string(R&& r) {
    std::string result;
    for (const auto& x : r) {
        result += x;
    }
    return result;
}

template <typename I>
inline auto to_string(I first, I last) {
    std::string result;
    while (first != last) {
        result += *first++;
    }
    return result;
}

/**************************************************************************************************/

template <typename Iterator>
void test_fullorder_traversal(Iterator first, Iterator last, std::string expected) {
    REQUIRE(to_string(first, last) == expected);
}

/**************************************************************************************************/

template <typename Iterator, FNS::forest_edge Edge, typename Forest>
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
        std::string result{to_string(first, last)};
        REQUIRE(result == expected);
    }

    return expected;
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

    SECTION("fullorder") {
        static const auto expected{"ABCFFGGHHCDIIJJKKDEEBA"};
        test_fullorder_traversal<iterator_t>(first, last, expected);
        test_fullorder_traversal<const_iterator_t>(first, last, expected);

#if 0 // fullorder_range doesn't exist?
        REQUIRE(range_value(fullorder_range(f)) == expected);
#endif
    }

    SECTION("preorder") {
        auto a = test_edge_traversal<iterator_t, FNS::forest_leading_edge>(f, first, last);
        auto b = test_edge_traversal<const_iterator_t, FNS::forest_leading_edge>(f, first, last);
        REQUIRE(a == b);
        REQUIRE(to_string(preorder_range(f)) == a);
    }

    SECTION("postorder") {
        auto a = test_edge_traversal<iterator_t, FNS::forest_trailing_edge>(f, first, last);
        auto b = test_edge_traversal<const_iterator_t, FNS::forest_trailing_edge>(f, first, last);
        REQUIRE(a == b);
        REQUIRE(to_string(postorder_range(f)) == a);
    }
}

/**************************************************************************************************/

TEST_CASE("reverse traversal") {
    auto f{big_test_forest()};
    auto rfirst{std::rbegin(f)};
    auto rlast{std::rend(f)};

    SECTION("fullorder") {
        static const auto expected{"ABEEDKKJJIIDCHHGGFFCBA"};
        test_fullorder_traversal<reverse_iterator_t>(rfirst, rlast, expected);
        test_fullorder_traversal<const_reverse_iterator_t>(rfirst, rlast, expected);
        REQUIRE(to_string(reverse_fullorder_range(f)) == expected);
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

TEST_CASE("child traversal") {
    auto f{big_test_forest()};
    auto parent{std::find_if(f.begin(), f.end(), [](auto& x){ return x == "B"; })};
    std::string expected;

    REQUIRE(*parent == "B");

    {
        auto first{child_begin(parent)};
        auto last{child_end(parent)};
        expected = to_string(first, last);
    }

    REQUIRE(to_string(child_range(parent)) == expected);

    if constexpr (false) { // I'm not sure this ever worked.
        FNS::forest<std::string>::reverse_child_iterator first{child_begin(parent)};
        FNS::forest<std::string>::reverse_child_iterator last{child_end(parent)};
        std::string result{to_string(first, last)};
        REQUIRE(result == expected);
    }
}

/**************************************************************************************************/

TEST_CASE("find parent") {
    auto f{big_test_forest()};

    {
        auto child{std::find_if(f.begin(), f.end(), [](auto& x){ return x == "B"; })};
        REQUIRE(*child == "B");
        auto parent{find_parent(child)};
        REQUIRE(*parent == "A");
    }

    {
        auto child{std::find_if(f.begin(), f.end(), [](auto& x){ return x == "J"; })};
        REQUIRE(*child == "J");
        auto parent{find_parent(child)};
        REQUIRE(*parent == "D");
    }
}

/**************************************************************************************************/

TEST_CASE("has_children") {
    auto f{big_test_forest()};

    {
        auto node{std::find_if(f.begin(), f.end(), [](auto& x){ return x == "B"; })};
        REQUIRE(*node == "B");
        REQUIRE(has_children(node));
    }

    {
        auto node{std::find_if(f.begin(), f.end(), [](auto& x){ return x == "J"; })};
        REQUIRE(*node == "J");
        REQUIRE(!has_children(node));
    }
}

/**************************************************************************************************/
