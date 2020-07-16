/**************************************************************************************************/

// stdc++
#include <iostream>

#define FORESTVG_USING_STLAB_FOREST() 0

// application
#include "../headers/forest_algorithms.hpp"
#include "../headers/forest_fwd.hpp"

/**************************************************************************************************/

using namespace FNS; // either adobe or stlab, depending.

/**************************************************************************************************/

auto test_empty() {
    forest<int> f;

    assert(f.empty());
    assert(f.begin() == f.end());
}

/**************************************************************************************************/

auto test_one_node() {
    forest<int> f;

    auto il = f.insert(f.end(), 42);
    assert(il.edge() == FNS::forest_leading_edge);
    assert(!f.empty());

    auto it = trailing_of(il);
    assert(it.edge() == FNS::forest_trailing_edge);
    assert(*it == *il);
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

    assert(f.size() == 11);

    assert(has_children(a_iter));
    assert(has_children(b_iter));
    assert(has_children(c_iter));
    assert(has_children(d_iter));
    assert(!has_children(e_iter));

    return f;
}

/**************************************************************************************************/

void test_fullorder_traversal() {
    const auto f{big_test_forest()};
    auto first{f.begin()};
    auto last{f.end()};
    std::string expected{"ABCFFGGHHCDIIJJKKDEEBA"};
    std::string result;

    while (first != last) {
        result += *first;
        ++first;
    }

    assert(result == expected);
}

/**************************************************************************************************/

void test_preorder_traversal() {
    std::string expected;
    const auto f{big_test_forest()};

    {
        auto first{f.begin()};
        auto last{f.end()};
        while (first != last) {
            if (first.edge() == FNS::forest_leading_edge)
                expected += *first;
            ++first;
        }
        assert(expected.size() == f.size());
    }

    {
        forest<std::string>::const_preorder_iterator first(f.begin());
        forest<std::string>::const_preorder_iterator last(f.end());
        std::string result;
        while (first != last) {
            result += *first;
            ++first;
        }
        assert(result == expected);
    }

    {
        std::string result;
        for (const auto& x : preorder_range(f)) {
            result += x;
        }
        assert(result == expected);
    }
}

/**************************************************************************************************/

void test_postorder_traversal() {
    std::string expected;
    const auto f{big_test_forest()};

    {
        auto first{f.begin()};
        auto last{f.end()};
        while (first != last) {
            if (first.edge() == FNS::forest_trailing_edge)
                expected += *first;
            ++first;
        }
        assert(expected.size() == f.size());
    }

    {
        forest<std::string>::const_postorder_iterator first(f.begin());
        forest<std::string>::const_postorder_iterator last(f.end());
        std::string result;
        while (first != last) {
            result += *first;
            ++first;
        }
        assert(result == expected);
    }

    {
        std::string result;
        for (const auto& x : postorder_range(f)) {
            result += x;
        }
        assert(result == expected);
    }
}

/**************************************************************************************************/

int main(int argc, const char * argv[]) try {
    test_empty();
    test_one_node();
    test_fullorder_traversal();
    test_preorder_traversal();
    test_postorder_traversal();

    // fvg::print(big_test_forest());

    return EXIT_SUCCESS;
} catch (const std::exception& error) {
    std::cerr << "Fatal exception: " << error.what() << '\n';
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "Fatal exception: unknown\n";
    return EXIT_FAILURE;
}

/**************************************************************************************************/
