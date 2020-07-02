/******/
#include <iostream>

#include <adobe/forest.hpp>

template <typename T>
void print(const adobe::forest<T>& f) {
    auto first{f.begin()};
    auto last{f.end()};
    std::size_t depth{0};

    while (first != last) {
        if (first.edge() == adobe::forest_trailing_edge) {
            --depth;
        }
        for (std::size_t i{0}; i < depth*4; ++i) std::cout << ' ';
        if (first.edge() == adobe::forest_leading_edge) {
            std::cout << "<";
            ++depth;
        } else {
            std::cout << "</";
        }
        std::cout << *first << ">\n";
        ++first;
    }
}

template <typename T, typename P, typename U = decltype(std::declval<P>()(T()))>
adobe::forest<U> transcribe_forest(const adobe::forest<T>& f, P&& proj) {
    adobe::forest<U> result;
    auto pos{result.root()};
    auto first{f.begin()};
    const auto last{f.end()};

    while (first != last) {
        ++pos;
        if (first.edge() == adobe::forest_leading_edge) {
            pos = result.insert(pos, proj(*first));
        } else {
            pos = adobe::trailing_of(pos);
        }
        ++first;
    }

    return result;
}

template <typename I1, typename I2, typename P>
I2 transform_forest(I1 first, I1 last, I2 out, P&& proj) {
    while (first != last) {
        ++out;
        if (first.edge() == adobe::forest_leading_edge) {
            out.insert(proj(*first));
        } else {
            out.trailing();
        }
        ++first;
    }

    return out;
}

template <typename T>
struct forest_inserter {
    adobe::forest<T>& _f;
    typename adobe::forest<T>::iterator _p;

    explicit forest_inserter(adobe::forest<T>& f) : _f{f}, _p{_f.root()} {}

    forest_inserter& operator++() { ++_p; return *this; }

    void insert(T&& x) { _p = _f.insert(_p, std::forward<T>(x)); }

    void trailing() { _p = adobe::trailing_of(_p); }
};

int main(int argc, const char * argv[]) {
    adobe::forest<int> int_forest;

    auto gparent = int_forest.insert(int_forest.begin(), 11);
    auto parent = int_forest.insert(adobe::trailing_of(gparent), 21);
    auto self = int_forest.insert(adobe::trailing_of(parent), 31);
    int_forest.insert(adobe::trailing_of(self), 41);
    int_forest.insert(adobe::trailing_of(self), 42);
    int_forest.insert(adobe::trailing_of(self), 43);
    auto sib1 = int_forest.insert(adobe::trailing_of(parent), 32);
    int_forest.insert(adobe::trailing_of(sib1), 51);
    int_forest.insert(adobe::trailing_of(sib1), 52);
    int_forest.insert(adobe::trailing_of(sib1), 53);
    int_forest.insert(adobe::trailing_of(parent), 33);

    print(int_forest);

    auto forest_copy = transcribe_forest(int_forest, [](const auto& x){
        return x;
    });

    if (int_forest != forest_copy) throw std::runtime_error("equivalence mismatch");

    adobe::forest<int> copy2;
    transform_forest(int_forest.begin(), int_forest.end(), forest_inserter(copy2), [](const auto& x){
        return x;
    });

    if (copy2 != int_forest) throw std::runtime_error("equivalence mismatch");
    if (copy2 != forest_copy) throw std::runtime_error("equivalence mismatch");

    auto other_forest = transcribe_forest(int_forest, [](const auto& x){
        return std::to_string(x);
    });

    std::cout << ".-==-.\n";
    print(other_forest);

    return 0;
}
